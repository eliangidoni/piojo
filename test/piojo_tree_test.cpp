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
#include <stdlib.h>
#include <time.h>
#include <piojo_test.h>
#include <piojo/piojo_tree.h>

void test_alloc()
{
        piojo_tree_t *tree;

        tree = piojo_tree_alloc_i32k(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);
        piojo_tree_free(tree);

        tree = piojo_tree_alloc_i64k(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);
        piojo_tree_free(tree);

        tree = piojo_tree_alloc_sizk(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);
        piojo_tree_free(tree);
}

void test_copy_def()
{
        piojo_tree_t *tree, *copy;
        int i=10,j;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), piojo_alloc_default);
        piojo_tree_insert(&i, &i, tree);

        copy = piojo_tree_copy(tree);
        PIOJO_ASSERT(copy);

        piojo_tree_free(tree);

        j = *(int*) piojo_tree_search(&i, copy);
        PIOJO_ASSERT(i == j);

        piojo_tree_free(copy);
}

void test_copy()
{
        piojo_tree_t *tree, *copy;
        int i=1;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        piojo_tree_insert(&i, &i, tree);
        assert_allocator_init(2);

        copy = piojo_tree_copy(tree);
        PIOJO_ASSERT(copy);
        assert_allocator_init(4);

        piojo_tree_free(tree);
        assert_allocator_init(2);

        piojo_tree_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        piojo_tree_insert(&i, &j, tree);

        piojo_tree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        piojo_tree_insert(&i, &j, tree);

        piojo_tree_clear(tree);
        assert_allocator_init(0);

        piojo_tree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        ++i;
        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 2);

        ++i;
        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 3);

        piojo_tree_delete(&i, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 2);

        piojo_tree_clear(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_free(tree);
}

void test_insert()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        piojo_tree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_set()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        PIOJO_ASSERT(piojo_tree_set(&i, &j, tree) == TRUE);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);
        PIOJO_ASSERT(*(int*) piojo_tree_search(&i, tree) == j);

        ++j;
        PIOJO_ASSERT(piojo_tree_set(&i, &j, tree) == FALSE);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);
        PIOJO_ASSERT(*(int*) piojo_tree_search(&i, tree) == j);

        piojo_tree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_insertset()
{
        piojo_tree_t *tree;
        int i=1;

        tree = piojo_tree_alloc_i32k(sizeof(bool));
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, NULL, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        piojo_tree_insert(&i, NULL, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);
        piojo_tree_free(tree);
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

void test_search()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_cmp(sizeof(int), my_cmp, sizeof(int));
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 2);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        i-=2;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));

        piojo_tree_free(tree);
}

void test_searchi32()
{
        piojo_tree_t *tree;
        int32_t i=3;
        int j=10;

        tree = piojo_tree_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        --i;
        ++j;
        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 2);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        --i;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        piojo_tree_free(tree);
}

void test_searchi64()
{
        piojo_tree_t *tree;
        int64_t i=3;
        int j=10;

        tree = piojo_tree_alloc_i64k(sizeof(int));
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        --i;
        ++j;
        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 2);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        --i;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        piojo_tree_free(tree);
}

void test_searchsiz()
{
        piojo_tree_t *tree;
        size_t i=3;
        int j=10;

        tree = piojo_tree_alloc_sizk(sizeof(int));
        PIOJO_ASSERT(piojo_tree_size(tree) == 0);

        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        --i;
        ++j;
        piojo_tree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 2);

        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        --i;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_tree_search(&i, tree));
        piojo_tree_free(tree);
}

void test_delete()
{
        piojo_tree_t *tree;
        int i=1, j=10;

        tree = piojo_tree_alloc_i32k(sizeof(int));
        piojo_tree_insert(&i, &j, tree);

        ++i;
        piojo_tree_insert(&i, &j, tree);
        piojo_tree_delete(&i, tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == 1);

        --i;
        j = *(int*) piojo_tree_search(&i, tree);
        PIOJO_ASSERT(10 == j);
        piojo_tree_free(tree);
}

