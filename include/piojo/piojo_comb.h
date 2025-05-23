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
 * Piojo Combinatoric functions API.
 */

#ifndef PIOJO_COMB_H_
#define PIOJO_COMB_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>
#include <piojo/piojo_array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_comb_t piojo_comb_t;
extern const size_t piojo_comb_sizeof;

piojo_comb_t*
piojo_comb_alloc(size_t n, size_t r);

piojo_comb_t*
piojo_comb_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator);

piojo_comb_t*
piojo_comb_with_rep_alloc(size_t n, size_t r);

piojo_comb_t*
piojo_comb_with_rep_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator);

piojo_comb_t*
piojo_comb_perm_alloc(size_t n, size_t r);

piojo_comb_t*
piojo_comb_perm_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator);

piojo_comb_t*
piojo_comb_prod_alloc(size_t n, size_t r);

piojo_comb_t*
piojo_comb_prod_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator);

piojo_array_t*
piojo_comb_next(piojo_comb_t *comb);

void
piojo_comb_free(const piojo_comb_t *comb);

#ifdef __cplusplus
}
#endif
#endif
