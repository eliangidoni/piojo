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
 * Piojo Ring API.
 */

/**
 * @file
 * @addtogroup piojoring
 */

#ifndef PIOJO_RING_H_
#define PIOJO_RING_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_ring;
typedef struct piojo_ring piojo_ring_t;
extern const size_t piojo_ring_sizeof;

piojo_ring_t*
piojo_ring_alloc(size_t ecount);

piojo_ring_t*
piojo_ring_alloc_s(size_t esize, size_t ecount);

piojo_ring_t*
piojo_ring_alloc_cb(size_t esize, size_t ecount, piojo_alloc_if allocator);

piojo_ring_t*
piojo_ring_copy(const piojo_ring_t *ring);

void
piojo_ring_free(const piojo_ring_t *ring);

void
piojo_ring_clear(piojo_ring_t *ring);

size_t
piojo_ring_size(const piojo_ring_t *ring);

bool
piojo_ring_full_p(const piojo_ring_t *ring);

void
piojo_ring_push(const void *data, piojo_ring_t *ring);

void
piojo_ring_pop(piojo_ring_t *ring);

void*
piojo_ring_peek(const piojo_ring_t *ring);

#ifdef __cplusplus
}
#endif
#endif
