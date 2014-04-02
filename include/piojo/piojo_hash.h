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
 * Piojo Hash Table API.
 */

#ifndef PIOJO_HASH_H_
#define PIOJO_HASH_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        uint8_t opaque[32]; /* Should be enough */
} piojo_hash_node_t;

struct piojo_hash;
typedef struct piojo_hash piojo_hash_t;
extern const size_t piojo_hash_sizeof;

piojo_hash_t*
piojo_hash_alloc_intk(size_t evsize);

piojo_hash_t*
piojo_hash_alloc_i32k(size_t evsize);

piojo_hash_t*
piojo_hash_alloc_i64k(size_t evsize);

piojo_hash_t*
piojo_hash_alloc_ptrk(size_t evsize);

piojo_hash_t*
piojo_hash_alloc_sizk(size_t evsize);

piojo_hash_t*
piojo_hash_alloc_cb_intk(size_t evsize, piojo_alloc_kv_if allocator);

piojo_hash_t*
piojo_hash_alloc_cb_i32k(size_t evsize, piojo_alloc_kv_if allocator);

piojo_hash_t*
piojo_hash_alloc_cb_i64k(size_t evsize, piojo_alloc_kv_if allocator);

piojo_hash_t*
piojo_hash_alloc_cb_ptrk(size_t evsize, piojo_alloc_kv_if allocator);

piojo_hash_t*
piojo_hash_alloc_cb_sizk(size_t evsize, piojo_alloc_kv_if allocator);

piojo_hash_t*
piojo_hash_alloc_eq(size_t evsize, piojo_eq_cb keyeq, size_t eksize);

piojo_hash_t*
piojo_hash_alloc_cb_eq(size_t evsize, piojo_eq_cb keyeq, size_t eksize,
                       piojo_alloc_kv_if allocator);

piojo_hash_t*
piojo_hash_copy(const piojo_hash_t *hash);

void
piojo_hash_free(const piojo_hash_t *hash);

void
piojo_hash_clear(piojo_hash_t *hash);

size_t
piojo_hash_size(const piojo_hash_t *hash);

bool
piojo_hash_insert(const void *key, const void *data, piojo_hash_t *hash);

bool
piojo_hash_set(const void *key, const void *data, piojo_hash_t *hash);

void*
piojo_hash_search(const void *key, const piojo_hash_t *hash);

bool
piojo_hash_delete(const void *key, piojo_hash_t *hash);

piojo_hash_node_t*
piojo_hash_first(const piojo_hash_t *hash, piojo_hash_node_t *node);

piojo_hash_node_t*
piojo_hash_next(piojo_hash_node_t *node);

const void*
piojo_hash_entryk(const piojo_hash_node_t *node);

void*
piojo_hash_entryv(const piojo_hash_node_t *node);

#ifdef __cplusplus
}
#endif
#endif
