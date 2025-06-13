/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 G. Elian Gidoni
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
#include <piojo/piojo_skiplist.h>
#include <time.h>

int int_cmp(const void *e1, const void *e2)
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

void test_alloc(void)
{
        piojo_skiplist_t *list;

        list = piojo_skiplist_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);
        piojo_skiplist_free(list);

        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), piojo_alloc_default);
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);
        piojo_skiplist_free(list);

        list = piojo_skiplist_alloc_cmp(sizeof(int), int_cmp, sizeof(int));
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);
        piojo_skiplist_free(list);
}

void test_copy_def(void)
{
        piojo_skiplist_t *list, *copy;
        int i=10,j;

        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), piojo_alloc_default);
        piojo_skiplist_insert(&i, &i, list);

        copy = piojo_skiplist_copy(list);
        PIOJO_ASSERT(copy);

        piojo_skiplist_free(list);

        j = *(int*) piojo_skiplist_first(copy, NULL);
        PIOJO_ASSERT(i == j);

        piojo_skiplist_free(copy);
}

void test_copy(void)
{
        piojo_skiplist_t *list, *copy;
        int i=0;

        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), my_allocator);
        piojo_skiplist_insert(&i, &i, list);

        copy = piojo_skiplist_copy(list);
        PIOJO_ASSERT(copy);
        assert_allocator_init(2);

        piojo_skiplist_free(list);
        assert_allocator_init(1);

        piojo_skiplist_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free(void)
{
        piojo_skiplist_t *list;
        int i=0;

        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), my_allocator);
        piojo_skiplist_insert(&i, &i, list);

        piojo_skiplist_free(list);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear(void)
{
        piojo_skiplist_t *list;
        int i=0;

        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), my_allocator);
        piojo_skiplist_insert(&i, &i, list);

        piojo_skiplist_clear(list);
        assert_allocator_init(0);

        piojo_skiplist_free(list);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size(void)
{
        piojo_skiplist_t *list;
        int i=0;

        list = piojo_skiplist_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);

        piojo_skiplist_insert(&i, &i, list);

        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        piojo_skiplist_delete(piojo_skiplist_first(list, NULL), list);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);

        piojo_skiplist_free(list);
}

void test_insert(void)
{
        piojo_skiplist_t *list;
        int i=1234, j=0;

        list = piojo_skiplist_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);

        bool inserted = piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(inserted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        ++i;
        inserted = piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(inserted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 2);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(i == j);

        ++i;
        inserted = piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(inserted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 3);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(i == j);

        bool deleted = piojo_skiplist_delete(piojo_skiplist_first(list, NULL), list);
        PIOJO_ASSERT(deleted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 2);

        piojo_skiplist_free(list);
}

void test_insert_bool(void)
{
        piojo_skiplist_t *list;
        int i=1234;

        list = piojo_skiplist_alloc_i32k(sizeof(bool));
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);

        bool inserted = piojo_skiplist_insert(&i, NULL, list);
        PIOJO_ASSERT(inserted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        piojo_skiplist_free(list);
}


