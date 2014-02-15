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
        piojo_bitset_t bitset;

        piojo_bitset_init(&bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(&bitset));

        piojo_bitset_init_m(3, &bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(&bitset));
}

void test_clear()
{
        piojo_bitset_t bitset;

        piojo_bitset_init(&bitset);
        piojo_bitset_set(2, &bitset);
        piojo_bitset_clear(&bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(&bitset));
}

void test_empty_p()
{
        piojo_bitset_t bitset;

        piojo_bitset_init(&bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(&bitset));

        piojo_bitset_set(2, &bitset);
        PIOJO_ASSERT(! piojo_bitset_empty_p(&bitset));
}

void test_full_p()
{
        piojo_bitset_t bitset;

        piojo_bitset_init_m(2, &bitset);
        PIOJO_ASSERT(! piojo_bitset_full_p(&bitset));

        piojo_bitset_set(0, &bitset);
        PIOJO_ASSERT(! piojo_bitset_full_p(&bitset));

        piojo_bitset_set(1, &bitset);
        PIOJO_ASSERT(piojo_bitset_full_p(&bitset));
}

void test_set_p()
{
        piojo_bitset_t bitset;

        piojo_bitset_init_m(2, &bitset);
        PIOJO_ASSERT(! piojo_bitset_set_p(0, &bitset));
        PIOJO_ASSERT(! piojo_bitset_set_p(1, &bitset));

        piojo_bitset_set(0, &bitset);
        PIOJO_ASSERT(piojo_bitset_set_p(0, &bitset));
        PIOJO_ASSERT(! piojo_bitset_set_p(1, &bitset));

        piojo_bitset_set(1, &bitset);
        PIOJO_ASSERT(piojo_bitset_set_p(0, &bitset));
        PIOJO_ASSERT(piojo_bitset_set_p(1, &bitset));
}

void test_set()
{
        piojo_bitset_t bitset;
        size_t i;

        piojo_bitset_init_m(32, &bitset);
        for (i = 0; i < 32; ++i){
                piojo_bitset_set(i, &bitset);
                PIOJO_ASSERT(piojo_bitset_set_p(i, &bitset));
        }
}

void test_unset()
{
        piojo_bitset_t bitset;
        size_t i;

        piojo_bitset_init_m(32, &bitset);
        for (i = 0; i < 32; ++i){
                piojo_bitset_set(i, &bitset);
                PIOJO_ASSERT(piojo_bitset_set_p(i, &bitset));

                piojo_bitset_unset(i, &bitset);
                PIOJO_ASSERT(! piojo_bitset_set_p(i, &bitset));
        }
        PIOJO_ASSERT(piojo_bitset_empty_p(&bitset));
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

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
