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
 * Piojo basic definitions.
 */

/**
 * @file
 * @addtogroup piojo Piojo
 */

#ifndef PIOJO_H_
#define PIOJO_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <piojo/piojo_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @{ */

/** @hideinitializer Equivalent to @b true. */
#define TRUE true
/** @hideinitializer Equivalent to @b false. */
#define FALSE false

/** Opaque data type. */
typedef uintptr_t piojo_opaque_t;

/**
 * Returns @b -1, @b 0, @b 1 when @a e1 is less than, equal to,
 * or greater than @a e2.
 */
typedef int
(*piojo_cmp_cb) (const void *e1, const void *e2);

/** Returns @b TRUE if @a e1 is less or equal to @a e2, @b FALSE otherwise. */
typedef bool
(*piojo_leq_cb) (const void *e1, const void *e2);

/** Returns @b TRUE if @a e1 is equal to @a e2, @b FALSE otherwise. */
typedef bool
(*piojo_eq_cb) (const void *e1, const void *e2);

/** Allocates @a size bytes of memory. */
typedef void*
(*piojo_alloc_cb) (size_t size);

/** Frees memory pointed by @a ptr. */
typedef void
(*piojo_free_cb) (const void *ptr);

/** Initializes @a esize bytes of memory pointed by @a ptr with @a data. */
typedef void
(*piojo_init_cb) (const void *data, size_t esize, void *ptr);

/** Copies @a esize bytes of memory pointed by @a ptr to @a newptr. */
typedef void
(*piojo_copy_cb) (const void *ptr, size_t esize, void *newptr);

/** Finishes memory pointed by @a ptr. */
typedef void
(*piojo_finish_cb) (void *ptr);

/** @} */

bool
piojo_safe_adduint_p(unsigned int v1, unsigned int v2);

bool
piojo_safe_addsiz_p(size_t v1, size_t v2);

bool
piojo_safe_addint_p(int v1, int v2);

bool
piojo_safe_subuint_p(unsigned int v1, unsigned int v2);

bool
piojo_safe_subsiz_p(size_t v1, size_t v2);

bool
piojo_safe_subint_p(int v1, int v2);

bool
piojo_safe_muluint_p(unsigned int v1, unsigned int v2);

bool
piojo_safe_mulsiz_p(size_t v1, size_t v2);

size_t
piojo_maxsiz(size_t v1, size_t v2);

unsigned int
piojo_maxuint(unsigned int v1, unsigned int v2);

int
piojo_maxint(int v1, int v2);

size_t
piojo_minsiz(size_t v1, size_t v2);

unsigned int
piojo_minuint(unsigned int v1, unsigned int v2);

int
piojo_minint(int v1, int v2);

size_t
piojo_clampsiz(size_t val, size_t start, size_t end);

unsigned int
piojo_clampuint(unsigned int val, unsigned int start, unsigned int end);

int
piojo_clampint(int val, int start, int end);

#ifdef __cplusplus
}
#endif

#endif
