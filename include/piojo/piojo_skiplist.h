/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 G. Elian Gidoni
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
 * Piojo Skip List API.
 */

#ifndef PIOJO_SKIPLIST_H_
#define PIOJO_SKIPLIST_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_skiplist_node_t piojo_skiplist_node_t;

typedef struct piojo_skiplist_t piojo_skiplist_t;
extern const size_t piojo_skiplist_sizeof;

piojo_skiplist_t*
piojo_skiplist_alloc_i32k(size_t evsize);

piojo_skiplist_t*
piojo_skiplist_alloc_i64k(size_t evsize);

piojo_skiplist_t*
piojo_skiplist_alloc_sizk(size_t evsize);

piojo_skiplist_t*
piojo_skiplist_alloc_cb_i32k(size_t evsize,
                             piojo_alloc_if allocator);

piojo_skiplist_t*
piojo_skiplist_alloc_cb_i64k(size_t evsize,
                             piojo_alloc_if allocator);

piojo_skiplist_t*
piojo_skiplist_alloc_cb_sizk(size_t evsize,
                             piojo_alloc_if allocator);

piojo_skiplist_t*
piojo_skiplist_alloc_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize);

piojo_skiplist_t*
piojo_skiplist_alloc_cb_cmp(size_t evsize,
                        piojo_cmp_cb keycmp, size_t eksize,
                        piojo_alloc_if allocator);

piojo_skiplist_t*
piojo_skiplist_copy(const piojo_skiplist_t *list);

void
piojo_skiplist_free(const piojo_skiplist_t *list);

void
piojo_skiplist_clear(piojo_skiplist_t *list);

size_t
piojo_skiplist_size(const piojo_skiplist_t *list);

bool
piojo_skiplist_insert(const void *key, const void *data, piojo_skiplist_t *list);

bool
piojo_skiplist_set(const void *key, const void *data, piojo_skiplist_t *list);

void*
piojo_skiplist_search(const void *key, const piojo_skiplist_t *list);

bool
piojo_skiplist_delete(const void *key, piojo_skiplist_t *list);

const void*
piojo_skiplist_first(const piojo_skiplist_t *list, void **data);

const void*
piojo_skiplist_last(const piojo_skiplist_t *list, void **data);

const void*
piojo_skiplist_next(const void *key, const piojo_skiplist_t *list, void **data);

const void*
piojo_skiplist_prev(const void *key, const piojo_skiplist_t *list, void **data);

#ifdef __cplusplus
}
#endif
#endif
