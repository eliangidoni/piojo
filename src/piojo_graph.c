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
#include <piojo/piojo_hash.h>
#include <piojo/piojo_stack.h>
#include <piojo/piojo_queue.h>
#include <piojo_defs.h>

typedef struct {
        piojo_array_t *edges_by_vid;
        /* Vertex identifier. */
        piojo_graph_vid_t vid;
        /* Optional user data. */
        void *data;
} piojo_graph_alist_t;

typedef struct {
        int weight;
        piojo_graph_vid_t end_vid;
} piojo_graph_edge_t;

struct piojo_graph {
        piojo_hash_t *alists_by_vid;
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

static piojo_graph_alist_t*
vid_to_alist(piojo_graph_vid_t vertex, const piojo_graph_t *graph);

static bool
vid_eq(const void *e1, const void *e2);

static piojo_hash_t*
alloc_visiteds(const piojo_graph_t *graph);

static void
free_visiteds(piojo_hash_t *visiteds);

static bool
is_visited_p(piojo_graph_vid_t vid, piojo_hash_t *visiteds);

static void
set_visited(bool visited_p, piojo_graph_vid_t vid, piojo_hash_t *visiteds);

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
        piojo_alloc_kv_if ator = piojo_alloc_kv_default;
        size_t esize;

        ator.alloc_cb = allocator.alloc_cb;
        ator.free_cb = allocator.free_cb;
        esize = sizeof(piojo_graph_alist_t);

        graph = (piojo_graph_t *) allocator.alloc_cb(sizeof(piojo_graph_t));
        PIOJO_ASSERT(graph);

        graph->allocator = allocator;
        graph->dir = directed;
        graph->alists_by_vid = piojo_hash_alloc_cb_eq(esize,
                                                      vid_eq,
                                                      sizeof(piojo_graph_vid_t),
                                                      ator);
        PIOJO_ASSERT(graph->alists_by_vid);

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
        piojo_hash_node_t *next, node;
        piojo_graph_t *newgraph;
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        allocator = graph->allocator;
        newgraph = (piojo_graph_t *) allocator.alloc_cb(sizeof(piojo_graph_t));
        PIOJO_ASSERT(newgraph);

        newgraph->allocator = allocator;
        newgraph->dir = graph->dir;
        newgraph->alists_by_vid = piojo_hash_copy(graph->alists_by_vid);

