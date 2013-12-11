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
 * Piojo Bitmap API.
 */

#ifndef PIOJO_BITMAP_H_
#define PIOJO_BITMAP_H_

#include <piojo/piojo_common.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        uint8_t opaque[32]; /* Should be enough */
} piojo_bitmap_t;

void
piojo_bitmap_init(piojo_bitmap_t *bitmap);

void
piojo_bitmap_init_m(uint8_t maxbits, piojo_bitmap_t *bitmap);

void
piojo_bitmap_clear(piojo_bitmap_t *bitmap);

bool
piojo_bitmap_empty_p(const piojo_bitmap_t *bitmap);

bool
piojo_bitmap_full_p(const piojo_bitmap_t *bitmap);

bool
piojo_bitmap_set_p(uint32_t bit, const piojo_bitmap_t *bitmap);

void
piojo_bitmap_set(uint32_t bit, piojo_bitmap_t *bitmap);

void
piojo_bitmap_unset(uint32_t bit, piojo_bitmap_t *bitmap);

#ifdef __cplusplus
}
#endif
#endif
