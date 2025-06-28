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
 * Piojo Bloom Filter API.
 */

#ifndef PIOJO_BLOOM_H_
#define PIOJO_BLOOM_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_bloom_t piojo_bloom_t;
extern const size_t piojo_bloom_sizeof;

piojo_bloom_t*
piojo_bloom_alloc_i32k(size_t capacity, float false_positive_rate);

piojo_bloom_t*
piojo_bloom_alloc_i64k(size_t capacity, float false_positive_rate);

piojo_bloom_t*
piojo_bloom_alloc_sizk(size_t capacity, float false_positive_rate);

piojo_bloom_t*
piojo_bloom_alloc_cb_i32k(size_t capacity, float false_positive_rate,
    piojo_alloc_if allocator);

piojo_bloom_t*
piojo_bloom_alloc_cb_i64k(size_t capacity, float false_positive_rate,
    piojo_alloc_if allocator);

piojo_bloom_t*
piojo_bloom_alloc_cb_sizk(size_t capacity, float false_positive_rate,
    piojo_alloc_if allocator);

piojo_bloom_t*
piojo_bloom_alloc_eq(size_t capacity, float false_positive_rate,
    size_t eksize);

piojo_bloom_t*
piojo_bloom_alloc_cb_eq(size_t capacity, float false_positive_rate,
    size_t eksize, piojo_alloc_if allocator);

piojo_bloom_t*
piojo_bloom_copy(const piojo_bloom_t *bloom);

void
piojo_bloom_free(const piojo_bloom_t *bloom);

void
piojo_bloom_clear(piojo_bloom_t *bloom);

void
piojo_bloom_insert(const void *key, piojo_bloom_t *bloom);

bool
piojo_bloom_search(const void *key, const piojo_bloom_t *bloom);

#ifdef __cplusplus
}
#endif
#endif
