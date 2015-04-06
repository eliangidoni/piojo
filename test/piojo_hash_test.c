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
#include <piojo/piojo_hash.h>

void test_alloc()
{
        piojo_hash_t *hash;

        hash = piojo_hash_alloc_intk(2);
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);
        piojo_hash_free(hash);

        hash = piojo_hash_alloc_i32k(2);
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);
        piojo_hash_free(hash);

        hash = piojo_hash_alloc_i64k(2);
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);
        piojo_hash_free(hash);

        hash = piojo_hash_alloc_sizk(2);
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);
        piojo_hash_free(hash);

        hash = piojo_hash_alloc_ptrk(2);
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);
        piojo_hash_free(hash);
}

void test_copy_def()
{
        piojo_hash_t *hash, *copy;
        int i=1, j=0;

        hash = piojo_hash_alloc_cb_intk(sizeof(int), piojo_alloc_default);
        piojo_hash_insert(&i, &i, hash);

        copy = piojo_hash_copy(hash);
        PIOJO_ASSERT(copy);

        piojo_hash_free(hash);

        j = *(int*) piojo_hash_search(&i, copy);
        PIOJO_ASSERT(i == j);

        piojo_hash_free(copy);
}

void test_copyptr()
{
        piojo_hash_t *hash, *copy;
        const char* i="test";
        int j=10;

        hash = piojo_hash_alloc_cb_ptrk(sizeof(int), my_allocator);
        piojo_hash_insert(i, &j, hash);
        assert_allocator_init(2);

        copy = piojo_hash_copy(hash);
        PIOJO_ASSERT(copy);
        assert_allocator_init(4);

        piojo_hash_free(hash);
        assert_allocator_init(2);

        j = *(int*) piojo_hash_search(i, copy);
        PIOJO_ASSERT(j == 10);

        piojo_hash_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_copy()
{
        piojo_hash_t *hash, *copy;
        int i=1, j=0;

        hash = piojo_hash_alloc_cb_intk(sizeof(int), my_allocator);
        piojo_hash_insert(&i, &i, hash);
        assert_allocator_init(2);

        copy = piojo_hash_copy(hash);
        PIOJO_ASSERT(copy);
        assert_allocator_init(4);

        piojo_hash_free(hash);
        assert_allocator_init(2);

        j = *(int*) piojo_hash_search(&i, copy);
        PIOJO_ASSERT(i == j);

        piojo_hash_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
{
        piojo_hash_t *hash;
        int i=1, j=10;

        hash = piojo_hash_alloc_cb_intk(sizeof(int), my_allocator);
        piojo_hash_insert(&i, &j, hash);

        piojo_hash_free(hash);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear()
{
        piojo_hash_t *hash;
        int i=1, j=10;

        hash = piojo_hash_alloc_cb_intk(sizeof(int), my_allocator);
        piojo_hash_insert(&i, &j, hash);

        piojo_hash_clear(hash);
        assert_allocator_init(0);

        piojo_hash_free(hash);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_size()
{
        piojo_hash_t *hash;
        int i=1, j=10;

        hash = piojo_hash_alloc_intk(sizeof(int));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        ++i;
        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        ++i;
        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 3);

        piojo_hash_delete(&i, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        piojo_hash_clear(hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_free(hash);
}

void test_insert()
{
        piojo_hash_t *hash;
        int i=1;

        hash = piojo_hash_alloc_cb_intk(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, &i, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        piojo_hash_insert(&i, &i, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        piojo_hash_free(hash);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_insertset()
{
        piojo_hash_t *hash;
        int i=1;

        hash = piojo_hash_alloc_intk(sizeof(bool));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, NULL, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        piojo_hash_insert(&i, NULL, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        piojo_hash_free(hash);
}

void test_set()
{
        piojo_hash_t *hash;
        int i=1, j=10;

        hash = piojo_hash_alloc_cb_intk(sizeof(int), my_allocator);
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        PIOJO_ASSERT(piojo_hash_set(&i, &j, hash) == TRUE);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);
        PIOJO_ASSERT(*(int*) piojo_hash_search(&i, hash) == j);

        ++j;
        PIOJO_ASSERT(piojo_hash_set(&i, &j, hash) == FALSE);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);
        PIOJO_ASSERT(*(int*) piojo_hash_search(&i, hash) == j);

        piojo_hash_free(hash);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

static bool
my_eq(const void *e1, const void *e2)
{
        int v1 = *(int*) e1;
        int v2 = *(int*) e2;
        if (v1 == v2){
                return TRUE;
        }
        return FALSE;
}

void test_search()
{
        piojo_hash_t *hash;
        int i=1, j=10;

        hash = piojo_hash_alloc_eq(sizeof(int), my_eq, sizeof(int));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(11 == j);

        piojo_hash_free(hash);
}

void test_search32()
{
        piojo_hash_t *hash;
        int32_t i=1;
        int j=10;

        hash = piojo_hash_alloc_i32k(sizeof(int));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(11 == j);

        piojo_hash_free(hash);
}

void test_search64()
{
        piojo_hash_t *hash;
        int64_t i=1;
        int j=10;

        hash = piojo_hash_alloc_i64k(sizeof(int));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(11 == j);

        piojo_hash_free(hash);
}

void test_searchsiz()
{
        piojo_hash_t *hash;
        size_t i=1;
        int j=10;

        hash = piojo_hash_alloc_sizk(sizeof(int));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_hash_insert(&i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(11 == j);

        piojo_hash_free(hash);
}

void test_searchptr()
{
        piojo_hash_t *hash;
        const char *i="test";
        int j=10;

        hash = piojo_hash_alloc_ptrk(sizeof(int));
        PIOJO_ASSERT(piojo_hash_size(hash) == 0);

        piojo_hash_insert(i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        j = *(int*) piojo_hash_search(i, hash);
        PIOJO_ASSERT(10 == j);

        ++i;
        ++j;
        piojo_hash_insert(i, &j, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 2);

        j = *(int*) piojo_hash_search(i, hash);
        PIOJO_ASSERT(11 == j);

        piojo_hash_free(hash);
}

void test_delete()
{
        piojo_hash_t *hash;
        int i=1, j=10;

        hash = piojo_hash_alloc_intk(sizeof(int));
        piojo_hash_insert(&i, &j, hash);

        ++i;
        piojo_hash_insert(&i, &j, hash);
        piojo_hash_delete(&i, hash);
        PIOJO_ASSERT(piojo_hash_size(hash) == 1);

        --i;
        j = *(int*) piojo_hash_search(&i, hash);
        PIOJO_ASSERT(10 == j);

        piojo_hash_free(hash);
}

void test_first_next()
{
        piojo_hash_t *hash;
        int i=1, i2=i+1, j=10, j2=j+1, tmp, key;

        hash = piojo_hash_alloc_intk(sizeof(int));
        piojo_hash_insert(&i, &j, hash);
        piojo_hash_insert(&i2, &j2, hash);

        piojo_hash_first(hash, &key);
        PIOJO_ASSERT(i == key || i2 == key);
        tmp = *(int*) piojo_hash_search(&key, hash);
        PIOJO_ASSERT(j == tmp || j2 == tmp);

        piojo_hash_next(hash, &key);
        PIOJO_ASSERT(i == key || i2 == key);
        tmp = *(int*) piojo_hash_search(&key, hash);
        PIOJO_ASSERT(j == tmp || j2 == tmp);

        piojo_hash_free(hash);
}

void test_hash_expand()
{
        piojo_hash_t *hash;
        int i,j;

        hash = piojo_hash_alloc_intk(sizeof(int));
        for (i = 0; i < 1024; ++i){
                j = i * 10;
                piojo_hash_insert(&i, &j, hash);
        }

        PIOJO_ASSERT(piojo_hash_size(hash) == 1024);

        for (i = 0; i < 1024; ++i){
                j = *(int*) piojo_hash_search(&i, hash);
                PIOJO_ASSERT(j == i * 10);
        }
        piojo_hash_free(hash);
}

void test_stress()
{
        piojo_hash_t *hash;
        int i,j;
        bool deleted_p;

        hash = piojo_hash_alloc_intk(sizeof(int));
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = i * 10;
                piojo_hash_insert(&i, &j, hash);
        }
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                j = i * 10;
                piojo_hash_insert(&i, &j, hash);
        }


        PIOJO_ASSERT(piojo_hash_size(hash) == TEST_STRESS_COUNT);

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = *(int*) piojo_hash_search(&i, hash);
                PIOJO_ASSERT(j == i * 10);
        }

        piojo_hash_first(hash, &j);
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                PIOJO_ASSERT(*(int*) piojo_hash_search(&j, hash) == j * 10);
                piojo_hash_next(hash, &j);
        }

        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                deleted_p = piojo_hash_delete(&i, hash);
                PIOJO_ASSERT(deleted_p == TRUE);
        }

        piojo_hash_free(hash);
}

int main()
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_copyptr();
        test_free();
        test_clear();
        test_size();
        test_insert();
        test_insertset();
        test_set();
        test_search();
        test_search32();
        test_search64();
        test_searchsiz();
        test_searchptr();
        test_delete();
        test_first_next();
        test_hash_expand();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
