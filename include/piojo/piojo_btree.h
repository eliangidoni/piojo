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

#ifndef PIOJO_BTREE_H_
#define PIOJO_BTREE_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        uint8_t opaque[32]; /* Should be enough */
} piojo_btree_node_t;

typedef struct piojo_btree_t piojo_btree_t;
extern const size_t piojo_btree_sizeof;

piojo_btree_t*
piojo_btree_alloc_intk(size_t evsize);

piojo_btree_t*
piojo_btree_alloc_i32k(size_t evsize);

piojo_btree_t*
piojo_btree_alloc_i64k(size_t evsize);

piojo_btree_t*
piojo_btree_alloc_ptrk(size_t evsize);

piojo_btree_t*
piojo_btree_alloc_sizk(size_t evsize);

piojo_btree_t*
piojo_btree_alloc_cb_intk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_btree_t*
piojo_btree_alloc_cb_i32k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_btree_t*
piojo_btree_alloc_cb_i64k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_btree_t*
piojo_btree_alloc_cb_ptrk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_btree_t*
piojo_btree_alloc_cb_sizk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator);

piojo_btree_t*
piojo_btree_alloc_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize);

piojo_btree_t*
piojo_btree_alloc_cb_cmp(uint8_t maxchildren, size_t evsize,
                        piojo_cmp_cb keycmp, size_t eksize,
                        piojo_alloc_kv_if allocator);

piojo_btree_t*
piojo_btree_copy(const piojo_btree_t *tree);

void
piojo_btree_free(const piojo_btree_t *tree);

void
piojo_btree_clear(piojo_btree_t *tree);

size_t
piojo_btree_size(const piojo_btree_t *tree);

bool
piojo_btree_insert(const void *key, const void *data, piojo_btree_t *tree);

bool
piojo_btree_set(const void *key, const void *data, piojo_btree_t *tree);

void*
piojo_btree_search(const void *key, const piojo_btree_t *tree);

bool
piojo_btree_delete(const void *key, piojo_btree_t *tree);

piojo_btree_node_t*
piojo_btree_first(const piojo_btree_t *tree, piojo_btree_node_t *node);

piojo_btree_node_t*
piojo_btree_last(const piojo_btree_t *tree, piojo_btree_node_t *node);

piojo_btree_node_t*
piojo_btree_next(piojo_btree_node_t *node);

piojo_btree_node_t*
piojo_btree_prev(piojo_btree_node_t *node);

const void*
piojo_btree_entryk(const piojo_btree_node_t *node);

void*
piojo_btree_entryv(const piojo_btree_node_t *node);

#ifdef __cplusplus
}
#endif
#endif
