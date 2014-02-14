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

#include <piojo_test.h>

int alloc_cnt = 0;
int init_cnt = 0;

void* my_alloc(size_t size)
{
        ++alloc_cnt;
        return piojo_alloc_def_alloc(size);
}

void my_free(const void *ptr)
{
        PIOJO_ASSERT(alloc_cnt > 0);
        --alloc_cnt;
        piojo_alloc_def_free(ptr);
}

void my_init(const void *data, size_t esize, void *newptr)
{
        ++init_cnt;
        piojo_alloc_def_init(data, esize, newptr);
}

void my_copy(const void *ptr, size_t esize, void *newptr)
{
        ++init_cnt;
        piojo_alloc_def_copy(ptr, esize, newptr);
}

void my_finish(void *ptr)
{
        PIOJO_ASSERT(init_cnt > 0);
        --init_cnt;
        piojo_alloc_def_finish(ptr);
}

piojo_alloc_if my_allocator = {
        my_alloc,
        my_free,
        my_init,
        my_copy,
        my_finish,
};

piojo_alloc_kv_if my_kvallocator = {
        my_alloc,
        my_free,
        my_init,
        my_copy,
        my_finish,
        my_init,
        my_copy,
        my_finish,
};
