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
#include <piojo/piojo_btree.h>

void test_alloc()
{
        piojo_btree_t *tree;

        tree = piojo_btree_alloc_intk(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(tree);

        tree = piojo_btree_alloc_i32k(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(tree);

        tree = piojo_btree_alloc_i64k(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(tree);

        tree = piojo_btree_alloc_sizk(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(tree);

        tree = piojo_btree_alloc_ptrk(2);
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(tree);
}

void test_copy_def()
{
        piojo_btree_t *tree, *copy;
        int i=10,j;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), piojo_alloc_kv_default);
        piojo_btree_insert(&i, &i, tree);

        copy = piojo_btree_copy(tree);
        PIOJO_ASSERT(copy);

        piojo_btree_free(tree);

        j = *(int*) piojo_btree_search(&i, copy);
        PIOJO_ASSERT(i == j);

        piojo_btree_free(copy);
}

void test_copy_ptr()
{
        piojo_btree_t *tree, *copy;
        const char * i="test";
        int j=10;

        tree = piojo_btree_alloc_cb_ptrk(4, sizeof(int), piojo_alloc_kv_default);
        piojo_btree_insert(&i, &j, tree);

        copy = piojo_btree_copy(tree);
        PIOJO_ASSERT(copy);

        piojo_btree_free(tree);

        j = *(int*) piojo_btree_search(&i, copy);
        PIOJO_ASSERT(10 == j);

        piojo_btree_free(copy);
}

void test_copy()
{
        piojo_btree_t *tree, *copy;
        int i=1;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        piojo_btree_insert(&i, &i, tree);
        assert_allocator_init(2);

        copy = piojo_btree_copy(tree);
        PIOJO_ASSERT(copy);
        assert_allocator_init(4);

        piojo_btree_free(tree);
        assert_allocator_init(2);

        piojo_btree_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        piojo_btree_insert(&i, &j, tree);

        piojo_btree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        piojo_btree_insert(&i, &j, tree);

        piojo_btree_clear(tree);
        assert_allocator_init(0);

        piojo_btree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_intk(sizeof(int));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        ++i;
        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        ++i;
        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 3);

        piojo_btree_delete(&i, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        piojo_btree_clear(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_free(tree);
}

void test_insert()
{
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        piojo_btree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_set()
{
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        PIOJO_ASSERT(piojo_btree_set(&i, &j, tree) == TRUE);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);
        PIOJO_ASSERT(*(int*) piojo_btree_search(&i, tree) == j);

        ++j;
        PIOJO_ASSERT(piojo_btree_set(&i, &j, tree) == FALSE);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);
        PIOJO_ASSERT(*(int*) piojo_btree_search(&i, tree) == j);

        piojo_btree_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_insertset()
{
        piojo_btree_t *tree;
        int i=1;

        tree = piojo_btree_alloc_intk(sizeof(bool));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, NULL, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        piojo_btree_insert(&i, NULL, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);
        piojo_btree_free(tree);
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
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_cmp(sizeof(int), my_cmp, sizeof(int));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        i-=2;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));

        piojo_btree_free(tree);
}

void test_searchi32()
{
        piojo_btree_t *tree;
        int32_t i=3;
        int j=10;

        tree = piojo_btree_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        --i;
        ++j;
        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        --i;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        piojo_btree_free(tree);
}

void test_searchi64()
{
        piojo_btree_t *tree;
        int64_t i=3;
        int j=10;

        tree = piojo_btree_alloc_i64k(sizeof(int));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        --i;
        ++j;
        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        --i;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        piojo_btree_free(tree);
}

void test_searchsiz()
{
        piojo_btree_t *tree;
        size_t i=3;
        int j=10;

        tree = piojo_btree_alloc_sizk(sizeof(int));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(10 == j);

        --i;
        ++j;
        piojo_btree_insert(&i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(11 == j);

        --i;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        i=100;
        PIOJO_ASSERT(NULL == piojo_btree_search(&i, tree));
        piojo_btree_free(tree);
}

void test_searchptr()
{
        piojo_btree_t *tree;
        const char *i="tzest";
        int j=10;

        tree = piojo_btree_alloc_ptrk(sizeof(int));
        PIOJO_ASSERT(piojo_btree_size(tree) == 0);

        piojo_btree_insert(i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        j = *(int*) piojo_btree_search(i, tree);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_btree_insert(i, &j, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 2);

        j = *(int*) piojo_btree_search(i, tree);
        PIOJO_ASSERT(11 == j);
        piojo_btree_free(tree);
}

void test_delete()
{
        piojo_btree_t *tree;
        int i=1, j=10;

        tree = piojo_btree_alloc_intk(sizeof(int));
        piojo_btree_insert(&i, &j, tree);

        ++i;
        piojo_btree_insert(&i, &j, tree);
        piojo_btree_delete(&i, tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == 1);

        --i;
        j = *(int*) piojo_btree_search(&i, tree);
        PIOJO_ASSERT(10 == j);
        piojo_btree_free(tree);
}

void test_first_next()
{
        piojo_btree_node_t *next, node;
        piojo_btree_t *tree;
        int i=1, i2=i+1, j=10, j2=j+1, tmp;

        tree = piojo_btree_alloc_intk(sizeof(int));
        piojo_btree_insert(&i, &j, tree);
        piojo_btree_insert(&i2, &j2, tree);

        next = piojo_btree_first(tree, &node);
        tmp = *(int*) piojo_btree_entryk(next);
        PIOJO_ASSERT(i == tmp);
        tmp = *(int*) piojo_btree_entryv(next);
        PIOJO_ASSERT(j == tmp);

        next = piojo_btree_next(next);
        tmp = *(int*) piojo_btree_entryk(next);
        PIOJO_ASSERT(i2 == tmp);
        tmp = *(int*) piojo_btree_entryv(next);
        PIOJO_ASSERT(j2 == tmp);
        piojo_btree_free(tree);
}

void test_last_prev()
{
        piojo_btree_node_t *next, node;
        piojo_btree_t *tree;
        int i=1, i2=i+1, j=10, j2=j+1, tmp;

        tree = piojo_btree_alloc_intk(sizeof(int));
        piojo_btree_insert(&i, &j, tree);
        piojo_btree_insert(&i2, &j2, tree);

        next = piojo_btree_last(tree, &node);
        tmp = *(int*) piojo_btree_entryk(next);
        PIOJO_ASSERT(i2 == tmp);
        tmp = *(int*) piojo_btree_entryv(next);
        PIOJO_ASSERT(j2 == tmp);

        next = piojo_btree_prev(next);
        tmp = *(int*) piojo_btree_entryk(next);
        PIOJO_ASSERT(i == tmp);
        tmp = *(int*) piojo_btree_entryv(next);
        PIOJO_ASSERT(j == tmp);
        piojo_btree_free(tree);
}

void test_tree_expand()
{
        piojo_btree_t *tree;
        int i,j;

        tree = piojo_btree_alloc_intk(sizeof(int));
        for (i = 1023; i >= 0; --i){
                j = i * 10;
                piojo_btree_insert(&i, &j, tree);
        }

        PIOJO_ASSERT(piojo_btree_size(tree) == 1024);

        for (i = 0; i < 1024; ++i){
                j = *(int*) piojo_btree_search(&i, tree);
                PIOJO_ASSERT(j == i * 10);
        }

        piojo_btree_free(tree);
}

void test_stress()
{
        piojo_btree_t *tree, *copy;
        piojo_btree_node_t *next,node;
        int i,j;
        bool deleted_p;

        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = i * 10;
                piojo_btree_insert(&i, &j, tree);
        }

        PIOJO_ASSERT(piojo_btree_size(tree) == TEST_STRESS_COUNT);

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_btree_search(&i, tree);
                PIOJO_ASSERT(j == i * 10);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_btree_search(&i, tree);
                PIOJO_ASSERT(j == i * 10);
        }

        next = piojo_btree_first(tree, &node);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_btree_entryk(next);
                PIOJO_ASSERT(i == j);
                j = *(int*) piojo_btree_entryv(next);
                PIOJO_ASSERT(i * 10 == j);
                next = piojo_btree_next(next);
        }

        next = piojo_btree_last(tree, &node);
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_btree_entryk(next);
                PIOJO_ASSERT(i == j);
                j = *(int*) piojo_btree_entryv(next);
                PIOJO_ASSERT(i * 10 == j);
                next = piojo_btree_prev(next);
        }

        copy = piojo_btree_copy(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == piojo_btree_size(copy));

        for (i = TEST_STRESS_COUNT/2; i > 0; --i){
                piojo_btree_last(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(TEST_STRESS_COUNT == j);

                piojo_btree_first(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(1 == j);

                deleted_p = piojo_btree_delete(&i, tree);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        for (i = TEST_STRESS_COUNT; i > TEST_STRESS_COUNT/2; --i){
                piojo_btree_last(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(i == j);

                piojo_btree_first(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT((TEST_STRESS_COUNT/2) + 1 == j);

                deleted_p = piojo_btree_delete(&i, tree);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(tree);
        piojo_btree_free(copy);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_stress_rand_uniq()
{
        piojo_btree_t *tree, *copy;
        piojo_btree_node_t *next,node;
        int elems[TEST_STRESS_COUNT];
        int i,j;
        bool deleted_p;
        std::map<int,bool>unique;

        srand(time(NULL));
        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                elems[i-1] = rand();
                while (unique.count(elems[i-1]) > 0){
                        elems[i-1] = rand();
                }
                unique[elems[i-1]]=true;
                j = elems[i-1] * 10;
                piojo_btree_insert(&elems[i-1], &j, tree);
        }

        PIOJO_ASSERT(piojo_btree_size(tree) == TEST_STRESS_COUNT);

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_btree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_btree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        next = piojo_btree_first(tree, &node);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_btree_entryk(next);
                PIOJO_ASSERT(j * 10 == *(int*) piojo_btree_entryv(next));
                next = piojo_btree_next(next);
        }

        next = piojo_btree_last(tree, &node);
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_btree_entryk(next);
                PIOJO_ASSERT(j * 10 == *(int*) piojo_btree_entryv(next));
                next = piojo_btree_prev(next);
        }

        copy = piojo_btree_copy(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == piojo_btree_size(copy));

        for (i = TEST_STRESS_COUNT/2; i > 0; --i){
                piojo_btree_last(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(unique.rbegin()->first == j);

                piojo_btree_first(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(unique.begin()->first == j);
                unique.erase(elems[i-1]);
                deleted_p = piojo_btree_delete(&elems[i-1], tree);

                PIOJO_ASSERT(deleted_p == TRUE);
        }

        for (i = TEST_STRESS_COUNT; i > TEST_STRESS_COUNT/2; --i){
                piojo_btree_last(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(unique.rbegin()->first == j);

                piojo_btree_first(tree, &node);
                j = *(int*) piojo_btree_entryk(&node);
                PIOJO_ASSERT(unique.begin()->first == j);

                unique.erase(elems[i-1]);
                deleted_p = piojo_btree_delete(&elems[i-1], tree);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(copy);
        piojo_btree_free(tree);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_stress_rand()
{
        piojo_btree_t *tree, *copy;
        piojo_btree_node_t *next,node;
        int elems[TEST_STRESS_COUNT];
        int i,j;
        size_t tmp;

        srand(time(NULL));
        tree = piojo_btree_alloc_cb_intk(4, sizeof(int), my_kvallocator);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                elems[i-1] = rand();
                j = elems[i-1] * 10;
                piojo_btree_insert(&elems[i-1], &j, tree);
        }

        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = *(int*) piojo_btree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_btree_search(&elems[i-1], tree);
                PIOJO_ASSERT(j == elems[i-1] * 10);
        }

        next = piojo_btree_first(tree, &node);
        for (tmp = 0; tmp < piojo_btree_size(tree); ++tmp){
                j = *(int*) piojo_btree_entryk(next);
                PIOJO_ASSERT(j * 10 == *(int*) piojo_btree_entryv(next));
                next = piojo_btree_next(next);
        }

        next = piojo_btree_last(tree, &node);
        for (tmp = 0; tmp < piojo_btree_size(tree); ++tmp){
                j = *(int*) piojo_btree_entryk(next);
                PIOJO_ASSERT(j * 10 == *(int*) piojo_btree_entryv(next));
                next = piojo_btree_prev(next);
        }

        copy = piojo_btree_copy(tree);
        PIOJO_ASSERT(piojo_btree_size(tree) == piojo_btree_size(copy));

        for (i = TEST_STRESS_COUNT/2; i > 0; --i){
                piojo_btree_delete(&elems[i-1], tree);
        }

        for (i = TEST_STRESS_COUNT; i > TEST_STRESS_COUNT/2; --i){
                piojo_btree_delete(&elems[i-1], tree);
        }

        PIOJO_ASSERT(piojo_btree_size(tree) == 0);
        piojo_btree_free(copy);
        piojo_btree_free(tree);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

int main()
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_copy_ptr();
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
        test_searchptr();
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
