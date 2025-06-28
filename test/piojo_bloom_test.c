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
#include <piojo/piojo_bloom.h>

void test_alloc(void)
{
        piojo_bloom_t *bloom;

        bloom = piojo_bloom_alloc_i32k(1000, 0.01f);
        PIOJO_ASSERT(bloom);
        piojo_bloom_free(bloom);

        bloom = piojo_bloom_alloc_i64k(1000, 0.01f);
        PIOJO_ASSERT(bloom);
        piojo_bloom_free(bloom);

        bloom = piojo_bloom_alloc_sizk(1000, 0.01f);
        PIOJO_ASSERT(bloom);
        piojo_bloom_free(bloom);
}

void test_copy_def(void)
{
        piojo_bloom_t *bloom, *copy;
        int i=1;

        bloom = piojo_bloom_alloc_cb_i32k(1000, 0.01f, piojo_alloc_default);
        piojo_bloom_insert(&i, bloom);

        copy = piojo_bloom_copy(bloom);
        PIOJO_ASSERT(copy);

        piojo_bloom_free(bloom);

        bool j = piojo_bloom_search(&i, copy);
        PIOJO_ASSERT(j);

        ++i;
        j = piojo_bloom_search(&i, copy);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(copy);
}

void test_copy(void)
{
        piojo_bloom_t *bloom, *copy;
        int i=1;

        bloom = piojo_bloom_alloc_cb_i32k(1000, 0.01f, my_allocator);
        piojo_bloom_insert(&i, bloom);
        assert_allocator_init(2);

        copy = piojo_bloom_copy(bloom);
        PIOJO_ASSERT(copy);
        assert_allocator_init(4);

        piojo_bloom_free(bloom);
        assert_allocator_init(2);

        bool j = piojo_bloom_search(&i, copy);
        PIOJO_ASSERT(j);

        ++i;
        j = piojo_bloom_search(&i, copy);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free(void)
{
        piojo_bloom_t *bloom;
        int i=1;

        bloom = piojo_bloom_alloc_cb_i32k(1000, 0.01f, my_allocator);
        piojo_bloom_insert(&i, bloom);

        piojo_bloom_free(bloom);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear(void)
{
        piojo_bloom_t *bloom;
        int i=1;

        bloom = piojo_bloom_alloc_cb_i32k(1000, 0.01f, my_allocator);
        piojo_bloom_insert(&i, bloom);
        bool j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        piojo_bloom_clear(bloom);
        assert_allocator_init(0);

        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(bloom);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_insert(void)
{
        piojo_bloom_t *bloom;
        int i=1;

        bloom = piojo_bloom_alloc_cb_i32k(1000, 0.01f, my_allocator);
        piojo_bloom_insert(&i, bloom);
        piojo_bloom_insert(&i, bloom);
        piojo_bloom_free(bloom);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_search(void)
{
        piojo_bloom_t *bloom;
        int i=1;

        bloom = piojo_bloom_alloc_eq(1000, 0.01f, sizeof(int));

        piojo_bloom_insert(&i, bloom);
        bool j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        ++i;
        piojo_bloom_insert(&i, bloom);
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        i = 0xbeef;
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(bloom);
}

void test_search32(void)
{
        piojo_bloom_t *bloom;
        int32_t i=1;

        bloom = piojo_bloom_alloc_i32k(1000, 0.01f);

        piojo_bloom_insert(&i, bloom);
        bool j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        ++i;
        piojo_bloom_insert(&i, bloom);
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        i = 0xbeef;
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(bloom);
}

void test_search64(void)
{
        piojo_bloom_t *bloom;
        int64_t i=1;

        bloom = piojo_bloom_alloc_i64k(1000, 0.01f);

        piojo_bloom_insert(&i, bloom);
        bool j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        ++i;
        piojo_bloom_insert(&i, bloom);
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        i = 0xbeef;
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(bloom);
}

void test_searchsiz(void)
{
        piojo_bloom_t *bloom;
        size_t i=1;

        bloom = piojo_bloom_alloc_sizk(1000, 0.01f);

        piojo_bloom_insert(&i, bloom);
        bool j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        ++i;
        piojo_bloom_insert(&i, bloom);
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(j);

        i = 0xbeef;
        j = piojo_bloom_search(&i, bloom);
        PIOJO_ASSERT(!j);

        piojo_bloom_free(bloom);
}

void test_stress(void)
{
        piojo_bloom_t *bloom;
        int i;
        bool j;

        bloom = piojo_bloom_alloc_i32k(TEST_STRESS_COUNT, 0.01f);
        for (i = TEST_STRESS_COUNT; i > 0; --i){
                piojo_bloom_insert(&i, bloom);
        }
        for (i = 1; i <= TEST_STRESS_COUNT; ++i){
                j = piojo_bloom_search(&i, bloom);
                PIOJO_ASSERT(j);
        }
        piojo_bloom_free(bloom);
}

int main(void)
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_free();
        test_clear();
        test_insert();
        test_search();
        test_search32();
        test_search64();
        test_searchsiz();
        test_stress();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
