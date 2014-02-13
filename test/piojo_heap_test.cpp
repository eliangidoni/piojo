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

#include <map>
#include <time.h>
#include <piojo_test.h>
#include <piojo/piojo_heap.h>

void test_alloc()
{
        piojo_heap_t *heap;

        heap = piojo_heap_alloc();
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        heap = piojo_heap_alloc_n(2);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        heap = piojo_heap_alloc_cb(piojo_alloc_default);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        heap = piojo_heap_alloc_cb_n(2, piojo_alloc_default);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);
}

void test_copy_def()
{
        piojo_heap_t *heap, *copy;
        int i=10,j;

        heap = piojo_heap_alloc_cb(piojo_alloc_default);
        piojo_heap_push((piojo_opaque_t)i, i, heap);

        copy = piojo_heap_copy(heap);
        PIOJO_ASSERT(copy);

        piojo_heap_free(heap);

        j = (int) piojo_heap_peek(copy);
        PIOJO_ASSERT(i == j);

        piojo_heap_free(copy);
}

void test_copy()
{
        piojo_heap_t *heap, *copy;
        int i=0;

        heap = piojo_heap_alloc_cb(my_allocator);
        piojo_heap_push((piojo_opaque_t)i, i, heap);

        copy = piojo_heap_copy(heap);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_heap_free(heap);
        assert_allocator_init(1);

        piojo_heap_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_heap_t *heap;
        int i=0;

        heap = piojo_heap_alloc_cb(my_allocator);
        piojo_heap_push((piojo_opaque_t)i, i, heap);

        piojo_heap_free(heap);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_heap_t *heap;
        int i=0;

        heap = piojo_heap_alloc_cb(my_allocator);
        piojo_heap_push((piojo_opaque_t)i, i, heap);

        piojo_heap_clear(heap);
        assert_allocator_init(0);

        piojo_heap_free(heap);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_heap_t *heap;
        int i=0;

        heap = piojo_heap_alloc();
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 3);

        piojo_heap_pop(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        piojo_heap_clear(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_free(heap);
}

void test_push()
{
        piojo_heap_t *heap;
        int i=1234, j=0;

        heap = piojo_heap_alloc();
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i-1 == j);

        piojo_heap_free(heap);
}

void test_pop()
{
        piojo_heap_t *heap;
        int i=1234, j=0;

        heap = piojo_heap_alloc();
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i - 1 == j);

        piojo_heap_pop(heap);
        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i == j);

        piojo_heap_free(heap);
}

void test_peek()
{
        piojo_heap_t *heap;
        int i=1234, j=0;

        heap = piojo_heap_alloc();
        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i - 1 == j);
        piojo_heap_free(heap);
}

void test_heap_expand()
{
        piojo_heap_t *heap;
        int i,j;

        heap = piojo_heap_alloc_n(2);
        for (i = 0; i <= 4; ++i){
                piojo_heap_push((piojo_opaque_t)i, i, heap);
        }

        PIOJO_ASSERT(piojo_heap_size(heap) == 5);

        for (i = 0; i <= 4; ++i){
                j = (int) piojo_heap_peek(heap);
                PIOJO_ASSERT(i == j);
                piojo_heap_pop(heap);
        }

        piojo_heap_free(heap);
}

void test_stress()
{
        piojo_heap_t *heap;
        int i,j,prev;
        std::map<int,bool>unique;

        srand(time(NULL));
        heap = piojo_heap_alloc();
        for (i = 0; i < TEST_STRESS_COUNT; ++i){
                j = rand();
                while (unique.count(j) > 0){
                        j = rand();
                }
                unique[j]=true;
                piojo_heap_push((piojo_opaque_t)j, j, heap);
        }

        PIOJO_ASSERT(piojo_heap_size(heap) == TEST_STRESS_COUNT);

        for (i = 0; i < TEST_STRESS_COUNT; ++i){
                j = (int) piojo_heap_peek(heap);
                if (i == 0){
                        prev = j;
                }
                PIOJO_ASSERT(j >= prev);
                prev = j;
                piojo_heap_pop(heap);
        }

        piojo_heap_free(heap);
}

int main()
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_free();
        test_clear();
        test_size();
        test_push();
        test_pop();
        test_peek();
        test_heap_expand();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
