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
#include <piojo/piojo_diset.h>

void test_alloc(void)
{
        piojo_diset_t *diset;

        diset = piojo_diset_alloc();
        PIOJO_ASSERT(diset);
        piojo_diset_free(diset);

        diset = piojo_diset_alloc_cb(piojo_alloc_default);
        PIOJO_ASSERT(diset);
        piojo_diset_free(diset);
}

void test_copy_def(void)
{
        piojo_diset_t *diset, *copy;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(piojo_alloc_default);
        piojo_diset_insert(i, diset);

        copy = piojo_diset_copy(diset);
        PIOJO_ASSERT(copy);

        piojo_diset_free(diset);
        piojo_diset_free(copy);
}

void test_copy(void)
{
        piojo_diset_t *diset, *copy;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(my_allocator);
        piojo_diset_insert(i, diset);

        copy = piojo_diset_copy(diset);
        PIOJO_ASSERT(copy);

        piojo_diset_free(diset);
        piojo_diset_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free(void)
{
        piojo_diset_t *diset;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(my_allocator);
        piojo_diset_insert(i, diset);

        piojo_diset_free(diset);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear(void)
{
        piojo_diset_t *diset;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(my_allocator);
        piojo_diset_insert(i, diset);

        piojo_diset_clear(diset);
        assert_allocator_init(0);

        piojo_diset_free(diset);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_insert(void)
{
        piojo_diset_t *diset;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(my_allocator);
        piojo_diset_insert(i, diset);
        piojo_diset_insert(i+1, diset);
        piojo_diset_insert(i+2, diset);

        piojo_diset_clear(diset);
        assert_allocator_init(0);

        piojo_diset_free(diset);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_find(void)
{
        piojo_diset_t *diset, *copy;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(my_allocator);
        piojo_diset_insert(i, diset);
        piojo_diset_insert(i+1, diset);
        piojo_diset_insert(i+2, diset);

        copy = piojo_diset_copy(diset);
        PIOJO_ASSERT(copy);

        PIOJO_ASSERT(piojo_diset_find(i, diset) == i);
        PIOJO_ASSERT(piojo_diset_find(i, copy) == i);
        PIOJO_ASSERT(piojo_diset_find(i+1, diset) == i+1);
        PIOJO_ASSERT(piojo_diset_find(i+1, copy) == i+1);
        PIOJO_ASSERT(piojo_diset_find(i+2, diset) == i+2);
        PIOJO_ASSERT(piojo_diset_find(i+2, copy) == i+2);

        piojo_diset_free(diset);
        piojo_diset_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_union(void)
{
        piojo_diset_t *diset, *copy;
        unsigned int i=10;

        diset = piojo_diset_alloc_cb(my_allocator);
        piojo_diset_insert(i, diset);
        piojo_diset_insert(i+1, diset);
        piojo_diset_insert(i+2, diset);
        piojo_diset_insert(i+3, diset);
        piojo_diset_insert(i+4, diset);
        piojo_diset_insert(i+5, diset);

        piojo_diset_union(i, i+1, diset);
        PIOJO_ASSERT(piojo_diset_find(i+1, diset) == piojo_diset_find(i, diset));
        PIOJO_ASSERT(piojo_diset_find(i+2, diset) != piojo_diset_find(i, diset));

        piojo_diset_union(i+1, i+2, diset);
        PIOJO_ASSERT(piojo_diset_find(i+1, diset) == piojo_diset_find(i, diset));
        PIOJO_ASSERT(piojo_diset_find(i+2, diset) == piojo_diset_find(i, diset));

        piojo_diset_union(i, i+2, diset);
        PIOJO_ASSERT(piojo_diset_find(i+1, diset) == piojo_diset_find(i, diset));
        PIOJO_ASSERT(piojo_diset_find(i+2, diset) == piojo_diset_find(i, diset));

        piojo_diset_union(i+3, i+4, diset);
        PIOJO_ASSERT(piojo_diset_find(i+3, diset) == piojo_diset_find(i+4, diset));
        PIOJO_ASSERT(piojo_diset_find(i+3, diset) != piojo_diset_find(i, diset));
        PIOJO_ASSERT(piojo_diset_find(i+4, diset) != piojo_diset_find(i, diset));

        PIOJO_ASSERT(piojo_diset_find(i+1, diset) != piojo_diset_find(i+5, diset));
        PIOJO_ASSERT(piojo_diset_find(i+2, diset) != piojo_diset_find(i+5, diset));
        PIOJO_ASSERT(piojo_diset_find(i+3, diset) != piojo_diset_find(i+5, diset));
        PIOJO_ASSERT(piojo_diset_find(i+4, diset) != piojo_diset_find(i+5, diset));
        PIOJO_ASSERT(piojo_diset_find(i+5, diset) == piojo_diset_find(i+5, diset));

        copy = piojo_diset_copy(diset);
        PIOJO_ASSERT(copy);

        PIOJO_ASSERT(piojo_diset_find(i+1, diset) == piojo_diset_find(i, diset));
        PIOJO_ASSERT(piojo_diset_find(i+2, diset) == piojo_diset_find(i, diset));

        PIOJO_ASSERT(piojo_diset_find(i+3, copy) == piojo_diset_find(i+4, copy));
        PIOJO_ASSERT(piojo_diset_find(i+3, copy) != piojo_diset_find(i, copy));
        PIOJO_ASSERT(piojo_diset_find(i+4, copy) != piojo_diset_find(i, copy));

        PIOJO_ASSERT(piojo_diset_find(i+1, copy) != piojo_diset_find(i+5, copy));
        PIOJO_ASSERT(piojo_diset_find(i+2, copy) != piojo_diset_find(i+5, copy));
        PIOJO_ASSERT(piojo_diset_find(i+3, copy) != piojo_diset_find(i+5, copy));
        PIOJO_ASSERT(piojo_diset_find(i+4, copy) != piojo_diset_find(i+5, copy));
        PIOJO_ASSERT(piojo_diset_find(i+5, copy) == piojo_diset_find(i+5, copy));

        piojo_diset_free(diset);
        piojo_diset_free(copy);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

int main(void)
{
        test_alloc();
        test_copy();
        test_copy_def();
        test_free();
        test_clear();
        test_insert();
        test_find();
        test_union();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
