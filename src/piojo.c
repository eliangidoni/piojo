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
 */

/**
 * @file
 * @addtogroup piojo Piojo
 * @{
 * Piojo useful types/functions.
 */

#include <piojo/piojo.h>
#include <piojo_defs.h>

/**
 * Checks addition overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_adduint_p(unsigned int v1, unsigned int v2)
{
        return (! (UINT_MAX - v1 < v2));
}

/**
 * Checks addition overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_addsiz_p(size_t v1, size_t v2)
{
        return (! (SIZE_MAX - v1 < v2));
}

/**
 * Checks addition overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_addint_p(int v1, int v2)
{
        return (! ((v2 > 0 && v1 > INT_MAX - v2) ||
                   (v2 < 0 && v1 < INT_MIN - v2)));
}

/**
 * Checks subtraction overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_subuint_p(unsigned int v1, unsigned int v2)
{
        return (! (v1 < v2));
}

/**
 * Checks subtraction overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_subsiz_p(size_t v1, size_t v2)
{
        return (! (v1 < v2));
}

/**
 * Checks subtraction overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_subint_p(int v1, int v2)
{
        return (! ((v2 > 0 && v1 < INT_MIN + v2) ||
                   (v2 < 0 && v1 > INT_MAX + v2)));
}

/**
 * Checks multiplication overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_muluint_p(unsigned int v1, unsigned int v2)
{
        return (! (v2 > 0 && v1 > UINT_MAX / v2));
}

/**
 * Checks multiplication overflow/wrap.
 * @param[in] v1
 * @param[in] v2
 * @return @b FALSE if overflows/wraps, @b TRUE otherwise.
 */
bool
piojo_safe_mulsiz_p(size_t v1, size_t v2)
{
        return (! (v2 > 0 && v1 > SIZE_MAX / v2));
}

/**
 * Returns maximum value.
 * @param[in] v1
 * @param[in] v2
 * @return Maximum value.
 */
size_t
piojo_maxsiz(size_t v1, size_t v2)
{
        return (v1 > v2 ? v1 : v2);
}

/**
 * Returns maximum value.
 * @param[in] v1
 * @param[in] v2
 * @return Maximum value.
 */
unsigned int
piojo_maxuint(unsigned int v1, unsigned int v2)
{
        return (v1 > v2 ? v1 : v2);
}

/**
 * Returns maximum value.
 * @param[in] v1
 * @param[in] v2
 * @return Maximum value.
 */
int
piojo_maxint(int v1, int v2)
{
        return (v1 > v2 ? v1 : v2);
}

/**
 * Returns minimum value.
 * @param[in] v1
 * @param[in] v2
 * @return Minimum value.
 */
size_t
piojo_minsiz(size_t v1, size_t v2)
{
        return (v1 < v2 ? v1 : v2);
}

/**
 * Returns minimum value.
 * @param[in] v1
 * @param[in] v2
 * @return Minimum value.
 */
unsigned int
piojo_minuint(unsigned int v1, unsigned int v2)
{
        return (v1 < v2 ? v1 : v2);
}

/**
 * Returns minimum value.
 * @param[in] v1
 * @param[in] v2
 * @return Minimum value.
 */
int
piojo_minint(int v1, int v2)
{
        return (v1 < v2 ? v1 : v2);
}

/** @} */