void test_first_next()
{
        piojo_tree_t *tree;
        int i=1, i2=i+1, j=10, j2=j+1, tmp, key;

        tree = piojo_tree_alloc_i32k(sizeof(int));
        piojo_tree_insert(&i, &j, tree);
        piojo_tree_insert(&i2, &j2, tree);

        key = *(const int*)piojo_tree_first(tree, NULL);
        PIOJO_ASSERT(i == key);
        tmp = *(int*) piojo_tree_search(&key, tree);
        PIOJO_ASSERT(j == tmp);

        key = *(const int*)piojo_tree_next(&key, tree, NULL);
        PIOJO_ASSERT(i2 == key);
        tmp = *(int*) piojo_tree_search(&key, tree);
        PIOJO_ASSERT(j2 == tmp);
        piojo_tree_free(tree);
}

void test_last_prev()
{
        piojo_tree_t *tree;
        int i=1, i2=i+1, j=10, j2=j+1, tmp, key;

        tree = piojo_tree_alloc_i32k(sizeof(int));
        piojo_tree_insert(&i, &j, tree);
        piojo_tree_insert(&i2, &j2, tree);

        key = *(const int*)piojo_tree_last(tree, NULL);
        PIOJO_ASSERT(i2 == key);
        tmp = *(int*) piojo_tree_search(&key, tree);
        PIOJO_ASSERT(j2 == tmp);

        key = *(const int*)piojo_tree_prev(&key, tree, NULL);
        PIOJO_ASSERT(i == key);
        tmp = *(int*) piojo_tree_search(&key, tree);
        PIOJO_ASSERT(j == tmp);
        piojo_tree_free(tree);
}

void test_tree_expand()
{
        piojo_tree_t *tree;
        int i,j;

        tree = piojo_tree_alloc_i32k(sizeof(int));
        for (i = 1023; i >= 0; --i){
                j = i * 10;
                piojo_tree_insert(&i, &j, tree);
        }

        PIOJO_ASSERT(piojo_tree_size(tree) == 1024);

        for (i = 0; i < 1024; ++i){
                j = *(int*) piojo_tree_search(&i, tree);
                PIOJO_ASSERT(j == i * 10);
        }

        piojo_tree_free(tree);
}

