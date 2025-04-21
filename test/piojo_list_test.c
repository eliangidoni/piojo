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
#include <piojo/piojo_list.h>

void test_alloc(void)
{
        piojo_list_t *list;

        list = piojo_list_alloc();
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(piojo_list_size(list) == 0);
        piojo_list_free(list);

        list = piojo_list_alloc_s(2);
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(piojo_list_size(list) == 0);
        piojo_list_free(list);

        list = piojo_list_alloc_cb(2, piojo_alloc_default);
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(piojo_list_size(list) == 0);
        piojo_list_free(list);
}

void test_copy_def(void)
{
        piojo_list_t *list, *copy;
        int i=10,j;

        list = piojo_list_alloc_cb(sizeof(int), piojo_alloc_default);
        piojo_list_append(&i, list);

        copy = piojo_list_copy(list);
        PIOJO_ASSERT(copy);

        piojo_list_free(list);

        j = *(int*) piojo_list_entry(piojo_list_first(copy));
        PIOJO_ASSERT(i == j);

        piojo_list_free(copy);
}

void test_copy(void)
{
        piojo_list_t *list, *copy;
        int i=0;

        list = piojo_list_alloc_cb(2, my_allocator);
        piojo_list_append(&i, list);

        copy = piojo_list_copy(list);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_list_free(list);
        assert_allocator_init(1);

        piojo_list_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free(void)
{
        piojo_list_t *list;
        int i=0;

        list = piojo_list_alloc_cb(2, my_allocator);
        piojo_list_append(&i, list);

        piojo_list_free(list);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear(void)
{
        piojo_list_t *list;
        int i=0;

        list = piojo_list_alloc_cb(2, my_allocator);
        piojo_list_append(&i, list);

        piojo_list_clear(list);
        assert_allocator_init(0);

        piojo_list_free(list);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size(void)
{
        piojo_list_t *list;
        int i=0;

        list = piojo_list_alloc();
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_append(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        piojo_list_insert(&i, piojo_list_last(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 2);

        piojo_list_append(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 3);

        piojo_list_delete(piojo_list_first(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 2);

        piojo_list_clear(list);
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_free(list);
}

void test_insert(void)
{
        piojo_list_t *list;
        int i=1234, j=0;

        list = piojo_list_alloc();
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_append(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        ++i;
        piojo_list_insert(&i, piojo_list_first(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 2);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_list_insert(&i, piojo_list_next(piojo_list_first(list)), list);
        PIOJO_ASSERT(piojo_list_size(list) == 3);

        j = *(int*) piojo_list_entry(piojo_list_prev(piojo_list_last(list)));
        PIOJO_ASSERT(i == j);

        piojo_list_free(list);
}

void test_set(void)
{
        piojo_list_t *list;
        int i=1234, j=0;

        list = piojo_list_alloc_cb(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_append(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_list_set(&i, piojo_list_first(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_list_insert(&i, piojo_list_first(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 2);

        j = *(int*) piojo_list_entry(piojo_list_prev(piojo_list_last(list)));
        PIOJO_ASSERT(i == j);

        piojo_list_free(list);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_prepend(void)
{
        piojo_list_t *list;
        int i=1234, j=0;

        list = piojo_list_alloc();
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_prepend(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_list_prepend(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 2);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(i == j);

        piojo_list_free(list);
}

void test_append(void)
{
        piojo_list_t *list;
        int i=1234, j=0;

        list = piojo_list_alloc();
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_append(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_list_append(&i, list);
        PIOJO_ASSERT(piojo_list_size(list) == 2);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(i == j);

        piojo_list_free(list);
}

void test_delete(void)
{
        piojo_list_t *list;
        piojo_list_node_t *node;
        int i=1234, j=0;

        list = piojo_list_alloc();
        piojo_list_append(&i, list);

        ++i;
        piojo_list_append(&i, list);

        piojo_list_delete(piojo_list_first(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(i == j);

        ++i;
        piojo_list_append(&i, list);

        piojo_list_delete(piojo_list_last(list), list);
        PIOJO_ASSERT(piojo_list_size(list) == 1);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(i - 1 == j);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(i - 1 == j);

        node = piojo_list_first(list);
        while(node){
                node = piojo_list_delete(node, list);
        }
        PIOJO_ASSERT(piojo_list_size(list) == 0);

        piojo_list_free(list);
}

void test_first_last(void)
{
        piojo_list_t *list;
        int i=1234, j=0;

        list = piojo_list_alloc();
        piojo_list_append(&i, list);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(j == i);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(j == i);

        ++i;
        piojo_list_append(&i, list);

        j = *(int*) piojo_list_entry(piojo_list_first(list));
        PIOJO_ASSERT(j == i - 1);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(j == i);

        piojo_list_free(list);
}

void test_next_prev(void)
{
        piojo_list_t *list;
        int i=1234, j=0;

        list = piojo_list_alloc();
        piojo_list_append(&i, list);

        ++i;
        piojo_list_append(&i, list);

        j = *(int*) piojo_list_entry(piojo_list_last(list));
        PIOJO_ASSERT(j == i);

        j = *(int*) piojo_list_entry(piojo_list_prev(piojo_list_last(list)));
        PIOJO_ASSERT(j == i - 1);

        j = *(int*) piojo_list_entry(piojo_list_next(piojo_list_first(list)));
        PIOJO_ASSERT(j == i);

        PIOJO_ASSERT(NULL == piojo_list_next(piojo_list_next(piojo_list_first(list))));
        PIOJO_ASSERT(NULL == piojo_list_prev(piojo_list_prev(piojo_list_last(list))));

        piojo_list_free(list);
}

void test_stress(void)
{
        piojo_list_t *list;
        piojo_list_node_t *node;
        int i, j;

        list = piojo_list_alloc();
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                piojo_list_append(&i, list);
        }

        PIOJO_ASSERT(piojo_list_size(list) == TEST_STRESS_COUNT);

        node = piojo_list_first(list);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_list_entry(node);
                PIOJO_ASSERT(j == i);
                node = piojo_list_next(node);
        }
        piojo_list_free(list);

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
        test_prepend();
        test_append();
        test_delete();
        test_first_last();
        test_next_prev();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
