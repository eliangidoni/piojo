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
#include <piojo/piojo_comb.h>

void test_alloc(void)
{
        piojo_comb_t *comb;

        comb = piojo_comb_alloc(2, 2);
        PIOJO_ASSERT(comb);
        piojo_comb_free(comb);

        comb = piojo_comb_alloc_cb(2, 2, piojo_alloc_default);
        PIOJO_ASSERT(comb);
        piojo_comb_free(comb);
}

void test_free(void)
{
        piojo_comb_t *comb;

        comb = piojo_comb_alloc_cb(2, 2, my_allocator);
        piojo_comb_next(comb);

        piojo_comb_free(comb);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_prod_next(void)
{
        piojo_comb_t *comb;
        piojo_array_t *sample;
        const size_t n=3, r=3;
        size_t expected[27][3] = {
                {0, 0, 0},
                {0, 0, 1},
                {0, 0, 2},
                {0, 1, 0},
                {0, 1, 1},
                {0, 1, 2},
                {0, 2, 0},
                {0, 2, 1},
                {0, 2, 2},
                {1, 0, 0},
                {1, 0, 1},
                {1, 0, 2},
                {1, 1, 0},
                {1, 1, 1},
                {1, 1, 2},
                {1, 2, 0},
                {1, 2, 1},
                {1, 2, 2},
                {2, 0, 0},
                {2, 0, 1},
                {2, 0, 2},
                {2, 1, 0},
                {2, 1, 1},
                {2, 1, 2},
                {2, 2, 0},
                {2, 2, 1},
                {2, 2, 2}
        };
        comb = piojo_comb_prod_alloc_cb(n, r, my_allocator);
        sample = piojo_comb_next(comb);
        PIOJO_ASSERT(sample);
        for (size_t i = 0; i < 27; i++) {
                size_t *s = (size_t *) piojo_array_at(0, sample);
                PIOJO_ASSERT(s[0] == expected[i][0]);
                s = (size_t *) piojo_array_at(1, sample);
                PIOJO_ASSERT(s[0] == expected[i][1]);
                s = (size_t *) piojo_array_at(2, sample);
                PIOJO_ASSERT(s[0] == expected[i][2]);
                sample = piojo_comb_next(comb);
        }
        PIOJO_ASSERT(sample == NULL);

        piojo_comb_free(comb);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_comb_next(void)
{
        piojo_comb_t *comb;
        piojo_array_t *sample;
        const size_t n=5, r=3;
        size_t expected[10][3] = {
                {0, 1, 2},
                {0, 1, 3},
                {0, 1, 4},
                {0, 2, 3},
                {0, 2, 4},
                {0, 3, 4},
                {1, 2, 3},
                {1, 2, 4},
                {1, 3, 4},
                {2, 3, 4}
        };
        comb = piojo_comb_alloc_cb(n, r, my_allocator);
        sample = piojo_comb_next(comb);
        PIOJO_ASSERT(sample);
        for (size_t i = 0; i < 10; i++) {
                size_t *s = (size_t *) piojo_array_at(0, sample);
                PIOJO_ASSERT(s[0] == expected[i][0]);
                s = (size_t *) piojo_array_at(1, sample);
                PIOJO_ASSERT(s[0] == expected[i][1]);
                s = (size_t *) piojo_array_at(2, sample);
                PIOJO_ASSERT(s[0] == expected[i][2]);
                sample = piojo_comb_next(comb);
        }
        PIOJO_ASSERT(sample == NULL);

        piojo_comb_free(comb);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_comb_with_rep_next(void)
{
        piojo_comb_t *comb;
        piojo_array_t *sample;
        const size_t n=5, r=3;
        size_t expected[35][3] = {
                {0, 0, 0},
                {0, 0, 1},
                {0, 0, 2},
                {0, 0, 3},
                {0, 0, 4},
                {0, 1, 1},
                {0, 1, 2},
                {0, 1, 3},
                {0, 1, 4},
                {0, 2, 2},
                {0, 2, 3},
                {0, 2, 4},
                {0, 3, 3},
                {0, 3, 4},
                {0, 4, 4},
                {1, 1, 1},
                {1, 1, 2},
                {1, 1, 3},
                {1, 1, 4},
                {1, 2, 2},
                {1, 2, 3},
                {1, 2, 4},
                {1, 3, 3},
                {1, 3, 4},
                {1, 4, 4},
                {2, 2, 2},
                {2, 2, 3},
                {2, 2, 4},
                {2, 3, 3},
                {2, 3, 4},
                {2, 4, 4},
                {3, 3, 3},
                {3, 3, 4},
                {3, 4, 4},
                {4, 4, 4}
        };
        comb = piojo_comb_with_rep_alloc_cb(n, r, my_allocator);
        sample = piojo_comb_next(comb);
        PIOJO_ASSERT(sample);
        for (size_t i = 0; i < 35; i++) {
                size_t *s = (size_t *) piojo_array_at(0, sample);
                PIOJO_ASSERT(s[0] == expected[i][0]);
                s = (size_t *) piojo_array_at(1, sample);
                PIOJO_ASSERT(s[0] == expected[i][1]);
                s = (size_t *) piojo_array_at(2, sample);
                PIOJO_ASSERT(s[0] == expected[i][2]);
                sample = piojo_comb_next(comb);
        }
        PIOJO_ASSERT(sample == NULL);

        piojo_comb_free(comb);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_perm_next(void)
{
        piojo_comb_t *comb;
        piojo_array_t *sample;
        const size_t n=4, r=3;
        size_t expected[24][3] = {
                {0, 1, 2},
                {0, 1, 3},
                {0, 2, 1},
                {0, 2, 3},
                {0, 3, 1},
                {0, 3, 2},
                {1, 0, 2},
                {1, 0, 3},
                {1, 2, 0},
                {1, 2, 3},
                {1, 3, 0},
                {1, 3, 2},
                {2, 0, 1},
                {2, 0, 3},
                {2, 1, 0},
                {2, 1, 3},
                {2, 3, 0},
                {2, 3, 1},
                {3, 0, 1},
                {3, 0, 2},
                {3, 1, 0},
                {3, 1, 2},
                {3, 2, 0},
                {3, 2, 1}
        };
        comb = piojo_comb_perm_alloc_cb(n, r, my_allocator);
        sample = piojo_comb_next(comb);
        PIOJO_ASSERT(sample);
        for (size_t i = 0; i < 24; i++) {
                size_t *s = (size_t *) piojo_array_at(0, sample);
                PIOJO_ASSERT(s[0] == expected[i][0]);
                s = (size_t *) piojo_array_at(1, sample);
                PIOJO_ASSERT(s[0] == expected[i][1]);
                s = (size_t *) piojo_array_at(2, sample);
                PIOJO_ASSERT(s[0] == expected[i][2]);
                sample = piojo_comb_next(comb);
        }
        PIOJO_ASSERT(sample == NULL);

        piojo_comb_free(comb);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

int main(void)
{
        test_alloc();
        test_free();
        test_prod_next();
        test_comb_next();
        test_comb_with_rep_next();
        test_perm_next();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
