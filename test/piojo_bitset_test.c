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
#include <piojo/piojo_bitset.h>

void test_init()
{
        piojo_bitset_t *bitset;

        bitset = piojo_bitset_alloc(1);
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        piojo_bitset_free(bitset);

        bitset = piojo_bitset_alloc_cb(3, my_allocator);
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_clear()
{
        piojo_bitset_t *bitset;

        bitset = piojo_bitset_alloc(32);
        piojo_bitset_set(2, bitset);
        piojo_bitset_clear(bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_empty_p()
{
        piojo_bitset_t *bitset;

        bitset = piojo_bitset_alloc(32);
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));

        piojo_bitset_set(2, bitset);
        PIOJO_ASSERT(! piojo_bitset_empty_p(bitset));
        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_full_p()
{
        piojo_bitset_t *bitset;

        bitset = piojo_bitset_alloc(2);
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_set(0, bitset);
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_set(1, bitset);
        PIOJO_ASSERT(piojo_bitset_full_p(bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_set_p()
{
        piojo_bitset_t *bitset;

        bitset = piojo_bitset_alloc(2);
        PIOJO_ASSERT(! piojo_bitset_set_p(0, bitset));
        PIOJO_ASSERT(! piojo_bitset_set_p(1, bitset));

        piojo_bitset_set(0, bitset);
        PIOJO_ASSERT(piojo_bitset_set_p(0, bitset));
        PIOJO_ASSERT(! piojo_bitset_set_p(1, bitset));

        piojo_bitset_set(1, bitset);
        PIOJO_ASSERT(piojo_bitset_set_p(0, bitset));
        PIOJO_ASSERT(piojo_bitset_set_p(1, bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_set()
{
        piojo_bitset_t *bitset;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        for (i = 0; i < 132; ++i){
                piojo_bitset_set(i, bitset);
                PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
        }
        PIOJO_ASSERT(! piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_full_p(bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_unset()
{
        piojo_bitset_t *bitset;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        for (i = 0; i < 132; ++i){
                piojo_bitset_set(i, bitset);
                PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));

                piojo_bitset_unset(i, bitset);
                PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
        }
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_copy()
{
        piojo_bitset_t *bitset, *copy;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        for (i = 0; i < 132; ++i){
                piojo_bitset_set(i, bitset);
                PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
        }
        PIOJO_ASSERT(! piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_full_p(bitset));

        copy = piojo_bitset_copy(bitset);
        piojo_bitset_free(bitset);

        PIOJO_ASSERT(! piojo_bitset_empty_p(copy));
        PIOJO_ASSERT(piojo_bitset_full_p(copy));
        piojo_bitset_clear(copy);
        PIOJO_ASSERT(piojo_bitset_empty_p(copy));
        piojo_bitset_free(copy);

        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

int main()
{
        test_init();
        test_clear();
        test_set();
        test_set_p();
        test_full_p();
        test_empty_p();
        test_unset();
        test_copy();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
