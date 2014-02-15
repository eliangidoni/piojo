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
 * @addtogroup piojobitset Piojo Bitset
 * @{
 * Piojo Bitset implementation.
 */

#include <piojo/piojo_bitset.h>
#include <piojo_defs.h>

typedef struct {
        uint64_t flags, mask;
        uint8_t maxbits;
} piojo_bitset_priv_t;
/** @hideinitializer Size of bitset in bytes */
const size_t piojo_bitset_sizeof = sizeof(piojo_bitset_t);

static const uint8_t BITSET_MAXBITS = 64;

/**
 * Initializes a bitset of @b 64 bits.
 * @param[out] bitset User allocated bitset.
 */
void
piojo_bitset_init(piojo_bitset_t *bitset)
{
        piojo_bitset_init_m(BITSET_MAXBITS, bitset);
}

/**
 * Initializes a bitset of @a maxbits bits.
 * @param[in] maxbits Maximum bits (from 1 to 64).
 * @param[out] bitset User allocated bitset.
 */
void
piojo_bitset_init_m(uint8_t maxbits, piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *bit;
        PIOJO_ASSERT(sizeof(piojo_bitset_t) >= sizeof(piojo_bitset_priv_t));
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(maxbits > 0 && maxbits <= BITSET_MAXBITS);

        bit = (piojo_bitset_priv_t*) bitset->opaque;
        bit->maxbits = maxbits;
        bit->flags = 0;
        bit->mask = (uint64_t) pow(2, maxbits) - (uint64_t) 1;
}

/**
 * Unsets every bit in a bitset.
 * @param[out] bitset Bitset being modified.
 */
void
piojo_bitset_clear(piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *bit;
        PIOJO_ASSERT(bitset);

        bit = (piojo_bitset_priv_t*) bitset->opaque;
        bit->flags = 0;
}

/**
 * Returns whether @a bitset is empty (all bits unset).
 * @param[in] bitset
 * @return @b TRUE if empty, @b FALSE otherwise.
 */
bool
piojo_bitset_empty_p(const piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *bit;
        PIOJO_ASSERT(bitset);

        bit = (piojo_bitset_priv_t*) bitset->opaque;
        return (bit->flags == (uint64_t) 0);
}

/**
 * Returns whether @a bitset is full (all bits set).
 * @param[in] bitset
 * @return @b TRUE if full, @b FALSE otherwise.
 */
bool
piojo_bitset_full_p(const piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *bit;
        PIOJO_ASSERT(bitset);

        bit = (piojo_bitset_priv_t*) bitset->opaque;
        return (bit->flags == bit->mask);
}

/**
 * Returns whether @a bit is set.
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits).
 * @param[in] bitset
 * @return @b TRUE if set, @b FALSE otherwise.
 */
bool
piojo_bitset_set_p(uint32_t bit, const piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *b;
        PIOJO_ASSERT(bitset);

        b = (piojo_bitset_priv_t*) bitset->opaque;
        PIOJO_ASSERT(bit < b->maxbits);

        return ((b->flags & ((uint64_t) 1 << bit)) > 0);
}

/**
 * Sets @a bit in bitset
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits).
 * @param[out] bitset
 */
void
piojo_bitset_set(uint32_t bit, piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *b;
        PIOJO_ASSERT(bitset);

        b = (piojo_bitset_priv_t*) bitset->opaque;
        PIOJO_ASSERT(bit < b->maxbits);

        b->flags |= ((uint64_t) 1 << bit);
}

/**
 * Unsets @a bit in bitset
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits).
 * @param[out] bitset
 */
void
piojo_bitset_unset(uint32_t bit, piojo_bitset_t *bitset)
{
        piojo_bitset_priv_t *b;
        PIOJO_ASSERT(bitset);

        b = (piojo_bitset_priv_t*) bitset->opaque;
        PIOJO_ASSERT(bit < b->maxbits);

        b->flags &= (b->mask ^ ((uint64_t) 1 << bit));
}

/** @} */
