/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 G. Elian Gidoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * @addtogroup piojograph Piojo Graph
 * @{
 * Piojo Graph implementation.
 */

#include <piojo/piojo_graph.h>
#include <piojo/piojo_array.h>
#include <piojo/piojo_stack.h>
#include <piojo_defs.h>

typedef struct {
        piojo_array_t *edges_by_vid;
        bool deleted_p;
        /* Vertex identifier. */
        size_t vid;
        /* Optional user data. */
        void *data;
} piojo_graph_alist_t;

typedef struct {
        int weight;
        size_t end_vid;
} piojo_graph_edge_t;

struct piojo_graph {
        piojo_array_t *alists_by_vid;
        piojo_stack_t *deleted_vids;
        piojo_graph_dir_t dir;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of graph in bytes */
const size_t piojo_graph_sizeof = sizeof(piojo_graph_t);

static const size_t DEFAULT_EDGE_COUNT = 8;

static void
link_vertices(int weight, piojo_graph_alist_t *from, piojo_graph_alist_t *to);

static void
unlink_vertices(piojo_graph_alist_t *from, piojo_graph_alist_t *to);

static int
edge_cmp(const void *e1, const void *e2);

static void
free_edges(const piojo_graph_t *graph);

static piojo_graph_vertex_t
vid_to_vertex(size_t vid);

static piojo_graph_alist_t*
vid_to_alist(size_t vid, const piojo_graph_t *graph);

static size_t
vertex_to_vid(piojo_graph_vertex_t vertex);

static piojo_graph_alist_t*
vertex_to_alist(piojo_graph_vertex_t vertex, const piojo_graph_t *graph);

/**
 * Allocates a new graph.
 * Uses default allocator.
 * @param[in] directed Graph direction.
 * @return New graph.
 */
piojo_graph_t*
piojo_graph_alloc(piojo_graph_dir_t directed)
{
        return piojo_graph_alloc_cb(directed, piojo_alloc_default);
}

/**
 * Allocates a new graph.
 * @param[in] directed Graph direction.
 * @param[in] allocator Allocator to be used.
 * @return New graph.
 */
piojo_graph_t*
piojo_graph_alloc_cb(piojo_graph_dir_t directed, piojo_alloc_if allocator)
{
        piojo_graph_t * graph;
        PIOJO_ASSERT(sizeof(piojo_graph_vertex_t) >= sizeof(size_t));

        graph = (piojo_graph_t *) allocator.alloc_cb(sizeof(piojo_graph_t));
        PIOJO_ASSERT(graph);

        graph->allocator = allocator;
        graph->dir = directed;
        graph->alists_by_vid = piojo_array_alloc_cb(NULL,
                                                    sizeof(piojo_graph_alist_t),
                                                    allocator);
        PIOJO_ASSERT(graph->alists_by_vid);
        graph->deleted_vids = piojo_stack_alloc_cb(sizeof(size_t), allocator);
        PIOJO_ASSERT(graph->deleted_vids);

        return graph;
}

/**
 * Copies @a graph and all its vertices/edges.
 * @param[in] graph Graph being copied.
 * @return New graph.
 */
piojo_graph_t*
piojo_graph_copy(const piojo_graph_t *graph)
{
        piojo_alloc_if allocator;
        size_t i, alist_cnt;
        piojo_graph_t *newgraph;
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        allocator = graph->allocator;
        newgraph = (piojo_graph_t *) allocator.alloc_cb(sizeof(piojo_graph_t));
        PIOJO_ASSERT(newgraph);

        newgraph->allocator = allocator;
        newgraph->dir = graph->dir;
        newgraph->alists_by_vid = piojo_array_copy(graph->alists_by_vid);
        newgraph->deleted_vids = piojo_stack_copy(graph->deleted_vids);

        /* Copy all edges. */
        alist_cnt = piojo_array_size(newgraph->alists_by_vid);
        for (i = 0; i < alist_cnt; ++i){
                alist = ((piojo_graph_alist_t *)
                        piojo_array_at(i, newgraph->alists_by_vid));
                alist->edges_by_vid = piojo_array_copy(alist->edges_by_vid);
        }

        return newgraph;
}

/**
 * Frees @a graph and all its vertices/edges.
 * @param[in] graph Graph being freed.
 */
void
piojo_graph_free(const piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        free_edges(graph);
        piojo_array_free(graph->alists_by_vid);
        piojo_stack_free(graph->deleted_vids);
        graph->allocator.free_cb(graph);
}


/**
 * Deletes all vertices/edges in @a graph.
 * @param[out] graph Graph being cleared.
 */
void
piojo_graph_clear(piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        free_edges(graph);
        piojo_array_clear(graph->alists_by_vid);
        piojo_stack_clear(graph->deleted_vids);
}

/**
 * Inserts vertex.
 * @param[out] graph
 * @return New vertex in @a graph.
 */
piojo_graph_vertex_t
piojo_graph_insert(piojo_graph_t *graph)
{
        size_t idx;
        piojo_graph_alist_t tmp, *alist;
        PIOJO_ASSERT(graph);

        if (piojo_stack_size(graph->deleted_vids) > 0){
                idx = *(size_t *)piojo_stack_peek(graph->deleted_vids);
                piojo_stack_pop(graph->deleted_vids);
        }else{
                idx = piojo_array_size(graph->alists_by_vid);
                tmp.vid = idx;
                tmp.data = NULL;
                tmp.edges_by_vid =
                        piojo_array_alloc_cb_n(edge_cmp,
                                               sizeof(piojo_graph_edge_t),
                                               DEFAULT_EDGE_COUNT,
                                               graph->allocator);
                piojo_array_append(&tmp, graph->alists_by_vid);
        }
        alist = vid_to_alist(idx, graph);
        alist->deleted_p = FALSE;

        return vid_to_vertex(idx);
}

/**
 * Deletes vertex.
 * @param[in] vertex Vertex being deleted.
 * @param[out] graph
 */
void
piojo_graph_delete(piojo_graph_vertex_t vertex, piojo_graph_t *graph)
{
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        piojo_graph_unlink_all(vertex, graph);

        alist = vertex_to_alist(vertex, graph);
        PIOJO_ASSERT(! alist->deleted_p);

        alist->data = NULL;
        alist->deleted_p = TRUE;
        piojo_stack_push(&alist->vid, graph->deleted_vids);
}

/**
 * Sets user-defined @a vertex value (optional).
 * @param[in] value
 * @param[in] vertex
 * @param[out] graph
 */
void
piojo_graph_set_vvalue(const void *value, piojo_graph_vertex_t vertex,
                       piojo_graph_t *graph)
{
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        alist = vertex_to_alist(vertex, graph);
        alist->data = (void *)value;
}

/**
 * Returns user-defined @a vertex value.
 * @param[in] vertex
 * @param[in] graph
 * @return Vertex value or @b NULL if it's not set.
 */
void*
piojo_graph_vvalue(piojo_graph_vertex_t vertex, const piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        return vertex_to_alist(vertex, graph)->data;
}

/**
 * Links two vertices, if they're already linked updates the weight.
 * @param[in] weight Edge weight.
 * @param[in] from Source vertex.
 * @param[in] to Destination vertex.
 * @param[out] graph
 */
void
piojo_graph_link(int weight, piojo_graph_vertex_t from,
                 piojo_graph_vertex_t to, piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        link_vertices(weight, vertex_to_alist(from, graph),
                      vertex_to_alist(to, graph));
        if (graph->dir == PIOJO_GRAPH_DIR_FALSE){
                link_vertices(weight, vertex_to_alist(to, graph),
                              vertex_to_alist(from, graph));
        }
}

/**
 * Tests if an edge exists between two vertices.
 * @param[in] from Source vertex.
 * @param[in] to Destination vertex.
 * @param[in] graph
 * @return @b TRUE if an edge exists, @b FALSE otherwise.
 */
bool
piojo_graph_link_p(piojo_graph_vertex_t from, piojo_graph_vertex_t to,
                   const piojo_graph_t *graph)
{
        piojo_graph_edge_t edge;
        PIOJO_ASSERT(graph);

        edge.end_vid = vertex_to_vid(to);
        return piojo_array_has_p(&edge,
                                 vertex_to_alist(from, graph)->edges_by_vid,
                                 NULL);
}

/**
 * Unlinks two vertices if they are linked (does nothing otherwise).
 * @param[in] from Vertex.
 * @param[in] to Vertex.
 * @param[out] graph
 */
void
piojo_graph_unlink(piojo_graph_vertex_t from, piojo_graph_vertex_t to,
                   piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        unlink_vertices(vertex_to_alist(from, graph),
                        vertex_to_alist(to, graph));
        if (graph->dir == PIOJO_GRAPH_DIR_FALSE){
                unlink_vertices(vertex_to_alist(to, graph),
                                vertex_to_alist(from, graph));
        }
}

/**
 * Unlinks every vertex from @a vertex.
 * @param[in] vertex Vertex.
 * @param[out] graph
 */
void
piojo_graph_unlink_all(piojo_graph_vertex_t vertex, piojo_graph_t *graph)
{
        piojo_graph_edge_t *edge;
        piojo_graph_alist_t *v;
        size_t cnt, i;
        PIOJO_ASSERT(graph);

        v = vertex_to_alist(vertex, graph);
        cnt = piojo_array_size(v->edges_by_vid);
        for (i = 0; i < cnt; ++i){
                edge = (piojo_graph_edge_t *)piojo_array_at(i, v->edges_by_vid);
                unlink_vertices(vid_to_alist(edge->end_vid, graph), v);
        }
        piojo_array_clear(v->edges_by_vid);
}

/**
 * Returns neighbor count from @a vertex.
 * @param[in] vertex Vertex.
 * @param[in] graph
 * @return Number of neighbor vertices.
 */
size_t
piojo_graph_neighbor_cnt(piojo_graph_vertex_t vertex,
                         const piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        return piojo_array_size(vertex_to_alist(vertex, graph)->edges_by_vid);
}

/**
 * Returns neighbor from @a vertex.
 * @param[in] idx Neighbor index.
 * @param[in] vertex Vertex.
 * @param[in] graph
 * @return Neighbor vertex.
 */
piojo_graph_vertex_t
piojo_graph_neighbor_at(size_t idx, piojo_graph_vertex_t vertex,
                        const piojo_graph_t *graph)
{
        piojo_graph_edge_t *edge;
        piojo_graph_alist_t *v;
        PIOJO_ASSERT(graph);

        v = vertex_to_alist(vertex, graph);
        edge = (piojo_graph_edge_t *)piojo_array_at(idx, v->edges_by_vid);
        return vid_to_vertex(edge->end_vid);
}

/**
 * Returns edge weight from @a vertex to neighbor @a idx.
 * @param[in] idx Neighbor index.
 * @param[in] vertex Vertex.
 * @param[in] graph
 * @return Neighbor edge weight.
 */
int
piojo_graph_edge_weight(size_t idx, piojo_graph_vertex_t vertex,
                        const piojo_graph_t *graph)
{
        piojo_graph_edge_t *edge;
        piojo_graph_alist_t * v;
        PIOJO_ASSERT(graph);

        v = vertex_to_alist(vertex, graph);
        edge = (piojo_graph_edge_t *)piojo_array_at(idx, v->edges_by_vid);
        return edge->weight;
}

/** @}
 * Private functions.
 */

static int
edge_cmp(const void *e1, const void *e2)
{
        piojo_graph_edge_t *v1 = (piojo_graph_edge_t *) e1;
        piojo_graph_edge_t *v2 = (piojo_graph_edge_t *) e2;
        if (v1->end_vid > v2->end_vid){
                return 1;
        }else if (v1->end_vid < v2->end_vid){
                return -1;
        }
        return 0;
}

static void
link_vertices(int weight, piojo_graph_alist_t *from, piojo_graph_alist_t *to)
{
        bool linked_p;
        size_t idx;
        piojo_graph_edge_t edge, *e;

        edge.weight = weight;
        edge.end_vid = to->vid;
        linked_p = piojo_array_has_p(&edge, from->edges_by_vid, &idx);
        if (linked_p){
                e = (piojo_graph_edge_t *)piojo_array_at(idx,
                                                         from->edges_by_vid);
                e->weight = weight;
        }else{
                piojo_array_append(&edge, from->edges_by_vid);
        }
}

static void
unlink_vertices(piojo_graph_alist_t *from, piojo_graph_alist_t *to)
{
        bool linked_p;
        size_t idx;
        piojo_graph_edge_t edge;

        edge.end_vid = to->vid;
        linked_p = piojo_array_has_p(&edge, from->edges_by_vid, &idx);
        if (linked_p){
                piojo_array_delete(idx, from->edges_by_vid);
        }
}

static void
free_edges(const piojo_graph_t *graph)
{
        size_t i, alist_cnt;
        piojo_graph_alist_t *alist;

        alist_cnt = piojo_array_size(graph->alists_by_vid);
        for (i = 0; i < alist_cnt; ++i){
                alist = ((piojo_graph_alist_t *)
                        piojo_array_at(i, graph->alists_by_vid));
                piojo_array_free(alist->edges_by_vid);
        }
}

static piojo_graph_alist_t*
vid_to_alist(size_t vid, const piojo_graph_t *graph)
{
        return (piojo_graph_alist_t *)piojo_array_at(vid, graph->alists_by_vid);
}

static piojo_graph_vertex_t
vid_to_vertex(size_t vid)
{
        piojo_graph_vertex_t v;
        *(size_t *)v.opaque = vid;
        return v;
}

static size_t
vertex_to_vid(piojo_graph_vertex_t vertex)
{
        return *(size_t *)vertex.opaque;
}

static piojo_graph_alist_t*
vertex_to_alist(piojo_graph_vertex_t vertex, const piojo_graph_t *graph)
{
        return vid_to_alist(vertex_to_vid(vertex), graph);
}
