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

bool int_leq(piojo_opaque_t e1, piojo_opaque_t e2)
{
        return ((int)e1 <= (int)e2);
}

struct entry {
        int i,key;
};

bool entry_leq(piojo_opaque_t e1, piojo_opaque_t e2)
{
        return (((struct entry*)e1)->key <= ((struct entry*)e2)->key);
}

void test_alloc()
{
        piojo_heap_t *heap;

        heap = piojo_heap_alloc(int_leq);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        heap = piojo_heap_alloc_n(int_leq,2);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        heap = piojo_heap_alloc_cb(int_leq,piojo_alloc_default);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        heap = piojo_heap_alloc_cb_n(int_leq,2, piojo_alloc_default);
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);
        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_copy_def()
{
        piojo_heap_t *heap, *copy;
        int i=10,j;

        heap = piojo_heap_alloc_cb(int_leq,piojo_alloc_default);
        piojo_heap_push((piojo_opaque_t)i, heap);

        copy = piojo_heap_copy(heap);
        PIOJO_ASSERT(copy);

        piojo_heap_free(heap);

        j = (int) piojo_heap_peek(copy);
        PIOJO_ASSERT(i == j);

        piojo_heap_free(copy);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_copy()
{
        piojo_heap_t *heap, *copy;
        int i=0;

        heap = piojo_heap_alloc_cb(int_leq,my_allocator);
        piojo_heap_push((piojo_opaque_t)i, heap);

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

        heap = piojo_heap_alloc_cb(int_leq,my_allocator);
        piojo_heap_push((piojo_opaque_t)i, heap);

        piojo_heap_free(heap);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_heap_t *heap;
        int i=0;

        heap = piojo_heap_alloc_cb(int_leq,my_allocator);
        piojo_heap_push((piojo_opaque_t)i, heap);

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

        heap = piojo_heap_alloc(int_leq);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 3);

        piojo_heap_pop(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        piojo_heap_clear(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_push()
{
        piojo_heap_t *heap;
        int i=1234, j=0;

        heap = piojo_heap_alloc(int_leq);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i-1 == j);

        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_pop()
{
        piojo_heap_t *heap;
        int i=1234, j=0;

        heap = piojo_heap_alloc(int_leq);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i - 1 == j);

        piojo_heap_pop(heap);
        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i == j);

        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_peek()
{
        piojo_heap_t *heap;
        int i=1234, j=0;

        heap = piojo_heap_alloc(int_leq);
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);

        j = (int) piojo_heap_peek(heap);
        PIOJO_ASSERT(i - 1 == j);
        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_heap_expand()
{
        piojo_heap_t *heap;
        int i,j;

        heap = piojo_heap_alloc_n(int_leq,2);
        for (i = 0; i <= 4; ++i){
                piojo_heap_push((piojo_opaque_t)i, heap);
        }

        PIOJO_ASSERT(piojo_heap_size(heap) == 5);

        for (i = 0; i <= 4; ++i){
                j = (int) piojo_heap_peek(heap);
                PIOJO_ASSERT(i == j);
                piojo_heap_pop(heap);
        }

        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_stress()
{
        piojo_heap_t *heap;
        int i,j,prev;
        std::map<int,bool>unique;

        srand(time(NULL));
        heap = piojo_heap_alloc(int_leq);
        for (i = 0; i < TEST_STRESS_COUNT; ++i){
                j = rand();
                while (unique.count(j) > 0){
                        j = rand();
                }
                unique[j]=true;
                piojo_heap_push((piojo_opaque_t)j, heap);
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

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_heap_decr()
{
        piojo_heap_t *heap;
        unsigned int j=0;
        struct entry entries[10] = {{1,10},{2,30},{3,8},{4,20},{5,80}};
        struct entry*e;

        heap = piojo_heap_alloc(entry_leq);
        PIOJO_ASSERT(piojo_heap_size(heap) == 0);

        for (j=0; j<5; ++j){
                piojo_heap_push((piojo_opaque_t)&entries[j], heap);
                PIOJO_ASSERT(piojo_heap_size(heap) == j+1);
        }

        e = (struct entry*) piojo_heap_peek(heap);
        PIOJO_ASSERT(3 == e->i);

        piojo_heap_pop(heap);
        entries[3].key = 2;
        piojo_heap_decrease((piojo_opaque_t)&entries[3], heap);
        e = (struct entry*) piojo_heap_peek(heap);
        PIOJO_ASSERT(4 == e->i);

        entries[4].key = 1;
        piojo_heap_decrease((piojo_opaque_t)&entries[4], heap);
        e = (struct entry*) piojo_heap_peek(heap);
        PIOJO_ASSERT(5 == e->i);

        piojo_heap_free(heap);

        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_heap_contain_p()
{
        piojo_heap_t *heap;
        int i=1234;

        heap = piojo_heap_alloc(int_leq);
        PIOJO_ASSERT(! piojo_heap_contain_p((piojo_opaque_t)i, heap));
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 1);
        PIOJO_ASSERT(piojo_heap_contain_p((piojo_opaque_t)i, heap));

        PIOJO_ASSERT(! piojo_heap_contain_p((piojo_opaque_t)i+1, heap));
        ++i;
        piojo_heap_push((piojo_opaque_t)i, heap);
        PIOJO_ASSERT(piojo_heap_size(heap) == 2);
        PIOJO_ASSERT(piojo_heap_contain_p((piojo_opaque_t)i, heap));

        piojo_heap_free(heap);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
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
        test_heap_decr();
        test_heap_contain_p();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
