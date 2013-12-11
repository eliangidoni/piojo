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
 * Piojo Stack API.
 */

#ifndef PIOJO_STACK_H_
#define PIOJO_STACK_H_

#include <piojo/piojo_common.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_stack;
typedef struct piojo_stack piojo_stack_t;
extern const size_t piojo_stack_sizeof;

piojo_stack_t*
piojo_stack_alloc(void);

piojo_stack_t*
piojo_stack_alloc_s(size_t esize);

piojo_stack_t*
piojo_stack_alloc_n(size_t esize, size_t ecount);

piojo_stack_t*
piojo_stack_alloc_cb(size_t esize, piojo_alloc_t allocator);

piojo_stack_t*
piojo_stack_alloc_cb_n(size_t esize, size_t ecount, piojo_alloc_t allocator);

piojo_stack_t*
piojo_stack_copy(const piojo_stack_t *stack);

void
piojo_stack_free(const piojo_stack_t *stack);

void
piojo_stack_clear(piojo_stack_t *stack);

size_t
piojo_stack_size(const piojo_stack_t *stack);

void
piojo_stack_push(const void *data, piojo_stack_t *stack);

void
piojo_stack_pop(piojo_stack_t *stack);

void*
piojo_stack_peek(const piojo_stack_t *stack);

#ifdef __cplusplus
}
#endif
#endif