void test_stress()
{
        piojo_tree_t *tree, *copy;
        int i,j,k;
        bool deleted_p;

        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = i * 10;
                piojo_tree_insert(&i, &j, tree);
        }

        PIOJO_ASSERT(piojo_tree_size(tree) == TEST_STRESS_COUNT);

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_tree_search(&i, tree);
                PIOJO_ASSERT(j == i * 10);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_tree_search(&i, tree);
                PIOJO_ASSERT(j == i * 10);
        }

        j = *(const int*)piojo_tree_first(tree, NULL);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                PIOJO_ASSERT(i == j);
                k = *(int*) piojo_tree_search(&j, tree);
                PIOJO_ASSERT(i * 10 == k);
                if (piojo_tree_next(&j, tree, NULL)){
                        j = *(const int*)piojo_tree_next(&j, tree, NULL);
                }
        }

        j = *(const int*)piojo_tree_last(tree, NULL);
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                PIOJO_ASSERT(i == j);
                k = *(int*) piojo_tree_search(&j, tree);
                PIOJO_ASSERT(i * 10 == k);
                if (piojo_tree_prev(&j, tree, NULL)){
                        j = *(const int*)piojo_tree_prev(&j, tree, NULL);
                }
        }

        copy = piojo_tree_copy(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == piojo_tree_size(copy));

        for (i = TEST_STRESS_COUNT/2; i > 0; --i){
                j = *(const int*)piojo_tree_last(tree, NULL);
                PIOJO_ASSERT(TEST_STRESS_COUNT == j);

                j = *(const int*)piojo_tree_first(tree, NULL);
                PIOJO_ASSERT(1 == j);

                deleted_p = piojo_tree_delete(&i, tree);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        for (i = TEST_STRESS_COUNT; i > TEST_STRESS_COUNT/2; --i){
                j = *(const int*)piojo_tree_last(tree, NULL);
                PIOJO_ASSERT(i == j);


                j = *(const int*)piojo_tree_first(tree, NULL);
                PIOJO_ASSERT((TEST_STRESS_COUNT/2) + 1 == j);

                deleted_p = piojo_tree_delete(&i, tree);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        PIOJO_ASSERT(piojo_tree_size(tree) == 0);
        piojo_tree_free(tree);
        piojo_tree_free(copy);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_stress_rand_uniq()
{
        piojo_tree_t *tree, *copy;
        int elems[TEST_STRESS_COUNT];
        int i,j;
        bool deleted_p;
        std::map<int,bool>unique;

        srand(time(NULL));
        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                elems[i-1] = rand();
                while (unique.count(elems[i-1]) > 0){
                        elems[i-1] = rand();
                }
                unique[elems[i-1]]=true;
                j = elems[i-1] * 10;
                piojo_tree_insert(&elems[i-1], &j, tree);
        }

        PIOJO_ASSERT(piojo_tree_size(tree) == TEST_STRESS_COUNT);

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_tree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_tree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        j = *(const int*)piojo_tree_first(tree, NULL);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                PIOJO_ASSERT(j * 10 == *(int*) piojo_tree_search(&j, tree));
                if (piojo_tree_next(&j, tree, NULL)){
                        j = *(const int*)piojo_tree_next(&j, tree, NULL);
                }
        }

        j = *(const int*)piojo_tree_last(tree, NULL);
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                PIOJO_ASSERT(j * 10 == *(int*) piojo_tree_search(&j, tree));
                if (piojo_tree_prev(&j, tree, NULL)){
                        j = *(const int*)piojo_tree_prev(&j, tree, NULL);
                }
        }

        copy = piojo_tree_copy(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == piojo_tree_size(copy));

        for (i = TEST_STRESS_COUNT/2; i > 0; --i){
                j = *(const int*)piojo_tree_last(tree, NULL);
                PIOJO_ASSERT(unique.rbegin()->first == j);

                j = *(const int*)piojo_tree_first(tree, NULL);
                PIOJO_ASSERT(unique.begin()->first == j);
                unique.erase(elems[i-1]);
                deleted_p = piojo_tree_delete(&elems[i-1], tree);

                PIOJO_ASSERT(deleted_p == TRUE);
        }

        for (i = TEST_STRESS_COUNT; i > TEST_STRESS_COUNT/2; --i){
                j = *(const int*)piojo_tree_last(tree, NULL);
                PIOJO_ASSERT(unique.rbegin()->first == j);

                j = *(const int*)piojo_tree_first(tree, NULL);
                PIOJO_ASSERT(unique.begin()->first == j);

                unique.erase(elems[i-1]);
                deleted_p = piojo_tree_delete(&elems[i-1], tree);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        PIOJO_ASSERT(piojo_tree_size(tree) == 0);
        piojo_tree_free(copy);
        piojo_tree_free(tree);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_stress_rand()
{
        piojo_tree_t *tree, *copy;
        int elems[TEST_STRESS_COUNT];
        int i,j;
        size_t tmp;

        srand(time(NULL));
        tree = piojo_tree_alloc_cb_i32k(sizeof(int), my_allocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                elems[i-1] = rand();
                j = elems[i-1] * 10;
                piojo_tree_insert(&elems[i-1], &j, tree);
        }

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_tree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_tree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        j = *(const int*)piojo_tree_first(tree, NULL);
        for (tmp = 0; tmp < piojo_tree_size(tree); ++tmp){
                PIOJO_ASSERT(j * 10 == *(int*) piojo_tree_search(&j, tree));
                if (piojo_tree_next(&j, tree, NULL)){
                        j = *(const int*)piojo_tree_next(&j, tree, NULL);
                }
        }

        j = *(const int*)piojo_tree_last(tree, NULL);
        for (tmp = 0; tmp < piojo_tree_size(tree); ++tmp){
                PIOJO_ASSERT(j * 10 == *(int*) piojo_tree_search(&j, tree));
                if (piojo_tree_prev(&j, tree, NULL)){
                        j = *(const int*)piojo_tree_prev(&j, tree, NULL);
                }
        }

        copy = piojo_tree_copy(tree);
        PIOJO_ASSERT(piojo_tree_size(tree) == piojo_tree_size(copy));

        for (i = TEST_STRESS_COUNT/2; i > 0; --i){
                piojo_tree_delete(&elems[i-1], tree);
        }

        for (i = TEST_STRESS_COUNT; i > TEST_STRESS_COUNT/2; --i){
                piojo_tree_delete(&elems[i-1], tree);
        }

        PIOJO_ASSERT(piojo_tree_size(tree) == 0);
        piojo_tree_free(copy);
        piojo_tree_free(tree);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

int main()
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_free();
        test_clear();
        test_size();
        test_insert();
        test_insertset();
        test_set();
        test_search();
        test_searchi32();
        test_searchi64();
        test_searchsiz();
        test_delete();
        test_first_next();
        test_last_prev();
        test_tree_expand();
        test_stress();
        test_stress_rand_uniq();
        test_stress_rand();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
