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
#include <piojo/piojo_stack.h>

void test_alloc()
{
        piojo_stack_t *stack;

        stack = piojo_stack_alloc();
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        stack = piojo_stack_alloc_s(2);
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        stack = piojo_stack_alloc_n(2, 2);
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        stack = piojo_stack_alloc_cb(2, piojo_alloc_default);
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        stack = piojo_stack_alloc_cb_n(2, 2, piojo_alloc_default);
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);
}

void test_copy_def()
{
        piojo_stack_t *stack, *copy;
        int i=10,j;

        stack = piojo_stack_alloc_cb(sizeof(int), piojo_alloc_default);
        piojo_stack_push(&i, stack);

        copy = piojo_stack_copy(stack);
        PIOJO_ASSERT(copy);

        piojo_stack_free(stack);

        j = *(int*) piojo_stack_peek(copy);
        PIOJO_ASSERT(i == j);

        piojo_stack_free(copy);
}

void test_copy()
{
        piojo_stack_t *stack, *copy;
        int i=0;

        stack = piojo_stack_alloc_cb(2, my_allocator);
        piojo_stack_push(&i, stack);

        copy = piojo_stack_copy(stack);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_stack_free(stack);
        assert_allocator_init(1);

        piojo_stack_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_stack_t *stack;
        int i=0;

        stack = piojo_stack_alloc_cb(2, my_allocator);
        piojo_stack_push(&i, stack);

        piojo_stack_free(stack);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_stack_t *stack;
        int i=0;

        stack = piojo_stack_alloc_cb(2, my_allocator);
        piojo_stack_push(&i, stack);

        piojo_stack_clear(stack);
        assert_allocator_init(0);

        piojo_stack_free(stack);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_stack_t *stack;
        int i=0;

        stack = piojo_stack_alloc();
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 1);

        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 2);

        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 3);

        piojo_stack_pop(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 2);

        piojo_stack_clear(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);
}

void test_push()
{
        piojo_stack_t *stack;
        int i=1234, j=0;

        stack = piojo_stack_alloc();
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 1);

        j = *(int*) piojo_stack_peek(stack);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 2);

        j = *(int*) piojo_stack_peek(stack);
        PIOJO_ASSERT(i == j);
}

void test_pop()
{
        piojo_stack_t *stack;
        int i=1234, j=0;

        stack = piojo_stack_alloc();
        PIOJO_ASSERT(piojo_stack_size(stack) == 0);

        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 1);

        ++i;
        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 2);

        j = *(int*) piojo_stack_peek(stack);
        PIOJO_ASSERT(i == j);

        piojo_stack_pop(stack);
        j = *(int*) piojo_stack_peek(stack);
        PIOJO_ASSERT(i - 1 == j);
}

void test_peek()
{
        piojo_stack_t *stack;
        int i=1234, j=0;

        stack = piojo_stack_alloc();
        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 1);

        j = *(int*) piojo_stack_peek(stack);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_stack_push(&i, stack);
        PIOJO_ASSERT(piojo_stack_size(stack) == 2);

        j = *(int*) piojo_stack_peek(stack);
        PIOJO_ASSERT(i == j);
}

void test_stack_expand()
{
        piojo_stack_t *stack;
        int i,j;

        stack = piojo_stack_alloc_n(sizeof(i), 2);
        for (i = 0; i <= 4; ++i){
                piojo_stack_push(&i, stack);
        }

        PIOJO_ASSERT(piojo_stack_size(stack) == 5);

        for (i = 4; i >= 0; --i){
                j = *(int*) piojo_stack_peek(stack);
                PIOJO_ASSERT(i == j);
                piojo_stack_pop(stack);
        }
}

void test_stress()
{
        piojo_stack_t *stack;
        int i,j;

        stack = piojo_stack_alloc();
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                piojo_stack_push(&i, stack);
        }

        PIOJO_ASSERT(piojo_stack_size(stack) == TEST_STRESS_COUNT);

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_stack_peek(stack);
                PIOJO_ASSERT(i == j);
                piojo_stack_pop(stack);
        }
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
        test_stack_expand();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
