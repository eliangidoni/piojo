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
#include <piojo/piojo_array.h>

void test_alloc(void)
{
        piojo_array_t *array;

        array = piojo_array_alloc(2);
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(piojo_array_size(array) == 0);
        piojo_array_free(array);

        array = piojo_array_alloc_cb(2, piojo_alloc_default);
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(piojo_array_size(array) == 0);
        piojo_array_free(array);
}

void test_copy_def(void)
{
        piojo_array_t *array, *copy;
        int i=10,j;

        array = piojo_array_alloc_cb(sizeof(int), piojo_alloc_default);
        piojo_array_push(&i, array);

        copy = piojo_array_copy(array);
        PIOJO_ASSERT(copy);

        j = *(int*) piojo_array_at(0, copy);
        PIOJO_ASSERT(i == j);

        piojo_array_free(array);
        piojo_array_free(copy);
}

void test_copy(void)
{
        piojo_array_t *array, *copy;
        int i=10;

        array = piojo_array_alloc_cb(2, my_allocator);
        piojo_array_push(&i, array);

        copy = piojo_array_copy(array);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_array_free(array);
        assert_allocator_init(1);

        piojo_array_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free(void)
{
        piojo_array_t *array;
        int i=0;

        array = piojo_array_alloc_cb(2, my_allocator);
        piojo_array_push(&i, array);

        piojo_array_free(array);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear(void)
{
        piojo_array_t *array;
        int i=0;

        array = piojo_array_alloc_cb(2, my_allocator);
        piojo_array_push(&i, array);

        piojo_array_clear(array);
        assert_allocator_init(0);

        piojo_array_free(array);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size(void)
{
        piojo_array_t *array;
        int i=0;

        array = piojo_array_alloc(sizeof(i));
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        piojo_array_insert(0, &i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 3);

        piojo_array_delete(0, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        piojo_array_clear(array);
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        piojo_array_free(array);
}

void test_insert(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc(sizeof(i));
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        piojo_array_insert(0, &i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_array_insert(0, &i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        piojo_array_free(array);
}

void test_set(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc_cb(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        piojo_array_insert(0, &i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_array_insert(0, &i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_array_set(0, &i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        piojo_array_free(array);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_push(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc(sizeof(i));
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        j = *(int*) piojo_array_at(1, array);
        PIOJO_ASSERT(i == j);

        piojo_array_free(array);
}

static int
my_cmp(const void *e1, const void *e2)
{
        int v1 = *(int*) e1;
        int v2 = *(int*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

void test_push_cmp(void)
{
        piojo_array_t *array;
        unsigned int i=1234, j=0, k;

        array = piojo_array_alloc(sizeof(i));
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        for (j = 0; j < 10; ++j, --i){
                piojo_array_sorted_insert(&i, my_cmp, array);
                PIOJO_ASSERT(piojo_array_size(array) == j+1);
        }
        i = 1234 - (j-1);
        for (j = 0; j < 10; ++j, ++i){
                k = *(int*) piojo_array_at(j, array);
                PIOJO_ASSERT(i == k);
        }
        i = 1234;
        piojo_array_sorted_insert(&i, my_cmp, array);
        k = *(int*) piojo_array_at(piojo_array_size(array)-1, array);
        PIOJO_ASSERT(i == k);

        piojo_array_free(array);
}

void test_delete(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc(sizeof(i));
        piojo_array_push(&i, array);

        ++i;
        piojo_array_push(&i, array);

        piojo_array_delete(0, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_array_push(&i, array);

        piojo_array_delete(1, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i - 1 == j);
        piojo_array_free(array);
}

void test_pop(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc(sizeof(i));
        PIOJO_ASSERT(piojo_array_size(array) == 0);

        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        ++i;
        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        j = *(int*) piojo_array_last(array);
        PIOJO_ASSERT(i == j);

        piojo_array_pop(array);
        j = *(int*) piojo_array_last(array);
        PIOJO_ASSERT(i - 1 == j);

        piojo_array_free(array);
}

void test_last(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc(sizeof(i));
        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 1);

        j = *(int*) piojo_array_last(array);
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_array_push(&i, array);
        PIOJO_ASSERT(piojo_array_size(array) == 2);

        j = *(int*) piojo_array_last(array);
        PIOJO_ASSERT(i == j);
        piojo_array_free(array);
}

void test_at(void)
{
        piojo_array_t *array;
        int i=1234, j=0;

        array = piojo_array_alloc(sizeof(i));
        piojo_array_push(&i, array);

        ++i;
        piojo_array_push(&i, array);

        ++i;
        piojo_array_push(&i, array);

        j = *(int*) piojo_array_at(0, array);
        PIOJO_ASSERT(i - 2 == j);

        j = *(int*) piojo_array_at(1, array);
        PIOJO_ASSERT(i - 1 == j);

        j = *(int*) piojo_array_at(2, array);
        PIOJO_ASSERT(i == j);

        piojo_array_free(array);
}

void test_array_expand(void)
{
        piojo_array_t *array;
        int i,j;

        array = piojo_array_alloc(sizeof(i));
        for (i = 0; i <= 4; ++i){
                piojo_array_push(&i, array);
        }

        PIOJO_ASSERT(piojo_array_size(array) == 5);

        for (i = 0; i < 5; ++i){
                j = *(int*) piojo_array_at(i, array);
                PIOJO_ASSERT(i == j);
        }

        piojo_array_free(array);
}

void test_stress(void)
{
        piojo_array_t *array;
        int i,j;

        array = piojo_array_alloc(sizeof(i));
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                piojo_array_push(&i, array);
        }

        PIOJO_ASSERT(piojo_array_size(array) == TEST_STRESS_COUNT);

        for (i = 0; i < TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_array_at(i, array);
                PIOJO_ASSERT(i+1 == j);
        }
        piojo_array_free(array);
}

int main(void)
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_free();
        test_clear();
        test_size();
        test_insert();
        test_set();
        test_push();
        test_push_cmp();
        test_delete();
        test_pop();
        test_last();
        test_at();
        test_array_expand();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
