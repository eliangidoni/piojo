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
 * Piojo Disjoint-set API.
 */

/**
 * @file
 * @addtogroup piojodiset
 */

#ifndef PIOJO_DISET_H_
#define PIOJO_DISET_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_diset;
typedef struct piojo_diset piojo_diset_t;
extern const size_t piojo_diset_sizeof;

/** @{ */
/** Set id. */
typedef unsigned int piojo_diset_sid_t;
/** @} */

piojo_diset_t*
piojo_diset_alloc(void);

piojo_diset_t*
piojo_diset_alloc_cb(piojo_alloc_if allocator);

piojo_diset_t*
piojo_diset_copy(const piojo_diset_t *diset);

void
piojo_diset_free(const piojo_diset_t *diset);

void
piojo_diset_clear(piojo_diset_t *diset);

void
piojo_diset_insert(piojo_diset_sid_t set, piojo_diset_t *diset);

piojo_diset_sid_t
piojo_diset_find(piojo_diset_sid_t set, const piojo_diset_t *diset);

void
piojo_diset_union(piojo_diset_sid_t set1, piojo_diset_sid_t set2,
                  piojo_diset_t *diset);

#ifdef __cplusplus
}
#endif
#endif
