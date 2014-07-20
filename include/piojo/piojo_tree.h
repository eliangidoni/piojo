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
 * Piojo B-tree API.
 */

#ifndef PIOJO_TREE_H_
#define PIOJO_TREE_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        uint8_t opaque[32]; /* Should be enough */
} piojo_tree_node_t;

typedef struct piojo_tree_t piojo_tree_t;
extern const size_t piojo_tree_sizeof;

piojo_tree_t*
piojo_tree_alloc_intk(size_t evsize);

piojo_tree_t*
piojo_tree_alloc_i32k(size_t evsize);

piojo_tree_t*
piojo_tree_alloc_i64k(size_t evsize);

piojo_tree_t*
piojo_tree_alloc_ptrk(size_t evsize);

piojo_tree_t*
piojo_tree_alloc_sizk(size_t evsize);

piojo_tree_t*
piojo_tree_alloc_cb_intk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_tree_t*
piojo_tree_alloc_cb_i32k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_tree_t*
piojo_tree_alloc_cb_i64k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_tree_t*
piojo_tree_alloc_cb_ptrk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_tree_t*
piojo_tree_alloc_cb_sizk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_tree_t*
piojo_tree_alloc_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize);

piojo_tree_t*
piojo_tree_alloc_cb_cmp(uint8_t maxchildren, size_t evsize,
                        piojo_cmp_cb keycmp, size_t eksize,
                        piojo_alloc_kv_if allocator);

piojo_tree_t*
piojo_tree_copy(const piojo_tree_t *tree);

void
piojo_tree_free(const piojo_tree_t *tree);

void
piojo_tree_clear(piojo_tree_t *tree);

size_t
piojo_tree_size(const piojo_tree_t *tree);

bool
piojo_tree_insert(const void *key, const void *data, piojo_tree_t *tree);

bool
piojo_tree_set(const void *key, const void *data, piojo_tree_t *tree);

void*
piojo_tree_search(const void *key, const piojo_tree_t *tree);

bool
piojo_tree_delete(const void *key, piojo_tree_t *tree);

piojo_tree_node_t*
piojo_tree_first(const piojo_tree_t *tree, piojo_tree_node_t *node);

piojo_tree_node_t*
piojo_tree_last(const piojo_tree_t *tree, piojo_tree_node_t *node);

piojo_tree_node_t*
piojo_tree_next(piojo_tree_node_t *node);

piojo_tree_node_t*
piojo_tree_prev(piojo_tree_node_t *node);

const void*
piojo_tree_entryk(const piojo_tree_node_t *node);

void*
piojo_tree_entryv(const piojo_tree_node_t *node);

#ifdef __cplusplus
}
#endif
#endif
