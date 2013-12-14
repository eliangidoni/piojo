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
#include <piojo/piojo_queue.h>

void test_alloc()
{
        piojo_queue_t *queue;

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_TRUE);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_s(PIOJO_QUEUE_DYN_TRUE, 2);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_n(PIOJO_QUEUE_DYN_TRUE, 2, 2);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE, 2, piojo_alloc_default);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_cb_n(PIOJO_QUEUE_DYN_TRUE, 2, 2, piojo_alloc_default);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_FALSE);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_s(PIOJO_QUEUE_DYN_FALSE, 2);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_n(PIOJO_QUEUE_DYN_FALSE, 2, 2);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_FALSE, 2, piojo_alloc_default);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);

        queue = piojo_queue_alloc_cb_n(PIOJO_QUEUE_DYN_FALSE, 2, 2, piojo_alloc_default);
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);
}

void test_copy_def()
{
        piojo_queue_t *queue, *copy;
        int i=10,j;

        queue = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE, sizeof(int),
                                     piojo_alloc_default);
        piojo_queue_push(&i, queue);

        copy = piojo_queue_copy(queue);
        PIOJO_ASSERT(copy);

        piojo_queue_free(queue);

        j = *(int*) piojo_queue_peek(copy);
        PIOJO_ASSERT(i == j);

        piojo_queue_free(copy);
}

void test_copy()
{
        piojo_queue_t *queue, *copy;
        int i=0;

        queue = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE, 2, my_allocator);
        piojo_queue_push(&i, queue);

        copy = piojo_queue_copy(queue);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_queue_free(queue);
        assert_allocator_init(1);

        piojo_queue_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_queue_t *queue;
        int i=0;

        queue = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE, 2, my_allocator);
        piojo_queue_push(&i, queue);

        piojo_queue_free(queue);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_queue_t *queue;
        int i=0;

        queue = piojo_queue_alloc_cb(PIOJO_QUEUE_DYN_TRUE, 2, my_allocator);
        piojo_queue_push(&i, queue);

        piojo_queue_clear(queue);
        assert_allocator_init(0);

        piojo_queue_free(queue);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_queue_t *queue;
        int i=0;

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_TRUE);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);

        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 1);

        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 2);

        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 3);

        piojo_queue_pop(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 2);

        piojo_queue_clear(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);
        piojo_queue_free(queue);
}

void test_push()
{
        piojo_queue_t *queue;
        int i=1234, j=0;

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_TRUE);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);

        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 1);

        j = *(int*) piojo_queue_peek(queue);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 2);

        j = *(int*) piojo_queue_peek(queue);
        PIOJO_ASSERT(i - 1 == j);

        piojo_queue_free(queue);
}

void test_pop()
{
        piojo_queue_t *queue;
        int i=1234, j=0;

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_TRUE);
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);

        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 1);

        ++i;
        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 2);

        j = *(int*) piojo_queue_peek(queue);
        PIOJO_ASSERT(i - 1 == j);

        piojo_queue_pop(queue);
        j = *(int*) piojo_queue_peek(queue);
        PIOJO_ASSERT(i == j);

        piojo_queue_free(queue);
}

void test_peek()
{
        piojo_queue_t *queue;
        int i=1234, j=0;

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_TRUE);
        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 1);

        j = *(int*) piojo_queue_peek(queue);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_queue_push(&i, queue);
        PIOJO_ASSERT(piojo_queue_size(queue) == 2);

        j = *(int*) piojo_queue_peek(queue);
        PIOJO_ASSERT(i - 1 == j);

        piojo_queue_free(queue);
}

void test_queue_expand()
{
        piojo_queue_t *queue;
        int i,j;

        queue = piojo_queue_alloc_n(PIOJO_QUEUE_DYN_TRUE, sizeof(i), 2);
        for (i = 0; i <= 4; ++i){
                piojo_queue_push(&i, queue);
        }

        PIOJO_ASSERT(piojo_queue_size(queue) == 5);

        for (i = 0; i < 5; ++i){
                j = *(int*) piojo_queue_peek(queue);
                PIOJO_ASSERT(i == j);
                piojo_queue_pop(queue);
        }
        piojo_queue_free(queue);
}

void test_full_p()
{
        piojo_queue_t *queue;
        int i;

        queue = piojo_queue_alloc_n(PIOJO_QUEUE_DYN_FALSE, sizeof(i), 2);
        for (i = 0; i < 2; ++i){
                piojo_queue_push(&i, queue);
        }

        PIOJO_ASSERT(piojo_queue_size(queue) == 2);
        PIOJO_ASSERT(piojo_queue_full_p(queue));

        piojo_queue_clear(queue);
        PIOJO_ASSERT(! piojo_queue_full_p(queue));
        PIOJO_ASSERT(piojo_queue_size(queue) == 0);

        piojo_queue_free(queue);
}

void test_stress()
{
        piojo_queue_t *queue;
        int i,j;

        queue = piojo_queue_alloc(PIOJO_QUEUE_DYN_TRUE);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                piojo_queue_push(&i, queue);
        }

        PIOJO_ASSERT(piojo_queue_size(queue) == TEST_STRESS_COUNT);

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_queue_peek(queue);
                PIOJO_ASSERT(i == j);
                piojo_queue_pop(queue);
        }
        piojo_queue_free(queue);
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
        test_queue_expand();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
