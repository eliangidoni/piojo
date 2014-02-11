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
 *
 * Piojo Graph API.
 */

/**
 * @file
 * @addtogroup piojograph
 */

#ifndef PIOJO_GRAPH_H_
#define PIOJO_GRAPH_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>
#include <piojo/piojo_hash.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_graph;
typedef struct piojo_graph piojo_graph_t;
extern const size_t piojo_graph_sizeof;

/** @{ */
/** Graph direction. */
typedef enum {
        /** Directed graph. */
        PIOJO_GRAPH_DIR_TRUE,
        /** Undirected graph. */
        PIOJO_GRAPH_DIR_FALSE
} piojo_graph_dir_t;

/** Vertex id. */
typedef uintptr_t piojo_graph_vid_t;

/** Edge weight. */
typedef int piojo_graph_weight_t;

/** Vertex visitor, returns @b TRUE to stop traversal, @b FALSE otherwise. */
typedef bool (*piojo_graph_visit_cb) (piojo_graph_vid_t v,
                                      const piojo_graph_t *graph,
                                      void *data);
/** @} */

piojo_graph_t*
piojo_graph_alloc(piojo_graph_dir_t directed);

piojo_graph_t*
piojo_graph_alloc_cb(piojo_graph_dir_t directed, piojo_alloc_if allocator);

piojo_graph_t*
piojo_graph_copy(const piojo_graph_t *graph);

void
piojo_graph_free(const piojo_graph_t *graph);

void
piojo_graph_clear(piojo_graph_t *graph);

bool
piojo_graph_insert(piojo_graph_vid_t vertex_id, piojo_graph_t *graph);

bool
piojo_graph_delete(piojo_graph_vid_t vertex, piojo_graph_t *graph);

void
piojo_graph_set_vvalue(const void *value, piojo_graph_vid_t vertex,
                       piojo_graph_t *graph);

void*
piojo_graph_vvalue(piojo_graph_vid_t vertex, const piojo_graph_t *graph);

void
piojo_graph_link(piojo_graph_weight_t weight, piojo_graph_vid_t from,
                 piojo_graph_vid_t to, piojo_graph_t *graph);

piojo_graph_weight_t*
piojo_graph_linked(piojo_graph_vid_t from, piojo_graph_vid_t to,
                   const piojo_graph_t *graph);

void
piojo_graph_unlink(piojo_graph_vid_t from, piojo_graph_vid_t to,
                   piojo_graph_t *graph);

void
piojo_graph_unlink_all(piojo_graph_vid_t vertex, piojo_graph_t *graph);

size_t
piojo_graph_neighbor_cnt(piojo_graph_vid_t vertex,
                         const piojo_graph_t *graph);

piojo_graph_vid_t
piojo_graph_neighbor_at(size_t idx, piojo_graph_vid_t vertex,
                        const piojo_graph_t *graph);

piojo_graph_weight_t
piojo_graph_edge_weight(size_t idx, piojo_graph_vid_t vertex,
                        const piojo_graph_t *graph);

bool
piojo_graph_vid_eq(const void *e1, const void *e2);

bool
piojo_graph_breadth_first(piojo_graph_vid_t root, piojo_graph_visit_cb cb,
                          const void *data, size_t limit,
                          const piojo_graph_t *graph);

bool
piojo_graph_depth_first(piojo_graph_vid_t root, piojo_graph_visit_cb cb,
                        const void *data, size_t limit,
                        const piojo_graph_t *graph);

void
piojo_graph_source_path(piojo_graph_vid_t root, const piojo_graph_t *graph,
                        piojo_hash_t *dists, piojo_hash_t *paths);

piojo_graph_weight_t
piojo_graph_pair_path(piojo_graph_vid_t root, piojo_graph_vid_t dst,
                      const piojo_graph_t *graph, piojo_hash_t *prevs);

bool
piojo_graph_neg_source_path(piojo_graph_vid_t root, const piojo_graph_t *graph,
                            piojo_hash_t *dists, piojo_hash_t *paths);

void
piojo_graph_min_tree(const piojo_graph_t *graph, piojo_graph_t *tree,
                     piojo_graph_weight_t *weight);

#ifdef __cplusplus
}
#endif
#endif
