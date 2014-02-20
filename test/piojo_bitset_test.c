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

void test_size()
{
        piojo_bitset_t *bitset;

        bitset = piojo_bitset_alloc(2);
        PIOJO_ASSERT(piojo_bitset_size(bitset) == 2);

        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_set(0, bitset);
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_set(1, bitset);
        PIOJO_ASSERT(piojo_bitset_full_p(bitset));

        PIOJO_ASSERT(piojo_bitset_size(bitset) == 2);
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

void test_count()
{
        piojo_bitset_t *bitset;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        for (i = 0; i < 132; ++i){
                PIOJO_ASSERT(piojo_bitset_count(bitset) == i);
                piojo_bitset_set(i, bitset);
                PIOJO_ASSERT(piojo_bitset_count(bitset) == i+1);
        }
        PIOJO_ASSERT(! piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_full_p(bitset));

        piojo_bitset_unset(i-1, bitset);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == i-1);

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_toggle()
{
        piojo_bitset_t *bitset;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        for (i = 0; i < 132; ++i){
                PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                piojo_bitset_toggle(i, bitset);
                PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));

                piojo_bitset_toggle(i, bitset);
                PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
        }
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_union()
{
        piojo_bitset_t *bitset,*bout;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_set(120, bitset);
        piojo_bitset_set(122, bitset);
        piojo_bitset_set(0, bout);
        piojo_bitset_set(120, bout);
        piojo_bitset_set(121, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 2);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 3);

        piojo_bitset_or(bitset, bout, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 2);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 4);
        for (i = 0; i < 132; ++i){
                if (i == 0){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }else if (i == 120){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else if (i == 121){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }else if (i == 122){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_inter()
{
        piojo_bitset_t *bitset,*bout;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_set(120, bitset);
        piojo_bitset_set(0, bout);
        piojo_bitset_set(120, bout);
        piojo_bitset_set(121, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 1);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 3);

        piojo_bitset_and(bitset, bout, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 1);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 1);
        for (i = 0; i < 132; ++i){
                if (i == 120){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_complement()
{
        piojo_bitset_t *bitset,*bout;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_not(bitset, bout);
        PIOJO_ASSERT(piojo_bitset_full_p(bout));
        PIOJO_ASSERT(! piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_diff()
{
        piojo_bitset_t *bitset,*bout;
        size_t i;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_set(120, bitset);
        piojo_bitset_set(122, bitset);
        piojo_bitset_set(0, bout);
        piojo_bitset_set(120, bout);
        piojo_bitset_set(121, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 2);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 3);

        piojo_bitset_diff(bitset, bout, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 2);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 1);
        for (i = 0; i < 132; ++i){
                if (i == 0){
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }else if (i == 120){
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else if (i == 121){
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }else if (i == 122){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_eq()
{
        piojo_bitset_t *bitset,*bout;

        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        PIOJO_ASSERT(piojo_bitset_equal_p(bitset,bout));
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_set(120, bitset);
        piojo_bitset_set(122, bitset);
        piojo_bitset_set(0, bout);
        piojo_bitset_set(120, bout);
        piojo_bitset_set(121, bout);
        PIOJO_ASSERT(piojo_bitset_count(bitset) == 2);
        PIOJO_ASSERT(piojo_bitset_count(bout) == 3);

        PIOJO_ASSERT(! piojo_bitset_equal_p(bitset,bout));
        piojo_bitset_unset(0, bout);
        piojo_bitset_unset(121, bout);
        piojo_bitset_set(122, bout);
        PIOJO_ASSERT(piojo_bitset_equal_p(bitset,bout));

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_lshift()
{
        piojo_bitset_t *bitset,*bout;
        size_t i;
        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        piojo_bitset_set(62, bitset);
        piojo_bitset_set(63, bitset);
        piojo_bitset_set(64, bitset);
        piojo_bitset_set(65, bitset);
        piojo_bitset_set(66, bitset);

        piojo_bitset_lshift(2, bitset, bout);
        for (i = 0; i < 132; ++i){
                if (i >= 64 && i <= 68){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                }
                if (i >= 62 && i <= 66){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_lshift(132, bitset, bout);
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_lshift(0, bitset, bout);
        PIOJO_ASSERT(piojo_bitset_equal_p(bitset, bout));

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_rshift()
{
        piojo_bitset_t *bitset,*bout;
        size_t i;
        bitset = piojo_bitset_alloc_cb(132, my_allocator);
        bout = piojo_bitset_alloc_cb(132, my_allocator);

        piojo_bitset_set(62, bitset);
        piojo_bitset_set(63, bitset);
        piojo_bitset_set(64, bitset);
        piojo_bitset_set(65, bitset);
        piojo_bitset_set(66, bitset);

        piojo_bitset_rshift(2, bitset, bout);
        for (i = 0; i < 132; ++i){
                if (i >= 60 && i <= 64){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bout));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bout));
                }
                if (i >= 62 && i <= 66){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_rshift(132, bitset, bout);
        PIOJO_ASSERT(piojo_bitset_empty_p(bout));
        PIOJO_ASSERT(! piojo_bitset_full_p(bout));

        piojo_bitset_rshift(0, bitset, bout);
        PIOJO_ASSERT(piojo_bitset_equal_p(bitset, bout));

        piojo_bitset_free(bitset);
        piojo_bitset_free(bout);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_lshift_2()
{
        piojo_bitset_t *bitset;
        size_t i;
        bitset = piojo_bitset_alloc_cb(132, my_allocator);

        piojo_bitset_set(62, bitset);
        piojo_bitset_set(63, bitset);
        piojo_bitset_set(64, bitset);
        piojo_bitset_set(65, bitset);
        piojo_bitset_set(66, bitset);

        piojo_bitset_lshift(2, bitset, bitset);
        for (i = 0; i < 132; ++i){
                if (i >= 64 && i <= 68){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_lshift(132, bitset, bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_lshift(0, bitset, bitset);
        PIOJO_ASSERT(piojo_bitset_equal_p(bitset, bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_rshift_2()
{
        piojo_bitset_t *bitset;
        size_t i;
        bitset = piojo_bitset_alloc_cb(132, my_allocator);

        piojo_bitset_set(62, bitset);
        piojo_bitset_set(63, bitset);
        piojo_bitset_set(64, bitset);
        piojo_bitset_set(65, bitset);
        piojo_bitset_set(66, bitset);

        piojo_bitset_rshift(2, bitset, bitset);
        for (i = 0; i < 132; ++i){
                if (i >= 60 && i <= 64){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_rshift(132, bitset, bitset);
        PIOJO_ASSERT(piojo_bitset_empty_p(bitset));
        PIOJO_ASSERT(! piojo_bitset_full_p(bitset));

        piojo_bitset_rshift(0, bitset, bitset);
        PIOJO_ASSERT(piojo_bitset_equal_p(bitset, bitset));

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_lshift_3()
{
        piojo_bitset_t *bitset;
        size_t i;
        bitset = piojo_bitset_alloc_cb(5, my_allocator);

        piojo_bitset_set(0, bitset);
        piojo_bitset_set(1, bitset);
        piojo_bitset_set(2, bitset);

        piojo_bitset_lshift(2, bitset, bitset);
        for (i = 0; i < 5; ++i){
                if (i >= 2 && i <= 4){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

void test_rshift_3()
{
        piojo_bitset_t *bitset;
        size_t i;
        bitset = piojo_bitset_alloc_cb(5, my_allocator);

        piojo_bitset_set(0, bitset);
        piojo_bitset_set(1, bitset);
        piojo_bitset_set(2, bitset);

        piojo_bitset_rshift(2, bitset, bitset);
        for (i = 0; i < 5; ++i){
                if (i == 0){
                        PIOJO_ASSERT(piojo_bitset_set_p(i, bitset));
                }else{
                        PIOJO_ASSERT(! piojo_bitset_set_p(i, bitset));
                }
        }

        piojo_bitset_free(bitset);
        assert_allocator_init(0);
        assert_allocator_alloc(0);
}

int main()
{
        test_init();
        test_clear();
        test_set();
        test_toggle();
        test_size();
        test_count();
        test_set_p();
        test_full_p();
        test_empty_p();
        test_unset();
        test_copy();
        test_union();
        test_diff();
        test_inter();
        test_complement();
        test_eq();
        test_lshift();
        test_rshift();
        test_lshift_2();
        test_rshift_2();
        test_lshift_3();
        test_rshift_3();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
