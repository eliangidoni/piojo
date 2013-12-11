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
#include <piojo/piojo_bitmap.h>

void test_init()
{
        piojo_bitmap_t bitmap;

        piojo_bitmap_init(&bitmap);
        PIOJO_ASSERT(piojo_bitmap_empty_p(&bitmap));

        piojo_bitmap_init_m(3, &bitmap);
        PIOJO_ASSERT(piojo_bitmap_empty_p(&bitmap));
}

void test_clear()
{
        piojo_bitmap_t bitmap;

        piojo_bitmap_init(&bitmap);
        piojo_bitmap_set(2, &bitmap);
        piojo_bitmap_clear(&bitmap);
        PIOJO_ASSERT(piojo_bitmap_empty_p(&bitmap));
}

void test_empty_p()
{
        piojo_bitmap_t bitmap;

        piojo_bitmap_init(&bitmap);
        PIOJO_ASSERT(piojo_bitmap_empty_p(&bitmap));

        piojo_bitmap_set(2, &bitmap);
        PIOJO_ASSERT(! piojo_bitmap_empty_p(&bitmap));
}

void test_full_p()
{
        piojo_bitmap_t bitmap;

        piojo_bitmap_init_m(2, &bitmap);
        PIOJO_ASSERT(! piojo_bitmap_full_p(&bitmap));

        piojo_bitmap_set(0, &bitmap);
        PIOJO_ASSERT(! piojo_bitmap_full_p(&bitmap));

        piojo_bitmap_set(1, &bitmap);
        PIOJO_ASSERT(piojo_bitmap_full_p(&bitmap));
}

void test_set_p()
{
        piojo_bitmap_t bitmap;

        piojo_bitmap_init_m(2, &bitmap);
        PIOJO_ASSERT(! piojo_bitmap_set_p(0, &bitmap));
        PIOJO_ASSERT(! piojo_bitmap_set_p(1, &bitmap));

        piojo_bitmap_set(0, &bitmap);
        PIOJO_ASSERT(piojo_bitmap_set_p(0, &bitmap));
        PIOJO_ASSERT(! piojo_bitmap_set_p(1, &bitmap));

        piojo_bitmap_set(1, &bitmap);
        PIOJO_ASSERT(piojo_bitmap_set_p(0, &bitmap));
        PIOJO_ASSERT(piojo_bitmap_set_p(1, &bitmap));
}

void test_set()
{
        piojo_bitmap_t bitmap;
        size_t i;

        piojo_bitmap_init_m(32, &bitmap);
        for (i = 0; i < 32; ++i){
                piojo_bitmap_set(i, &bitmap);
                PIOJO_ASSERT(piojo_bitmap_set_p(i, &bitmap));
        }
}

void test_unset()
{
        piojo_bitmap_t bitmap;
        size_t i;

        piojo_bitmap_init_m(32, &bitmap);
        for (i = 0; i < 32; ++i){
                piojo_bitmap_set(i, &bitmap);
                PIOJO_ASSERT(piojo_bitmap_set_p(i, &bitmap));

                piojo_bitmap_unset(i, &bitmap);
                PIOJO_ASSERT(! piojo_bitmap_set_p(i, &bitmap));
        }
        PIOJO_ASSERT(piojo_bitmap_empty_p(&bitmap));
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
