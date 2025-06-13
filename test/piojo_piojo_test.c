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
#include <limits.h>

void test_opaque_eq(void)
{
        piojo_opaque_t v1 = 42;
        piojo_opaque_t v2 = 42;
        piojo_opaque_t v3 = 24;

        PIOJO_ASSERT(piojo_opaque_eq(&v1, &v2) == TRUE);
        PIOJO_ASSERT(piojo_opaque_eq(&v1, &v3) == FALSE);
        PIOJO_ASSERT(piojo_opaque_eq(&v2, &v3) == FALSE);
}

void test_id_eq(void)
{
        piojo_id_t id1 = 100;
        piojo_id_t id2 = 100;
        piojo_id_t id3 = 200;

        PIOJO_ASSERT(piojo_id_eq(&id1, &id2) == TRUE);
        PIOJO_ASSERT(piojo_id_eq(&id1, &id3) == FALSE);
        PIOJO_ASSERT(piojo_id_eq(&id2, &id3) == FALSE);
}

void test_safe_adduint_p(void)
{
        PIOJO_ASSERT(piojo_safe_adduint_p(10, 20) == TRUE);
        PIOJO_ASSERT(piojo_safe_adduint_p(0, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_adduint_p(UINT_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_adduint_p(0, UINT_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_adduint_p(UINT_MAX, 1) == FALSE);
        PIOJO_ASSERT(piojo_safe_adduint_p(UINT_MAX - 1, 2) == FALSE);
        PIOJO_ASSERT(piojo_safe_adduint_p(UINT_MAX / 2, UINT_MAX / 2 + 2) == FALSE);
}

void test_safe_addsiz_p(void)
{
        PIOJO_ASSERT(piojo_safe_addsiz_p(10, 20) == TRUE);
        PIOJO_ASSERT(piojo_safe_addsiz_p(0, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_addsiz_p(SIZE_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_addsiz_p(0, SIZE_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_addsiz_p(SIZE_MAX, 1) == FALSE);
        PIOJO_ASSERT(piojo_safe_addsiz_p(SIZE_MAX - 1, 2) == FALSE);
}

void test_safe_addint_p(void)
{
        PIOJO_ASSERT(piojo_safe_addint_p(10, 20) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(-10, 20) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(10, -20) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(-10, -20) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(0, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(INT_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(INT_MIN, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_addint_p(INT_MAX, 1) == FALSE);
        PIOJO_ASSERT(piojo_safe_addint_p(INT_MIN, -1) == FALSE);
        PIOJO_ASSERT(piojo_safe_addint_p(INT_MAX - 1, 2) == FALSE);
}

void test_safe_subuint_p(void)
{
        PIOJO_ASSERT(piojo_safe_subuint_p(20, 10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subuint_p(10, 10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subuint_p(UINT_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_subuint_p(UINT_MAX, UINT_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_subuint_p(10, 20) == FALSE);
        PIOJO_ASSERT(piojo_safe_subuint_p(0, 1) == FALSE);
}

void test_safe_subsiz_p(void)
{
        PIOJO_ASSERT(piojo_safe_subsiz_p(20, 10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subsiz_p(10, 10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subsiz_p(SIZE_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_subsiz_p(SIZE_MAX, SIZE_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_subsiz_p(10, 20) == FALSE);
        PIOJO_ASSERT(piojo_safe_subsiz_p(0, 1) == FALSE);
}

void test_safe_subint_p(void)
{
        PIOJO_ASSERT(piojo_safe_subint_p(20, 10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(-10, -20) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(10, -10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(-10, 10) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(0, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(INT_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(INT_MIN, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_subint_p(INT_MIN, 1) == FALSE);
        PIOJO_ASSERT(piojo_safe_subint_p(INT_MAX, -1) == FALSE);
}

void test_safe_muluint_p(void)
{
        PIOJO_ASSERT(piojo_safe_muluint_p(10, 20) == TRUE);
        PIOJO_ASSERT(piojo_safe_muluint_p(0, UINT_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_muluint_p(UINT_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_muluint_p(1, UINT_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_muluint_p(UINT_MAX, 1) == TRUE);
        PIOJO_ASSERT(piojo_safe_muluint_p(UINT_MAX, 2) == FALSE);
        PIOJO_ASSERT(piojo_safe_muluint_p(2, UINT_MAX) == FALSE);
        PIOJO_ASSERT(piojo_safe_muluint_p(65536, 65536) == FALSE);
}

void test_safe_mulsiz_p(void)
{
        PIOJO_ASSERT(piojo_safe_mulsiz_p(10, 20) == TRUE);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(0, SIZE_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(SIZE_MAX, 0) == TRUE);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(1, SIZE_MAX) == TRUE);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(SIZE_MAX, 1) == TRUE);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(SIZE_MAX, 2) == FALSE);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(2, SIZE_MAX) == FALSE);
}

void test_maxsiz(void)
{
        PIOJO_ASSERT(piojo_maxsiz(10, 20) == 20);
        PIOJO_ASSERT(piojo_maxsiz(20, 10) == 20);
        PIOJO_ASSERT(piojo_maxsiz(15, 15) == 15);
        PIOJO_ASSERT(piojo_maxsiz(0, SIZE_MAX) == SIZE_MAX);
        PIOJO_ASSERT(piojo_maxsiz(SIZE_MAX, 0) == SIZE_MAX);
}

void test_maxuint(void)
{
        PIOJO_ASSERT(piojo_maxuint(10, 20) == 20);
        PIOJO_ASSERT(piojo_maxuint(20, 10) == 20);
        PIOJO_ASSERT(piojo_maxuint(15, 15) == 15);
        PIOJO_ASSERT(piojo_maxuint(0, UINT_MAX) == UINT_MAX);
        PIOJO_ASSERT(piojo_maxuint(UINT_MAX, 0) == UINT_MAX);
}

void test_maxint(void)
{
        PIOJO_ASSERT(piojo_maxint(10, 20) == 20);
        PIOJO_ASSERT(piojo_maxint(20, 10) == 20);
        PIOJO_ASSERT(piojo_maxint(15, 15) == 15);
        PIOJO_ASSERT(piojo_maxint(-10, 5) == 5);
        PIOJO_ASSERT(piojo_maxint(-20, -10) == -10);
        PIOJO_ASSERT(piojo_maxint(INT_MIN, INT_MAX) == INT_MAX);
}

void test_minsiz(void)
{
        PIOJO_ASSERT(piojo_minsiz(10, 20) == 10);
        PIOJO_ASSERT(piojo_minsiz(20, 10) == 10);
        PIOJO_ASSERT(piojo_minsiz(15, 15) == 15);
        PIOJO_ASSERT(piojo_minsiz(0, SIZE_MAX) == 0);
        PIOJO_ASSERT(piojo_minsiz(SIZE_MAX, 0) == 0);
}

void test_minuint(void)
{
        PIOJO_ASSERT(piojo_minuint(10, 20) == 10);
        PIOJO_ASSERT(piojo_minuint(20, 10) == 10);
        PIOJO_ASSERT(piojo_minuint(15, 15) == 15);
        PIOJO_ASSERT(piojo_minuint(0, UINT_MAX) == 0);
        PIOJO_ASSERT(piojo_minuint(UINT_MAX, 0) == 0);
}

void test_minint(void)
{
        PIOJO_ASSERT(piojo_minint(10, 20) == 10);
        PIOJO_ASSERT(piojo_minint(20, 10) == 10);
        PIOJO_ASSERT(piojo_minint(15, 15) == 15);
        PIOJO_ASSERT(piojo_minint(-10, 5) == -10);
        PIOJO_ASSERT(piojo_minint(-20, -10) == -20);
        PIOJO_ASSERT(piojo_minint(INT_MIN, INT_MAX) == INT_MIN);
}

void test_clampsiz(void)
{
        PIOJO_ASSERT(piojo_clampsiz(15, 10, 20) == 15);
        PIOJO_ASSERT(piojo_clampsiz(5, 10, 20) == 10);
        PIOJO_ASSERT(piojo_clampsiz(25, 10, 20) == 20);
        PIOJO_ASSERT(piojo_clampsiz(10, 10, 20) == 10);
        PIOJO_ASSERT(piojo_clampsiz(20, 10, 20) == 20);
        PIOJO_ASSERT(piojo_clampsiz(15, 15, 15) == 15);
}

void test_clampuint(void)
{
        PIOJO_ASSERT(piojo_clampuint(15, 10, 20) == 15);
        PIOJO_ASSERT(piojo_clampuint(5, 10, 20) == 10);
        PIOJO_ASSERT(piojo_clampuint(25, 10, 20) == 20);
        PIOJO_ASSERT(piojo_clampuint(10, 10, 20) == 10);
        PIOJO_ASSERT(piojo_clampuint(20, 10, 20) == 20);
        PIOJO_ASSERT(piojo_clampuint(15, 15, 15) == 15);
}

void test_clampint(void)
{
        PIOJO_ASSERT(piojo_clampint(15, 10, 20) == 15);
        PIOJO_ASSERT(piojo_clampint(5, 10, 20) == 10);
        PIOJO_ASSERT(piojo_clampint(25, 10, 20) == 20);
        PIOJO_ASSERT(piojo_clampint(10, 10, 20) == 10);
        PIOJO_ASSERT(piojo_clampint(20, 10, 20) == 20);
        PIOJO_ASSERT(piojo_clampint(-5, -10, 0) == -5);
        PIOJO_ASSERT(piojo_clampint(-15, -10, 0) == -10);
        PIOJO_ASSERT(piojo_clampint(5, -10, 0) == 0);
}

void test_piojo(void)
{
        test_opaque_eq();
        test_id_eq();
        test_safe_adduint_p();
        test_safe_addsiz_p();
        test_safe_addint_p();
        test_safe_subuint_p();
        test_safe_subsiz_p();
        test_safe_subint_p();
        test_safe_muluint_p();
        test_safe_mulsiz_p();
        test_maxsiz();
        test_maxuint();
        test_maxint();
        test_minsiz();
        test_minuint();
        test_minint();
        test_clampsiz();
        test_clampuint();
        test_clampint();
}

int main(void)
{
        test_piojo();
        return 0;
}
