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
 * Piojo Bitset API.
 */

#ifndef PIOJO_BITSET_H_
#define PIOJO_BITSET_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_bitset_t piojo_bitset_t;
extern const size_t piojo_bitset_sizeof;

piojo_bitset_t*
piojo_bitset_alloc(size_t maxbits);

piojo_bitset_t*
piojo_bitset_alloc_cb(size_t maxbits, piojo_alloc_if allocator);

piojo_bitset_t*
piojo_bitset_copy(const piojo_bitset_t *bitset);

void
piojo_bitset_free(const piojo_bitset_t *bitset);

void
piojo_bitset_clear(piojo_bitset_t *bitset);

size_t
piojo_bitset_size(const piojo_bitset_t *bitset);

size_t
piojo_bitset_count(const piojo_bitset_t *bitset);

bool
piojo_bitset_empty_p(const piojo_bitset_t *bitset);

bool
piojo_bitset_full_p(const piojo_bitset_t *bitset);

bool
piojo_bitset_equal_p(const piojo_bitset_t *b1, const piojo_bitset_t *b2);

bool
piojo_bitset_set_p(size_t bit, const piojo_bitset_t *bitset);

void
piojo_bitset_set(size_t bit, piojo_bitset_t *bitset);

void
piojo_bitset_toggle(size_t bit, piojo_bitset_t *bitset);

void
piojo_bitset_unset(size_t bit, piojo_bitset_t *bitset);

void
piojo_bitset_not(const piojo_bitset_t *bitset, piojo_bitset_t *bout);

void
piojo_bitset_or(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                piojo_bitset_t *bout);

void
piojo_bitset_xor(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                 piojo_bitset_t *bout);

void
piojo_bitset_and(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                 piojo_bitset_t *bout);
void
piojo_bitset_diff(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                  piojo_bitset_t *bout);

void
piojo_bitset_lshift(size_t count, const piojo_bitset_t *bitset,
                    piojo_bitset_t *bout);

void
piojo_bitset_rshift(size_t count, const piojo_bitset_t *bitset,
                    piojo_bitset_t *bout);

#ifdef __cplusplus
}
#endif
#endif
