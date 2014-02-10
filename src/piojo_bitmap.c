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
 * @addtogroup piojobitmap Piojo Bitmap
 * @{
 * Piojo Bitmap implementation.
 */

#include <piojo/piojo_bitmap.h>
#include <piojo_defs.h>

typedef struct {
        uint64_t flags, mask;
        uint8_t maxbits;
} piojo_bitmap_priv_t;
/** @hideinitializer Size of bitmap in bytes */
const size_t piojo_bitmap_sizeof = sizeof(piojo_bitmap_t);

static const uint8_t BITMAP_MAXBITS = 64;

/**
 * Initializes a bitmap of @b 64 bits.
 * @param[out] bitmap User allocated bitmap.
 */
void
piojo_bitmap_init(piojo_bitmap_t *bitmap)
{
        piojo_bitmap_init_m(BITMAP_MAXBITS, bitmap);
}

/**
 * Initializes a bitmap of @a maxbits bits.
 * @param[in] maxbits Maximum bits (from 1 to 64).
 * @param[out] bitmap User allocated bitmap.
 */
void
piojo_bitmap_init_m(uint8_t maxbits, piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *bit;
        PIOJO_ASSERT(sizeof(piojo_bitmap_t) >= sizeof(piojo_bitmap_priv_t));
        PIOJO_ASSERT(bitmap);
        PIOJO_ASSERT(maxbits > 0 && maxbits <= BITMAP_MAXBITS);

        bit = (piojo_bitmap_priv_t*) bitmap->opaque;
        bit->maxbits = maxbits;
        bit->flags = 0;
        bit->mask = (uint64_t) pow(2, maxbits) - (uint64_t) 1;
}

/**
 * Unsets every bit in a bitmap.
 * @param[out] bitmap Bitmap being modified.
 */
void
piojo_bitmap_clear(piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *bit;
        PIOJO_ASSERT(bitmap);

        bit = (piojo_bitmap_priv_t*) bitmap->opaque;
        bit->flags = 0;
}

/**
 * Returns whether @a bitmap is empty (all bits unset).
 * @param[in] bitmap
 * @return @b TRUE if empty, @b FALSE otherwise.
 */
bool
piojo_bitmap_empty_p(const piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *bit;
        PIOJO_ASSERT(bitmap);

        bit = (piojo_bitmap_priv_t*) bitmap->opaque;
        return (bit->flags == (uint64_t) 0);
}

/**
 * Returns whether @a bitmap is full (all bits set).
 * @param[in] bitmap
 * @return @b TRUE if full, @b FALSE otherwise.
 */
bool
piojo_bitmap_full_p(const piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *bit;
        PIOJO_ASSERT(bitmap);

        bit = (piojo_bitmap_priv_t*) bitmap->opaque;
        return (bit->flags == bit->mask);
}

/**
 * Returns whether @a bit is set.
 * @param[in] bit Bit in bitmap (should be between 0 and @a maxbits).
 * @param[in] bitmap
 * @return @b TRUE if set, @b FALSE otherwise.
 */
bool
piojo_bitmap_set_p(uint32_t bit, const piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *b;
        PIOJO_ASSERT(bitmap);

        b = (piojo_bitmap_priv_t*) bitmap->opaque;
        PIOJO_ASSERT(bit < b->maxbits);

        return ((b->flags & ((uint64_t) 1 << bit)) > 0);
}

/**
 * Sets @a bit in bitmap
 * @param[in] bit Bit in bitmap (should be between 0 and @a maxbits).
 * @param[out] bitmap
 */
void
piojo_bitmap_set(uint32_t bit, piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *b;
        PIOJO_ASSERT(bitmap);

        b = (piojo_bitmap_priv_t*) bitmap->opaque;
        PIOJO_ASSERT(bit < b->maxbits);

        b->flags |= ((uint64_t) 1 << bit);
}

/**
 * Unsets @a bit in bitmap
 * @param[in] bit Bit in bitmap (should be between 0 and @a maxbits).
 * @param[out] bitmap
 */
void
piojo_bitmap_unset(uint32_t bit, piojo_bitmap_t *bitmap)
{
        piojo_bitmap_priv_t *b;
        PIOJO_ASSERT(bitmap);

        b = (piojo_bitmap_priv_t*) bitmap->opaque;
        PIOJO_ASSERT(bit < b->maxbits);

        b->flags &= (b->mask ^ ((uint64_t) 1 << bit));
}

/** @} */
