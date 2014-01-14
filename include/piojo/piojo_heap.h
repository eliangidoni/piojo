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
 * Piojo Heap API.
 */

#ifndef PIOJO_HEAP_H_
#define PIOJO_HEAP_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_heap;
typedef struct piojo_heap piojo_heap_t;
extern const size_t piojo_heap_sizeof;

piojo_heap_t*
piojo_heap_alloc();

piojo_heap_t*
piojo_heap_alloc_s(piojo_leq_cb cmp, size_t esize);

piojo_heap_t*
piojo_heap_alloc_n(piojo_leq_cb cmp, size_t esize, size_t ecount);

piojo_heap_t*
piojo_heap_alloc_cb(piojo_leq_cb cmp, size_t esize,
                    piojo_alloc_if allocator);

piojo_heap_t*
piojo_heap_alloc_cb_n(piojo_leq_cb cmp, size_t esize, size_t ecount,
                      piojo_alloc_if allocator);

piojo_heap_t*
piojo_heap_copy(const piojo_heap_t *heap);

void
piojo_heap_free(const piojo_heap_t *heap);

void
piojo_heap_clear(piojo_heap_t *heap);

size_t
piojo_heap_size(const piojo_heap_t *heap);

void
piojo_heap_push(const void *data, piojo_heap_t *heap);

void
piojo_heap_pop(piojo_heap_t *heap);

void*
piojo_heap_peek(const piojo_heap_t *heap);

#ifdef __cplusplus
}
#endif
#endif
