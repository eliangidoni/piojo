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
#include <piojo/piojo_ring.h>

void test_alloc()
{
        piojo_ring_t *ring;

        ring = piojo_ring_alloc(2);
        PIOJO_ASSERT(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);
        piojo_ring_free(ring);

        ring = piojo_ring_alloc_s( 2, 2);
        PIOJO_ASSERT(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);
        piojo_ring_free(ring);

        ring = piojo_ring_alloc_cb( 2, 2, piojo_alloc_default);
        PIOJO_ASSERT(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);
        piojo_ring_free(ring);
}

void test_copy_def()
{
        piojo_ring_t *ring, *copy;
        int i=10,j;

        ring = piojo_ring_alloc_cb( sizeof(int), 10,
                                     piojo_alloc_default);
        piojo_ring_push(&i, ring);

        copy = piojo_ring_copy(ring);
        PIOJO_ASSERT(copy);

        piojo_ring_free(ring);

        j = *(int*) piojo_ring_peek(copy);
        PIOJO_ASSERT(i == j);

        piojo_ring_free(copy);
}

void test_copy()
{
        piojo_ring_t *ring, *copy;
        int i=0;

        ring = piojo_ring_alloc_cb( 2, 10, my_allocator);
        piojo_ring_push(&i, ring);

        copy = piojo_ring_copy(ring);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_ring_free(ring);
        assert_allocator_init(1);

        piojo_ring_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_ring_t *ring;
        int i=0;

        ring = piojo_ring_alloc_cb( 2, 10, my_allocator);
        piojo_ring_push(&i, ring);

        piojo_ring_free(ring);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_ring_t *ring;
        int i=0;

        ring = piojo_ring_alloc_cb( 2, 10, my_allocator);
        piojo_ring_push(&i, ring);

        piojo_ring_clear(ring);
        assert_allocator_init(0);

        piojo_ring_free(ring);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_ring_t *ring;
        int i=0;

        ring = piojo_ring_alloc(10);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);

        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 1);

        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 2);

        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 3);

        piojo_ring_pop(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 2);

        piojo_ring_clear(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);
        piojo_ring_free(ring);
}

void test_push()
{
        piojo_ring_t *ring;
        int i=1234, j=0;

        ring = piojo_ring_alloc(10);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);

        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 1);

        j = *(int*) piojo_ring_peek(ring);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 2);

        j = *(int*) piojo_ring_peek(ring);
        PIOJO_ASSERT(i - 1 == j);

        piojo_ring_free(ring);
}

void test_pop()
{
        piojo_ring_t *ring;
        int i=1234, j=0;

        ring = piojo_ring_alloc(10);
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);

        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 1);

        ++i;
        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 2);

        j = *(int*) piojo_ring_peek(ring);
        PIOJO_ASSERT(i - 1 == j);

        piojo_ring_pop(ring);
        j = *(int*) piojo_ring_peek(ring);
        PIOJO_ASSERT(i == j);

        piojo_ring_free(ring);
}

void test_peek()
{
        piojo_ring_t *ring;
        int i=1234, j=0;

        ring = piojo_ring_alloc(10);
        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 1);

        j = *(int*) piojo_ring_peek(ring);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_ring_push(&i, ring);
        PIOJO_ASSERT(piojo_ring_size(ring) == 2);

        j = *(int*) piojo_ring_peek(ring);
        PIOJO_ASSERT(i - 1 == j);

        piojo_ring_free(ring);
}

void test_full_p()
{
        piojo_ring_t *ring;
        int i;

        ring = piojo_ring_alloc_s( sizeof(i), 2);
        for (i = 0; i < 2; ++i){
                piojo_ring_push(&i, ring);
        }

        PIOJO_ASSERT(piojo_ring_size(ring) == 2);
        PIOJO_ASSERT(piojo_ring_full_p(ring));

        piojo_ring_clear(ring);
        PIOJO_ASSERT(! piojo_ring_full_p(ring));
        PIOJO_ASSERT(piojo_ring_size(ring) == 0);

        piojo_ring_free(ring);
}

void test_stress()
{
        piojo_ring_t *ring;
        int i,j;

        ring = piojo_ring_alloc_s(sizeof(i), TEST_STRESS_COUNT);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                piojo_ring_push(&i, ring);
        }

        PIOJO_ASSERT(piojo_ring_size(ring) == TEST_STRESS_COUNT);

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_ring_peek(ring);
                PIOJO_ASSERT(i == j);
                piojo_ring_pop(ring);
        }
        piojo_ring_free(ring);
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
        test_full_p();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
