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

#include <piojo/piojo_common.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        uint8_t opaque[8];
} piojo_graph_vertex_t;

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

piojo_graph_vertex_t
piojo_graph_insert(piojo_graph_t *graph);

void
piojo_graph_delete(piojo_graph_vertex_t vertex, piojo_graph_t *graph);

void
piojo_graph_set_vvalue(const void *value, piojo_graph_vertex_t vertex,
                       piojo_graph_t *graph);

void*
piojo_graph_vvalue(piojo_graph_vertex_t vertex, const piojo_graph_t *graph);

void
piojo_graph_link(int weight, piojo_graph_vertex_t from,
                 piojo_graph_vertex_t to, piojo_graph_t *graph);

bool
piojo_graph_link_p(piojo_graph_vertex_t from, piojo_graph_vertex_t to,
                   const piojo_graph_t *graph);

void
piojo_graph_unlink(piojo_graph_vertex_t from, piojo_graph_vertex_t to,
                   piojo_graph_t *graph);

void
piojo_graph_unlink_all(piojo_graph_vertex_t vertex, piojo_graph_t *graph);

size_t
piojo_graph_neighbor_cnt(piojo_graph_vertex_t vertex,
                         const piojo_graph_t *graph);

piojo_graph_vertex_t
piojo_graph_neighbor_at(size_t idx, piojo_graph_vertex_t vertex,
                        const piojo_graph_t *graph);

int
piojo_graph_neighbor_w(size_t idx, piojo_graph_vertex_t vertex,
                       const piojo_graph_t *graph);

#ifdef __cplusplus
}
#endif
#endif