void test_set(void)
{
        piojo_skiplist_t *list;
        int i=1234, j=0;

        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 0);

        bool inserted = piojo_skiplist_set(&i, &i, list);
        PIOJO_ASSERT(inserted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        j = *(int*) piojo_skiplist_first(list, NULL);
        PIOJO_ASSERT(i == j);

        ++i;
        bool replaced = piojo_skiplist_set(piojo_skiplist_first(list, NULL), &i, list);
        PIOJO_ASSERT(!replaced);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        int *tmp;
        j = *(int*) piojo_skiplist_first(list, (void**)&tmp);
        PIOJO_ASSERT(i - 1 == j);
        PIOJO_ASSERT(*tmp == i);

        piojo_skiplist_free(list);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_delete(void)
{
        piojo_skiplist_t *list;
        int i=1234, j=0;
        bool inserted, deleted;

        list = piojo_skiplist_alloc_i32k(sizeof(int));
        inserted = piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(inserted);

        ++i;
        inserted = piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(inserted);

        deleted = piojo_skiplist_delete(piojo_skiplist_first(list, NULL), list);
        PIOJO_ASSERT(deleted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(i == j);

        ++i;
        inserted = piojo_skiplist_insert(&i, &i, list);
        PIOJO_ASSERT(inserted);

        deleted = piojo_skiplist_delete(piojo_skiplist_last(list, NULL), list);
        PIOJO_ASSERT(deleted);
        PIOJO_ASSERT(piojo_skiplist_size(list) == 1);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(i - 1 == j);

        j = *(int*) piojo_skiplist_first(list, NULL);
        PIOJO_ASSERT(i - 1 == j);

        j = 9999;
        deleted = piojo_skiplist_delete(&j, list);
        PIOJO_ASSERT(!deleted);

        piojo_skiplist_free(list);
}

void test_first_last(void)
{
        piojo_skiplist_t *list;
        int i=1234, j=0;

        list = piojo_skiplist_alloc_i32k(sizeof(int));

        const void * last = piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(last == NULL);

        const void * first = piojo_skiplist_first(list, NULL);
        PIOJO_ASSERT(first == NULL);

        piojo_skiplist_insert(&i, &i, list);

        j = *(int*) piojo_skiplist_first(list, NULL);
        PIOJO_ASSERT(j == i);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(j == i);

        ++i;
        piojo_skiplist_insert(&i, &i, list);

        j = *(int*) piojo_skiplist_first(list, NULL);
        PIOJO_ASSERT(j == i - 1);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(j == i);

        piojo_skiplist_free(list);
}

void test_next_prev(void)
{
        piojo_skiplist_t *list;
        int i=1234, j=0;

        list = piojo_skiplist_alloc_i32k(sizeof(int));
        piojo_skiplist_insert(&i, &i, list);

        ++i;
        piojo_skiplist_insert(&i, &i, list);

        j = *(int*) piojo_skiplist_last(list, NULL);
        PIOJO_ASSERT(j == i);

        j = *(int*) piojo_skiplist_prev(piojo_skiplist_last(list, NULL), list, NULL);
        PIOJO_ASSERT(j == i - 1);

        j = *(int*) piojo_skiplist_next(piojo_skiplist_first(list, NULL), list, NULL);
        PIOJO_ASSERT(j == i);

        PIOJO_ASSERT(NULL == piojo_skiplist_next(piojo_skiplist_next(piojo_skiplist_first(list, NULL), list, NULL), list, NULL));
        PIOJO_ASSERT(NULL == piojo_skiplist_prev(piojo_skiplist_prev(piojo_skiplist_last(list, NULL), list, NULL), list, NULL));

        piojo_skiplist_free(list);
}

void test_search(void)
{
        piojo_skiplist_t *list;
        const size_t *nodekey, *nodedata;
        int i;
        size_t sorted[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        size_t unsorted[10] = {10, 3, 2, 6, 7, 5, 4, 9, 8, 1};
        list = piojo_skiplist_alloc_sizk(sizeof(size_t));

        void * notfound = piojo_skiplist_search(&unsorted[0], list);
        PIOJO_ASSERT(notfound == NULL);

        for (i = 0; i < 10; ++i){
                bool inserted = piojo_skiplist_insert(&unsorted[i], &unsorted[i], list);
                PIOJO_ASSERT(inserted);
                size_t elem = *(size_t*)piojo_skiplist_search(&unsorted[i], list);
                PIOJO_ASSERT(elem == unsorted[i]);
        }

        PIOJO_ASSERT(piojo_skiplist_size(list) == 10);

        nodekey = piojo_skiplist_first(list, (void**)&nodedata);
        for (i = 0; i < 10; ++i){
                PIOJO_ASSERT(*nodekey == sorted[i]);
                PIOJO_ASSERT(*nodedata == sorted[i]);
                nodekey = piojo_skiplist_next(nodekey, list, (void**)&nodedata);
        }
        nodekey = piojo_skiplist_last(list, (void**)&nodedata);
        for (i = 9; i >= 0; --i){
                PIOJO_ASSERT(*nodekey == sorted[i]);
                PIOJO_ASSERT(*nodedata == sorted[i]);
                nodekey = piojo_skiplist_prev(nodekey, list, (void**)&nodedata);
        }
        piojo_skiplist_free(list);
}

void test_stress(void)
{
        piojo_skiplist_t *list;
        const int *nodekey, *nodedata;
        int64_t i;

        list = piojo_skiplist_alloc_i64k(sizeof(int));
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                bool inserted = piojo_skiplist_insert(&i, &i, list);
                PIOJO_ASSERT(inserted);
        }

        PIOJO_ASSERT(piojo_skiplist_size(list) == TEST_STRESS_COUNT);
        nodekey = piojo_skiplist_first(list, (void**)&nodedata);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                PIOJO_ASSERT(*nodekey == i);
                PIOJO_ASSERT(*nodedata == i);
                nodekey = piojo_skiplist_next(nodekey, list, (void**)&nodedata);
        }
        piojo_skiplist_free(list);
}

void test_stress_rand(void)
{
        piojo_skiplist_t *list;
        int elems[TEST_STRESS_COUNT];
        int i,j;

        srand(time(NULL));
        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), my_allocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                elems[i-1] = rand();
                j = elems[i-1] * 10;
                piojo_skiplist_insert(&elems[i-1], &j, list);
        }

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_skiplist_search(&elems[i-1], list);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }
        piojo_skiplist_free(list);
}

void test_stress_set_rand(void)
{
        piojo_skiplist_t *list;
        int elems[TEST_STRESS_COUNT];
        int i,j;

        srand(time(NULL));
        list = piojo_skiplist_alloc_cb_i32k(sizeof(int), my_allocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                elems[i-1] = rand();
                j = elems[i-1] * 10;
                piojo_skiplist_set(&elems[i-1], &j, list);
        }

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_skiplist_search(&elems[i-1], list);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }
        piojo_skiplist_free(list);
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
        test_insert_bool();
        test_set();
        test_delete();
        test_first_last();
        test_next_prev();
        test_search();
        test_stress();
        test_stress_rand();
        test_stress_set_rand();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
