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
 * Piojo Array API.
 */

#ifndef PIOJO_ARRAY_H_
#define PIOJO_ARRAY_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_array;
typedef struct piojo_array piojo_array_t;
extern const size_t piojo_array_sizeof;

piojo_array_t*
piojo_array_alloc(size_t esize);

piojo_array_t*
piojo_array_alloc_cb(size_t esize, piojo_alloc_if allocator);

piojo_array_t*
piojo_array_copy(const piojo_array_t *array);

void
piojo_array_free(const piojo_array_t *array);

void
piojo_array_clear(piojo_array_t *array);

void
piojo_array_reserve(size_t ecount, piojo_array_t *array);

size_t
piojo_array_size(const piojo_array_t *array);

void
piojo_array_insert(size_t idx, const void *data, piojo_array_t *array);

void
piojo_array_set(size_t idx, const void *data, piojo_array_t *array);

void
piojo_array_push(const void *data, piojo_array_t *array);

size_t*
piojo_array_index(const void *data, piojo_cmp_cb cmp,
                  const piojo_array_t *array, size_t *idx);

void
piojo_array_sorted_insert(const void *data, piojo_cmp_cb cmp,
                          piojo_array_t *array);

size_t*
piojo_array_sorted_index(const void *data, piojo_cmp_cb cmp,
                         const piojo_array_t *array, size_t *idx);

void
piojo_array_delete(size_t idx, piojo_array_t *array);

void
piojo_array_pop(piojo_array_t *array);

void*
piojo_array_at(size_t idx, const piojo_array_t *array);

void*
piojo_array_first(const piojo_array_t *array);

void*
piojo_array_last(const piojo_array_t *array);

#ifdef __cplusplus
}
#endif
#endif