        /* Copy all edges. */
        next = piojo_hash_first(newgraph->alists_by_vid, &node);
        while (next){
                alist = (piojo_graph_alist_t *) piojo_hash_entryv(next);
                alist->edges_by_vid = piojo_array_copy(alist->edges_by_vid);
                next = piojo_hash_next(next);
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
        piojo_hash_free(graph->alists_by_vid);
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
        piojo_hash_clear(graph->alists_by_vid);
}

/**
 * Inserts new vertex.
 * @param[in] vertex_id
 * @param[out] graph
 * @return @b TRUE if inserted, @b FALSE if it's duplicated.
 */
bool
piojo_graph_insert(piojo_graph_vid_t vertex_id, piojo_graph_t *graph)
{
        piojo_graph_alist_t tmp;
        PIOJO_ASSERT(graph);

        tmp.vid = vertex_id;
        tmp.data = NULL;
        tmp.edges_by_vid =
                piojo_array_alloc_cb_n(edge_cmp,
                                       sizeof(piojo_graph_edge_t),
                                       DEFAULT_EDGE_COUNT,
                                       graph->allocator);
        return piojo_hash_insert(&tmp.vid, &tmp, graph->alists_by_vid);
}

/**
 * Deletes vertex.
 * @param[in] vertex Vertex being deleted.
 * @param[out] graph
 * @return @b TRUE if deleted, @b FALSE if it's not present.
 */
bool
piojo_graph_delete(piojo_graph_vid_t vertex, piojo_graph_t *graph)
{
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        alist = vid_to_alist(vertex, graph);
        alist->data = NULL;

        piojo_graph_unlink_all(vertex, graph);
        piojo_array_free(alist->edges_by_vid);

        return piojo_hash_delete(&alist->vid, graph->alists_by_vid);
}

/**
 * Sets user-defined @a vertex value (optional).
 * @param[in] value
 * @param[in] vertex
 * @param[out] graph
 */
void
piojo_graph_set_vvalue(const void *value, piojo_graph_vid_t vertex,
                       piojo_graph_t *graph)
{
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        alist = vid_to_alist(vertex, graph);
        alist->data = (void *)value;
}

/**
 * Returns user-defined @a vertex value.
 * @param[in] vertex
 * @param[in] graph
 * @return Vertex value or @b NULL if it's not set.
 */
void*
piojo_graph_vvalue(piojo_graph_vid_t vertex, const piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        return vid_to_alist(vertex, graph)->data;
}

/**
 * Links two vertices, if they're already linked updates the weight.
 * @param[in] weight Edge weight.
 * @param[in] from Source vertex.
 * @param[in] to Destination vertex.
 * @param[out] graph
 */
void
piojo_graph_link(int weight, piojo_graph_vid_t from,
                 piojo_graph_vid_t to, piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        link_vertices(weight, vid_to_alist(from, graph),
                      vid_to_alist(to, graph));
        if (graph->dir == PIOJO_GRAPH_DIR_FALSE){
                link_vertices(weight, vid_to_alist(to, graph),
                              vid_to_alist(from, graph));
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
piojo_graph_link_p(piojo_graph_vid_t from, piojo_graph_vid_t to,
                   const piojo_graph_t *graph)
{
        piojo_graph_edge_t edge;
        PIOJO_ASSERT(graph);

        edge.end_vid = to;
        return piojo_array_has_p(&edge,
                                 vid_to_alist(from, graph)->edges_by_vid,
                                 NULL);
}

/**
 * Unlinks two vertices if they are linked (does nothing otherwise).
 * @param[in] from Vertex.
 * @param[in] to Vertex.
 * @param[out] graph
 */
void
piojo_graph_unlink(piojo_graph_vid_t from, piojo_graph_vid_t to,
                   piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        unlink_vertices(vid_to_alist(from, graph),
                        vid_to_alist(to, graph));
        if (graph->dir == PIOJO_GRAPH_DIR_FALSE){
                unlink_vertices(vid_to_alist(to, graph),
                                vid_to_alist(from, graph));
        }
}

/**
 * Unlinks every vertex from @a vertex.
 * @param[in] vertex Vertex.
 * @param[out] graph
 */
void
piojo_graph_unlink_all(piojo_graph_vid_t vertex, piojo_graph_t *graph)
{
        piojo_graph_edge_t *edge;
        piojo_graph_alist_t *v;
        size_t cnt, i;
        PIOJO_ASSERT(graph);

        v = vid_to_alist(vertex, graph);
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
piojo_graph_neighbor_cnt(piojo_graph_vid_t vertex,
                         const piojo_graph_t *graph)
{
        PIOJO_ASSERT(graph);

        return piojo_array_size(vid_to_alist(vertex, graph)->edges_by_vid);
}

/**
 * Returns neighbor from @a vertex.
 * @param[in] idx Neighbor index.
 * @param[in] vertex Vertex.
 * @param[in] graph
 * @return Neighbor vertex.
 */
piojo_graph_vid_t
piojo_graph_neighbor_at(size_t idx, piojo_graph_vid_t vertex,
                        const piojo_graph_t *graph)
{
        piojo_graph_edge_t *edge;
        piojo_graph_alist_t *v;
        PIOJO_ASSERT(graph);

        v = vid_to_alist(vertex, graph);
        edge = (piojo_graph_edge_t *)piojo_array_at(idx, v->edges_by_vid);
        return edge->end_vid;
}

/**
 * Returns edge weight from @a vertex to neighbor @a idx.
 * @param[in] idx Neighbor index.
 * @param[in] vertex Vertex.
 * @param[in] graph
 * @return Neighbor edge weight.
 */
int
piojo_graph_edge_weight(size_t idx, piojo_graph_vid_t vertex,
                        const piojo_graph_t *graph)
{
        piojo_graph_edge_t *edge;
        piojo_graph_alist_t * v;
        PIOJO_ASSERT(graph);

        v = vid_to_alist(vertex, graph);
        edge = (piojo_graph_edge_t *)piojo_array_at(idx, v->edges_by_vid);
        return edge->weight;
}

/**
 * Traverses @a graph following a breadth first search.
 * @param[in] root Starting vertex.
 * @param[in] cb Vertex visit function.
 * @param[in] data Argument passed to @a cb function.
 * @param[in] graph
 */
void
piojo_graph_breadth_first(piojo_graph_vid_t root, piojo_graph_visit_cb cb,
                          const void *data, const piojo_graph_t *graph)
{
        piojo_queue_t *q;
        piojo_hash_t *visiteds;
        size_t i, cnt;
        piojo_graph_vid_t vcur, vneighbor;

        q = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE,
                                 sizeof(piojo_graph_vid_t),
                                 graph->allocator);
        visiteds = alloc_visiteds(graph);

        piojo_queue_push(&root, q);
        set_visited(TRUE, root, visiteds);
        while(piojo_queue_size(q) > 0){
                vcur = *(piojo_graph_vid_t*) piojo_queue_peek(q);
                piojo_queue_pop(q);
                if (cb(vcur, graph, (void *)data)){
                        break;
                }
                cnt = piojo_graph_neighbor_cnt(vcur, graph);
                for (i = 0; i < cnt; ++i){
                        vneighbor = piojo_graph_neighbor_at(i, vcur, graph);
                        if (! is_visited_p(vneighbor, visiteds)){
                                piojo_queue_push(&vneighbor, q);
                                set_visited(TRUE, vneighbor, visiteds);
                        }
                }
        }
        piojo_queue_free(q);
        free_visiteds(visiteds);
}

/**
 * Traverses @a graph following a depth first search.
 * @param[in] root Starting vertex.
 * @param[in] cb Vertex visit function.
 * @param[in] data Argument passed to @a cb function.
 * @param[in] graph
 */
void
piojo_graph_depth_first(piojo_graph_vid_t root, piojo_graph_visit_cb cb,
                        const void *data, const piojo_graph_t *graph)
{
        piojo_stack_t *st;
        piojo_hash_t *visiteds;
        size_t i, cnt;
        piojo_graph_vid_t vcur, vneighbor;

        st = piojo_stack_alloc_cb(sizeof(piojo_graph_vid_t), graph->allocator);
        visiteds = alloc_visiteds(graph);

        piojo_stack_push(&root, st);
        set_visited(TRUE, root, visiteds);
        while(piojo_stack_size(st) > 0){
                vcur = *(piojo_graph_vid_t*) piojo_stack_peek(st);
                piojo_stack_pop(st);
                if (cb(vcur, graph, (void *)data)){
                        break;
                }
                cnt = piojo_graph_neighbor_cnt(vcur, graph);
                for (i = 0; i < cnt; ++i){
                        vneighbor = piojo_graph_neighbor_at(i, vcur, graph);
                        if (! is_visited_p(vneighbor, visiteds)){
                                piojo_stack_push(&vneighbor, st);
                                set_visited(TRUE, vneighbor, visiteds);
                        }
                }
        }
        piojo_stack_free(st);
        free_visiteds(visiteds);
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
        piojo_hash_node_t *next, node;
        piojo_graph_alist_t *alist;

        next = piojo_hash_first(graph->alists_by_vid, &node);
        while (next){
                alist = (piojo_graph_alist_t *) piojo_hash_entryv(next);
                piojo_array_free(alist->edges_by_vid);
                next = piojo_hash_next(next);
        }
}

static piojo_graph_alist_t*
vid_to_alist(piojo_graph_vid_t vertex, const piojo_graph_t *graph)
{
        return ((piojo_graph_alist_t*)
                piojo_hash_search(&vertex, graph->alists_by_vid));
}

static bool
vid_eq(const void *e1, const void *e2)
{
        piojo_graph_vid_t v1 = *(piojo_graph_vid_t*) e1;
        piojo_graph_vid_t v2 = *(piojo_graph_vid_t*) e2;
        if (v1 == v2){
                return TRUE;
        }
        return FALSE;
}

static piojo_hash_t*
alloc_visiteds(const piojo_graph_t *graph)
{
        piojo_alloc_kv_if ator = piojo_alloc_kv_default;

        ator.alloc_cb = graph->allocator.alloc_cb;
        ator.free_cb = graph->allocator.free_cb;
        return piojo_hash_alloc_cb_eq(sizeof(bool), vid_eq,
                                      sizeof(piojo_graph_vid_t), ator);
}

static void
free_visiteds(piojo_hash_t *visiteds)
{
        piojo_hash_free(visiteds);
}

static bool
is_visited_p(piojo_graph_vid_t vid, piojo_hash_t *visiteds)
{
        return (piojo_hash_search(&vid, visiteds) != NULL);
}

static void
set_visited(bool visited_p, piojo_graph_vid_t vid, piojo_hash_t *visiteds)
{
        if (visited_p){
                piojo_hash_set(&vid, NULL, visiteds);
        }else{
                piojo_hash_delete(&vid, visiteds);
        }
}
