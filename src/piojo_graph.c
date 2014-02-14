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
#include <piojo/piojo_diset.h>
#include <piojo/piojo_stack.h>
#include <piojo/piojo_queue.h>
#include <piojo/piojo_heap.h>
#include <piojo_defs.h>

typedef struct {
        piojo_array_t *edges_by_vid;
        piojo_graph_vid_t vid;          /* Vertex identifier. */
        piojo_opaque_t data;            /* Optional user data. */
} piojo_graph_alist_t;

typedef struct {
        piojo_graph_weight_t weight;
        piojo_graph_vid_t beg_vid, end_vid;
} piojo_graph_edge_t;

struct piojo_graph {
        piojo_hash_t *alists_by_vid;
        piojo_graph_dir_t dir;
        piojo_opaque_t data;            /* User data. */
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of graph in bytes */
const size_t piojo_graph_sizeof = sizeof(piojo_graph_t);

/* Used to avoid weight sum overflows. */
typedef unsigned int piojo_graph_uweight_t;

/* Used by DFS/BFS functions. */
typedef struct {
        piojo_graph_vid_t vid;
        size_t depth;
} piojo_graph_vtx_t;

static const size_t DEFAULT_EDGE_COUNT = 8;
static const piojo_graph_uweight_t WEIGHT_MAX = INT_MAX;

static void
link_vertices(piojo_graph_weight_t weight, piojo_graph_alist_t *from,
              piojo_graph_alist_t *to);

static void
unlink_vertices(piojo_graph_alist_t *from, piojo_graph_alist_t *to);

static int
edge_cmp(const void *e1, const void *e2);

static void
free_edges(const piojo_graph_t *graph);

static piojo_graph_alist_t*
vid_to_alist(piojo_graph_vid_t vertex, const piojo_graph_t *graph);

static piojo_hash_t*
alloc_visiteds(const piojo_graph_t *graph);

static void
free_visiteds(piojo_hash_t *visiteds);

static bool
is_visited_p(piojo_graph_vid_t vid, piojo_hash_t *visiteds);

static void
mark_visited(piojo_graph_vid_t vid, piojo_hash_t *visiteds);

static piojo_heap_t*
alloc_prioq(const piojo_graph_t *graph);

static void
free_prioq(piojo_heap_t *prioq);

static void
insert_prioq(piojo_opaque_t data, piojo_graph_weight_t dist,
             piojo_heap_t *prioq);

static void
update_prioq(piojo_opaque_t data, piojo_graph_weight_t dist,
             piojo_heap_t *prioq);

static bool
in_prioq(piojo_opaque_t data, piojo_heap_t *prioq);

static piojo_opaque_t
del_min_prioq(piojo_heap_t *prioq);

static bool
empty_prioq_p(piojo_heap_t *prioq);

static void
dijkstra_search(piojo_graph_vid_t root, const piojo_graph_vid_t *dst,
                const piojo_graph_t *graph, piojo_hash_t *dists,
                piojo_hash_t *prevs);

static void
dijkstra_relax(piojo_graph_vid_t bestv, const piojo_graph_t *graph,
               piojo_heap_t *prioq, piojo_hash_t *dists, piojo_hash_t *prevs);

static bool
bellman_ford_relax(piojo_array_t *edges, bool find_cycle_p, piojo_hash_t *dists,
                   piojo_hash_t *prevs, bool *relaxed_p);

static void
a_star_relax(piojo_graph_vid_t bestv, piojo_graph_vid_t dst,
             piojo_graph_cost_cb h, const piojo_graph_t *graph,
             piojo_hash_t *gscores, piojo_hash_t *closedset,
             piojo_heap_t *openset, piojo_hash_t *prevs);

/**
 * Allocates a new graph.
 * Uses default allocator.
 * @param[in] directed Graph direction.
 * @param[in] data User-defined data passed to graph callbacks.
 * @return New graph.
 */
piojo_graph_t*
piojo_graph_alloc(piojo_graph_dir_t directed, piojo_opaque_t data)
{
        return piojo_graph_alloc_cb(directed, data, piojo_alloc_default);
}

/**
 * Allocates a new graph.
 * @param[in] directed Graph direction.
 * @param[in] data User-defined data passed to graph callbacks.
 * @param[in] allocator Allocator to be used.
 * @return New graph.
 */
piojo_graph_t*
piojo_graph_alloc_cb(piojo_graph_dir_t directed, piojo_opaque_t data,
                     piojo_alloc_if allocator)
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
        graph->data = data;
        graph->alists_by_vid = piojo_hash_alloc_cb_eq(esize,
                                                      piojo_graph_vid_eq,
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

        /* Copy all edges for each vertex. */
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
 * @param[in] vertex
 * @param[out] graph
 * @return @b TRUE if inserted, @b FALSE if it's duplicated.
 */
bool
piojo_graph_insert(piojo_graph_vid_t vertex, piojo_graph_t *graph)
{
        piojo_graph_alist_t tmp;
        PIOJO_ASSERT(graph);

        tmp.vid = vertex;
        tmp.data = 0;
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
        alist->data = 0;

        piojo_graph_unlink_all(vertex, graph);
        piojo_array_free(alist->edges_by_vid);

        return piojo_hash_delete(&alist->vid, graph->alists_by_vid);
}

/**
 * Sets user-defined @a vertex value (default is @b 0).
 * @param[in] value
 * @param[in] vertex
 * @param[out] graph
 */
void
piojo_graph_set_vvalue(piojo_opaque_t value, piojo_graph_vid_t vertex,
                       piojo_graph_t *graph)
{
        piojo_graph_alist_t *alist;
        PIOJO_ASSERT(graph);

        alist = vid_to_alist(vertex, graph);
        alist->data = value;
}

/**
 * Returns user-defined @a vertex value (default is @b 0).
 * @param[in] vertex
 * @param[in] graph
 * @return Vertex value.
 */
piojo_opaque_t
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
piojo_graph_link(piojo_graph_weight_t weight, piojo_graph_vid_t from,
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
 * Returns edge weight if an edge exists between two vertices.
 * @param[in] from Source vertex.
 * @param[in] to Destination vertex.
 * @param[in] graph
 * @return @b Pointer to edge weight if an edge exists, @b NULL otherwise.
 */
piojo_graph_weight_t*
piojo_graph_linked(piojo_graph_vid_t from, piojo_graph_vid_t to,
                   const piojo_graph_t *graph)
{
        piojo_graph_edge_t edge;
        piojo_graph_alist_t *v;
        piojo_graph_edge_t *e;
        size_t idx;
        PIOJO_ASSERT(graph);

        v = vid_to_alist(from, graph);
        edge.end_vid = to;
        if (piojo_array_has_p(&edge, v->edges_by_vid, &idx)){
                e = (piojo_graph_edge_t *)piojo_array_at(idx,
                                                         v->edges_by_vid);
                return &e->weight;
        }
        return NULL;
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
piojo_graph_weight_t
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
 * Vertex equality function.
 * @param[in] e1 Vertex pointer.
 * @param[in] e2 Vertex pointer.
 * @return @b TRUE if @a e1 is equal to @a e2, @b FALSE otherwise.
 */
bool
piojo_graph_vid_eq(const void *e1, const void *e2)
{
        return piojo_id_eq(e1, e2);
}

/**
 * Traverses @a graph following a breadth first search.
 * @param[in] root Starting vertex.
 * @param[in] cb Vertex visit function.
 * @param[in] limit Depth limit or @b 0 for no limit.
 * @param[in] graph
 * @return Value returned by @a cb.
 */
bool
piojo_graph_breadth_first(piojo_graph_vid_t root, piojo_graph_visit_cb cb,
                          size_t limit, const piojo_graph_t *graph)
{
        piojo_queue_t *q;
        piojo_hash_t *visiteds;
        size_t i, cnt;
        piojo_graph_vtx_t vcur, nbor;
        piojo_graph_alist_t *v;
        bool ret = FALSE, limited_p = (limit != 0);

        q = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE,
                                 sizeof(piojo_graph_vtx_t),
                                 graph->allocator);
        visiteds = alloc_visiteds(graph);

        vcur.vid = root;
        vcur.depth = 0;
        piojo_queue_push(&vcur, q);
        mark_visited(vcur.vid, visiteds);
        while (piojo_queue_size(q) > 0){
                vcur = *(piojo_graph_vtx_t*) piojo_queue_peek(q);
                piojo_queue_pop(q);
                if (cb(vcur.vid, graph, graph->data)){
                        ret = TRUE;
                        break;
                }
                nbor.depth = vcur.depth + 1;
                if (limited_p && nbor.depth > limit){
                        continue;
                }
                v = vid_to_alist(vcur.vid, graph);
                cnt = piojo_array_size(v->edges_by_vid);
                for (i = 0; i < cnt; ++i){
                        nbor.vid = ((piojo_graph_edge_t *)
                                    piojo_array_at(i,v->edges_by_vid))->end_vid;
                        if (! is_visited_p(nbor.vid, visiteds)){
                                piojo_queue_push(&nbor, q);
                                mark_visited(nbor.vid, visiteds);
                        }
                }
        }

        free_visiteds(visiteds);
        piojo_queue_free(q);
        return ret;
}

/**
 * Traverses @a graph following a depth first search.
 * @param[in] root Starting vertex.
 * @param[in] cb Vertex visit function.
 * @param[in] limit Depth limit or @b 0 for no limit.
 * @param[in] graph
 * @return Value returned by @a cb.
 */
bool
piojo_graph_depth_first(piojo_graph_vid_t root, piojo_graph_visit_cb cb,
                        size_t limit, const piojo_graph_t *graph)
{
        piojo_stack_t *st;
        piojo_hash_t *visiteds;
        size_t i, cnt;
        piojo_graph_vtx_t vcur, nbor;
        piojo_graph_alist_t *v;
        bool ret = FALSE, limited_p = (limit != 0);

        st = piojo_stack_alloc_cb(sizeof(piojo_graph_vtx_t), graph->allocator);
        visiteds = alloc_visiteds(graph);

        vcur.vid = root;
        vcur.depth = 0;
        piojo_stack_push(&vcur, st);
        mark_visited(vcur.vid, visiteds);
        while (piojo_stack_size(st) > 0){
                vcur = *(piojo_graph_vtx_t*) piojo_stack_peek(st);
                piojo_stack_pop(st);
                if (cb(vcur.vid, graph, graph->data)){
                        ret = TRUE;
                        break;
                }
                nbor.depth = vcur.depth + 1;
                if (limited_p && nbor.depth > limit){
                        continue;
                }
                v = vid_to_alist(vcur.vid, graph);
                cnt = piojo_array_size(v->edges_by_vid);
                for (i = 0; i < cnt; ++i){
                        nbor.vid = ((piojo_graph_edge_t *)
                                    piojo_array_at(i,v->edges_by_vid))->end_vid;
                        if (! is_visited_p(nbor.vid, visiteds)){
                                piojo_stack_push(&nbor, st);
                                mark_visited(nbor.vid, visiteds);
                        }
                }
        }

        free_visiteds(visiteds);
        piojo_stack_free(st);
        return ret;
}

/**
 * Finds shortest path from @a root to all vertices (Dijkstra's algorithm).
 * @warning The graph can't have negative edge weights.
 * @param[in] root Starting vertex.
 * @param[in] graph
 * @param[out] dists Distance (weight sum) for each vertex (if a path exists).
 * @param[out] prevs Previous vertex in path for each vertex (if a path exists),
 *                   can be @b NULL.
 */
void
piojo_graph_source_path(piojo_graph_vid_t root, const piojo_graph_t *graph,
                        piojo_hash_t *dists, piojo_hash_t *prevs)
{
        PIOJO_ASSERT(graph);
        PIOJO_ASSERT(dists);
        PIOJO_ASSERT(sizeof(piojo_graph_uweight_t) ==
                     sizeof(piojo_graph_weight_t));

        dijkstra_search(root, NULL, graph, dists, prevs);
}

/**
 * Finds shortest path from @a root to @a dst vertex (Dijkstra's algorithm).
 * @warning The graph can't have negative edge weights.
 * @warning @a root must be different from @a dst.
 * @param[in] root Starting vertex.
 * @param[in] dst Destination vertex.
 * @param[in] graph
 * @param[out] prevs Previous vertex in path for each vertex (if a path exists),
 *                   can be @b NULL.
 * @return Distance (weight sum) to @a dst or @b 0 if there isn't any path.
 */
piojo_graph_weight_t
piojo_graph_pair_path(piojo_graph_vid_t root, piojo_graph_vid_t dst,
                      const piojo_graph_t *graph, piojo_hash_t *prevs)
{
        piojo_hash_t *dists;
        piojo_graph_weight_t w, *wp;
        PIOJO_ASSERT(graph);
        PIOJO_ASSERT(root != dst);
        PIOJO_ASSERT(sizeof(piojo_graph_uweight_t) ==
                     sizeof(piojo_graph_weight_t));

        dists = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        dijkstra_search(root, &dst, graph, dists, prevs);
        wp = (piojo_graph_weight_t *)piojo_hash_search(&dst, dists);
        w = (wp != NULL) ? *wp : 0;

        piojo_hash_free(dists);
        return w;
}

/**
 * Finds shortest path from @a root to all vertices (Bellman-Ford algorithm).
 * @warning If the graph have negative cycles reachable from @a root,
 *          shortest paths won't be found.
 * @warning If the graph is undirected then any edge with negative weight forms
 *          a negative cycle.
 * @param[in] root Starting vertex.
 * @param[in] graph
 * @param[out] dists Distance (weight sum) for each vertex (if a path exists).
 * @param[out] prevs Previous vertex in path for each vertex (if a path exists),
 *                   can be @b NULL.
 * @return @b TRUE if @a root reaches a negative cycle, @b FALSE otherwise.
 */
bool
piojo_graph_neg_source_path(piojo_graph_vid_t root, const piojo_graph_t *graph,
                            piojo_hash_t *dists, piojo_hash_t *prevs)
{
        piojo_graph_weight_t dist=0;
        piojo_graph_edge_t *e;
        piojo_graph_alist_t *v;
        piojo_array_t *edges;
        piojo_hash_node_t *next, node;
        size_t i, ecnt, vcnt;
        bool found_cycle_p = FALSE, relaxed_p = TRUE;
        PIOJO_ASSERT(graph);
        PIOJO_ASSERT(dists);

        piojo_hash_set(&root, &dist, dists);
        edges = piojo_array_alloc_cb(NULL, sizeof(piojo_graph_edge_t*),
                                     graph->allocator);

        /* Insert every edge to edges array. */
        next = piojo_hash_first(graph->alists_by_vid, &node);
        while (next){
                v = (piojo_graph_alist_t *) piojo_hash_entryv(next);
                ecnt = piojo_array_size(v->edges_by_vid);
                for (i = 0; i < ecnt; ++i){
                        e = ((piojo_graph_edge_t *)
                             piojo_array_at(i, v->edges_by_vid));
                        piojo_array_append(&e, edges);
                }
                next = piojo_hash_next(next);
        }

        /* Relax every edge for at most |V| times. */
        vcnt = piojo_hash_size(graph->alists_by_vid);
        while (vcnt-- > 1 && relaxed_p){
                bellman_ford_relax(edges, FALSE, dists, prevs, &relaxed_p);
        }
        if (relaxed_p){
                found_cycle_p = bellman_ford_relax(edges, TRUE, dists,
                                                   prevs, &relaxed_p);
        }

        piojo_array_free(edges);
        return found_cycle_p;
}

/**
 * Finds minimum spanning tree when @a graph is connected. Otherwise,
 * finds the minimum spanning forest (Kruskal's algorithm).
 * @warning The graph must be undirected.
 * @param[in] graph
 * @param[out] tree Minimum spanning tree/forest of @a graph.
 * @return Tree weight or @b 0 for an empty graph.
 */
piojo_graph_weight_t
piojo_graph_min_tree(const piojo_graph_t *graph, piojo_graph_t *tree)
{
        piojo_heap_t *prioq;
        piojo_diset_t *diset;
        piojo_graph_alist_t *v;
        piojo_graph_edge_t *e;
        piojo_hash_node_t *next, node;
        size_t ecnt, i;
        piojo_graph_weight_t w=0;
        PIOJO_ASSERT(graph);
        PIOJO_ASSERT(graph->dir == PIOJO_GRAPH_DIR_FALSE);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(sizeof(piojo_graph_uweight_t) ==
                     sizeof(piojo_graph_weight_t));

        if (piojo_hash_size(graph->alists_by_vid) == 0){
                return 0;
        }

        prioq = alloc_prioq(graph);
        diset = piojo_diset_alloc_cb(graph->allocator);

        next = piojo_hash_first(graph->alists_by_vid, &node);
        while (next){
                v = (piojo_graph_alist_t *) piojo_hash_entryv(next);
                piojo_graph_insert(v->vid, tree);
                piojo_diset_insert(v->vid, diset);

                ecnt = piojo_array_size(v->edges_by_vid);
                for (i = 0; i < ecnt; ++i){
                        e = ((piojo_graph_edge_t *)
                             piojo_array_at(i, v->edges_by_vid));
                        insert_prioq((piojo_opaque_t)e, e->weight, prioq);
                }
                next = piojo_hash_next(next);
        }

        while (! empty_prioq_p(prioq)){
                e = (piojo_graph_edge_t *)del_min_prioq(prioq);
                if (piojo_diset_find(e->beg_vid, diset) !=
                    piojo_diset_find(e->end_vid, diset)){
                        piojo_diset_union(e->beg_vid, e->end_vid, diset);
                        piojo_graph_link(e->weight, e->beg_vid, e->end_vid,
                                         tree);
                        w += e->weight;
                }
        }

        piojo_diset_free(diset);
        free_prioq(prioq);
        return w;
}

/**
 * Finds shortest path from @a root to @a dst vertex using A* algorithm.
 * @warning The graph can't have negative edge weights.
 * @warning @a root must be different from @a dst.
 * @param[in] root Starting vertex.
 * @param[in] dst Destination vertex.
 * @param[in] heuristic Cost estimate function (should be consistent).
 * @param[in] graph
 * @param[out] prevs Previous vertex in path for each vertex (if a path exists),
 *                   can be @b NULL.
 * @return Distance (weight sum) to @a dst or @b 0 if there isn't any path.
 */
piojo_graph_weight_t
piojo_graph_a_star(piojo_graph_vid_t root, piojo_graph_vid_t dst,
                   piojo_graph_cost_cb heuristic, const piojo_graph_t *graph,
                   piojo_hash_t *prevs)
{
        piojo_graph_vid_t bestv;
        piojo_graph_weight_t dist=0, *wp;
        piojo_hash_t *closedset, *gscores;
        piojo_heap_t *openset;

        gscores = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                      piojo_graph_vid_eq,
                                      sizeof(piojo_graph_vid_t));
        closedset = alloc_visiteds(graph);
        openset = alloc_prioq(graph);

        piojo_hash_insert(&root, &dist, gscores);
        dist = heuristic(root, dst, graph, graph->data);
        PIOJO_ASSERT(dist >= 0);

        /* Relax the nearest vertex on each iteration. */
        insert_prioq((piojo_opaque_t)root, dist, openset);
        while (! empty_prioq_p(openset)){
                bestv = (piojo_graph_vid_t)del_min_prioq(openset);
                if (bestv == dst){
                        break;
                }
                /* Add vertex to closed set, heuristic is consistent. */
                mark_visited(bestv, closedset);
                a_star_relax(bestv, dst, heuristic, graph, gscores,
                             closedset, openset, prevs);
        }
        wp = (piojo_graph_weight_t *)piojo_hash_search(&dst, gscores);
        dist = (wp != NULL) ? *wp : 0;

        free_prioq(openset);
        free_visiteds(closedset);
        piojo_hash_free(gscores);
        return dist;
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
link_vertices(piojo_graph_weight_t weight, piojo_graph_alist_t *from,
              piojo_graph_alist_t *to)
{
        bool linked_p;
        size_t idx;
        piojo_graph_edge_t edge, *e;

        edge.weight = weight;
        edge.beg_vid = from->vid;
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

static piojo_hash_t*
alloc_visiteds(const piojo_graph_t *graph)
{
        piojo_alloc_kv_if ator = piojo_alloc_kv_default;

        ator.alloc_cb = graph->allocator.alloc_cb;
        ator.free_cb = graph->allocator.free_cb;
        return piojo_hash_alloc_cb_eq(sizeof(bool), piojo_graph_vid_eq,
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
mark_visited(piojo_graph_vid_t vid, piojo_hash_t *visiteds)
{
        piojo_hash_insert(&vid, NULL, visiteds);
}

static piojo_heap_t*
alloc_prioq(const piojo_graph_t *graph)
{
        piojo_alloc_if ator = piojo_alloc_default;

        ator.alloc_cb = graph->allocator.alloc_cb;
        ator.free_cb = graph->allocator.free_cb;
        return piojo_heap_alloc_cb(ator);
}

static void
free_prioq(piojo_heap_t *prioq)
{
        piojo_heap_free(prioq);
}

static void
insert_prioq(piojo_opaque_t data, piojo_graph_weight_t dist,
             piojo_heap_t *prioq)
{
        piojo_heap_push(data, dist, prioq);
}

static void
update_prioq(piojo_opaque_t data, piojo_graph_weight_t dist,
             piojo_heap_t *prioq)
{
        piojo_heap_decrease(data, dist, prioq);
}

static bool
in_prioq(piojo_opaque_t data, piojo_heap_t *prioq)
{
        return piojo_heap_has_p(data, prioq);
}

static piojo_opaque_t
del_min_prioq(piojo_heap_t *prioq)
{
        piojo_opaque_t data = piojo_heap_peek(prioq);
        piojo_heap_pop(prioq);
        return data;
}

static bool
empty_prioq_p(piojo_heap_t *prioq)
{
        return (piojo_heap_size(prioq) == 0);
}

static void
dijkstra_search(piojo_graph_vid_t root, const piojo_graph_vid_t *dst,
                const piojo_graph_t *graph, piojo_hash_t *dists,
                piojo_hash_t *prevs)
{
        piojo_graph_vid_t bestv;
        piojo_hash_t *visiteds;
        piojo_graph_weight_t dist=0;
        piojo_heap_t *prioq;

        piojo_hash_set(&root, &dist, dists);
        visiteds = alloc_visiteds(graph);
        prioq = alloc_prioq(graph);

        /* Relax the nearest (unvisited) vertex on each iteration. */
        insert_prioq((piojo_opaque_t)root, dist, prioq);
        while (! empty_prioq_p(prioq)){
                bestv = (piojo_graph_vid_t)del_min_prioq(prioq);
                if (dst != NULL && bestv == *dst){
                        break;
                }
                if (! is_visited_p(bestv, visiteds)){
                        dijkstra_relax(bestv, graph, prioq, dists, prevs);
                        mark_visited(bestv, visiteds);
                }
        }

        free_prioq(prioq);
        free_visiteds(visiteds);
}

static void
dijkstra_relax(piojo_graph_vid_t bestv, const piojo_graph_t *graph,
               piojo_heap_t *prioq, piojo_hash_t *dists, piojo_hash_t *prevs)
{
        size_t i, cnt;
        piojo_graph_weight_t *bestw;
        piojo_graph_uweight_t ndist, *vdist;
        piojo_graph_edge_t *e;
        piojo_graph_alist_t *v;

        bestw = (piojo_graph_weight_t *) piojo_hash_search(&bestv, dists);
        PIOJO_ASSERT(bestw != NULL && *bestw >= 0);

        v = vid_to_alist(bestv, graph);
        cnt = piojo_array_size(v->edges_by_vid);
        for (i = 0; i < cnt; ++i){
                e = (piojo_graph_edge_t *) piojo_array_at(i, v->edges_by_vid);
                PIOJO_ASSERT(e->weight >= 0);

                ndist = ((piojo_graph_uweight_t) e->weight +
                         (piojo_graph_uweight_t) *bestw);
                if (ndist > WEIGHT_MAX){
                        ndist = WEIGHT_MAX;
                }

                vdist = ((piojo_graph_uweight_t *)
                         piojo_hash_search(&e->end_vid, dists));
                if (vdist == NULL || ndist < *vdist){
                        piojo_hash_set(&e->end_vid, &ndist, dists);
                        if (prevs != NULL){
                                piojo_hash_set(&e->end_vid, &v->vid, prevs);
                        }
                        if (vdist == NULL){
                                insert_prioq((piojo_opaque_t)e->end_vid, ndist,
                                             prioq);
                        }else{
                                update_prioq((piojo_opaque_t)e->end_vid, ndist,
                                             prioq);
                        }
                }
        }
}

static bool
bellman_ford_relax(piojo_array_t *edges, bool find_cycle_p, piojo_hash_t *dists,
                   piojo_hash_t *prevs, bool *relaxed_p)
{
        piojo_graph_edge_t *edge;
        piojo_graph_weight_t *end_dist, *beg_dist, new_dist;
        size_t i, ecnt;

        *relaxed_p = FALSE;
        ecnt = piojo_array_size(edges);
        for (i = 0; i < ecnt; ++i){
                edge = *(piojo_graph_edge_t **)piojo_array_at(i, edges);
                beg_dist = ((piojo_graph_weight_t *)
                            piojo_hash_search(&edge->beg_vid, dists));
                end_dist = ((piojo_graph_weight_t *)
                            piojo_hash_search(&edge->end_vid, dists));

                if (beg_dist != NULL &&
                    (end_dist == NULL || *beg_dist + edge->weight < *end_dist)){
                        if (find_cycle_p){
                                return TRUE;
                        }
                        *relaxed_p = TRUE;

                        new_dist = *beg_dist + edge->weight;
                        piojo_hash_set(&edge->end_vid, &new_dist, dists);

                        if (prevs != NULL){
                                piojo_hash_set(&edge->end_vid, &edge->beg_vid,
                                               prevs);
                        }
                }
        }
        return FALSE;
}

static void
a_star_relax(piojo_graph_vid_t bestv, piojo_graph_vid_t dst,
             piojo_graph_cost_cb h, const piojo_graph_t *graph,
             piojo_hash_t *gscores, piojo_hash_t *closedset,
             piojo_heap_t *openset, piojo_hash_t *prevs)
{
        size_t i, cnt;
        bool open_p;
        piojo_graph_weight_t *bestw, hw;
        piojo_graph_uweight_t ndist, *vdist, fscore;
        piojo_graph_edge_t *e;
        piojo_graph_alist_t *v;

        bestw = (piojo_graph_weight_t *) piojo_hash_search(&bestv, gscores);
        PIOJO_ASSERT(bestw != NULL && *bestw >= 0);

        v = vid_to_alist(bestv, graph);
        cnt = piojo_array_size(v->edges_by_vid);
        for (i = 0; i < cnt; ++i){
                e = (piojo_graph_edge_t *) piojo_array_at(i, v->edges_by_vid);
                if (is_visited_p(e->end_vid, closedset)){
                        continue;
                }
                PIOJO_ASSERT(e->weight >= 0);

                ndist = ((piojo_graph_uweight_t) e->weight +
                         (piojo_graph_uweight_t) *bestw);
                if (ndist > WEIGHT_MAX){
                        ndist = WEIGHT_MAX;
                }

                vdist = ((piojo_graph_uweight_t *)
                         piojo_hash_search(&e->end_vid, gscores));

                open_p = in_prioq((piojo_opaque_t)e->end_vid, openset);
                if (! open_p || (vdist == NULL || ndist < *vdist)){
                        piojo_hash_set(&e->end_vid, &ndist, gscores);
                        if (prevs != NULL){
                                piojo_hash_set(&e->end_vid, &v->vid, prevs);
                        }
                        hw = h(e->end_vid, dst, graph, graph->data);
                        PIOJO_ASSERT(hw >= 0);
                        fscore = ndist + (piojo_graph_uweight_t) hw;
                        if (fscore > WEIGHT_MAX){
                                fscore = WEIGHT_MAX;
                        }
                        if (! open_p){
                                insert_prioq((piojo_opaque_t)e->end_vid, fscore,
                                             openset);
                        }else{
                                update_prioq((piojo_opaque_t)e->end_vid, fscore,
                                             openset);
                        }
                }
        }
}
