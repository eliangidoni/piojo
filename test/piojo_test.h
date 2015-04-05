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
 */

#ifndef PIOJO_TEST_H_
#define PIOJO_TEST_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <piojo/piojo_alloc.h>

/* Note that this enforces a read when 'x' is volatile. */
#define PIOJO_UNUSED(x) (void)(x)
#define PIOJO_ASSERT(cond) do{ assert(cond); } while(0)
#define PIOJO_FAIL_IF(cond) do{ assert(! (cond)); } while(0)
#define TEST_STRESS_COUNT 10000000

#define assert_allocator_init(val) PIOJO_ASSERT(init_cnt == val)
#define assert_allocator_alloc(val) PIOJO_ASSERT(alloc_cnt == val)

extern piojo_alloc_if my_allocator;
extern int alloc_cnt;
extern int init_cnt;

void* my_alloc(size_t size);
void* my_realloc(const void *ptr, size_t size);
void my_free(const void *ptr);

#endif
