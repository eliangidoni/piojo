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

/* Keep these in sync. */
typedef uint64_t piojo_bitset_word_t;
static const piojo_bitset_word_t BITSET_MASK = 0xffffffffffffffff;
static const size_t BITSET_BITS = 64;

struct piojo_bitset {
        piojo_bitset_word_t *set, lastmask;
        size_t maxbits, wcnt;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of bitset in bytes */
const size_t piojo_bitset_sizeof = sizeof(piojo_bitset_t);

static piojo_bitset_word_t
bit_mask(size_t n);

static size_t
popcount(uint64_t x);

/**
 * Allocates a new bitset of @a maxbits bits.
 * Uses default allocator.
 * @param[in] maxbits Maximum number of bits.
 * @return New bitset.
 */
piojo_bitset_t*
piojo_bitset_alloc(size_t maxbits)
{
        return piojo_bitset_alloc_cb(maxbits, piojo_alloc_default);
}

/**
 * Allocates a new bitset of @a maxbits bits.
 * @param[in] maxbits Maximum number of bits.
 * @param[in] allocator Allocator to be used.
 * @return New bitset.
 */
piojo_bitset_t*
piojo_bitset_alloc_cb(size_t maxbits, piojo_alloc_if allocator)
{
        piojo_bitset_t * b;
        size_t i, setsize;
        PIOJO_ASSERT(maxbits > 0);

        b = (piojo_bitset_t *) allocator.alloc_cb(sizeof(piojo_bitset_t));
        PIOJO_ASSERT(b);

        b->allocator = allocator;
        b->maxbits = maxbits;
        b->wcnt = maxbits / BITSET_BITS;
        b->lastmask = bit_mask(maxbits % BITSET_BITS);
        if (b->lastmask != 0){
                ++b->wcnt;
        }

        PIOJO_ASSERT(piojo_safe_mulsiz_p(sizeof(piojo_bitset_word_t), b->wcnt));
        setsize = sizeof(piojo_bitset_word_t) * b->wcnt;

        b->set = (piojo_bitset_word_t *)allocator.alloc_cb(setsize);
        PIOJO_ASSERT(b->set);
        for (i = 0; i < b->wcnt; ++i){
                b->set[i] = 0;
        }

        return b;
}

/**
 * Copies @a bitset.
 * @param[in] bitset Bitset being copied.
 * @return New bitset.
 */
piojo_bitset_t*
piojo_bitset_copy(const piojo_bitset_t *bitset)
{
        piojo_bitset_t * b;
        size_t i, setsize;
        PIOJO_ASSERT(bitset);

        b = (piojo_bitset_t*)bitset->allocator.alloc_cb(sizeof(piojo_bitset_t));
        PIOJO_ASSERT(b);

        b->allocator = bitset->allocator;
        b->maxbits = bitset->maxbits;
        b->wcnt = bitset->wcnt;
        b->lastmask = bitset->lastmask;

        setsize = sizeof(piojo_bitset_word_t) * b->wcnt;

        b->set = (piojo_bitset_word_t *)b->allocator.alloc_cb(setsize);
        PIOJO_ASSERT(b->set);
        for (i = 0; i < b->wcnt; ++i){
                b->set[i] = bitset->set[i];
        }

        return b;
}

/**
 * Unsets every bit in a bitset.
 * @param[out] bitset Bitset being modified.
 */
void
piojo_bitset_clear(piojo_bitset_t *bitset)
{
        size_t i;
        PIOJO_ASSERT(bitset);

        for (i = 0; i < bitset->wcnt; ++i){
                bitset->set[i] = 0;
        }
}

/**
 * Frees @a bitset.
 * @param[in] bitset Bitset being freed.
 */
void
piojo_bitset_free(const piojo_bitset_t *bitset)
{
        piojo_alloc_if ator;
        PIOJO_ASSERT(bitset);

        ator = bitset->allocator;
        ator.free_cb(bitset->set);
        ator.free_cb(bitset);
}

/**
 * Returns bitset size.
 * @param[in] bitset
 * @return @a maxbits.
 */
size_t
piojo_bitset_size(const piojo_bitset_t *bitset)
{
        PIOJO_ASSERT(bitset);

        return bitset->maxbits;
}

/**
 * Finds number of set bits (Hamming weight).
 * @param[in] bitset
 * @return Number of set bits.
 */
size_t
piojo_bitset_count(const piojo_bitset_t *bitset)
{
        size_t i, count=0;
        PIOJO_ASSERT(bitset);

        for (i = 0; i < bitset->wcnt; ++i){
                count += popcount(bitset->set[i]);
        }
        return count;
}

/**
 * Returns whether @a bitset is empty (all bits unset).
 * @param[in] bitset
 * @return @b TRUE if empty, @b FALSE otherwise.
 */
bool
piojo_bitset_empty_p(const piojo_bitset_t *bitset)
{
        size_t i;
        PIOJO_ASSERT(bitset);

        for (i = 0; i < bitset->wcnt; ++i){
                if (bitset->set[i] != 0){
                        return FALSE;
                }
        }
        return TRUE;
}

/**
 * Returns whether @a bitset is full (all bits set).
 * @param[in] bitset
 * @return @b TRUE if full, @b FALSE otherwise.
 */
bool
piojo_bitset_full_p(const piojo_bitset_t *bitset)
{
        size_t i;
        PIOJO_ASSERT(bitset);

        for (i = 0; i < bitset->wcnt - 1; ++i){
                if (bitset->set[i] != BITSET_MASK){
                        return FALSE;
                }
        }
        return (bitset->set[i] == bitset->lastmask);
}

/**
 * Returns whether two bitsets are equal.
 * @param[in] b1 bitset.
 * @param[in] b2 bitset.
 * @return @b TRUE if equal, @b FALSE otherwise.
 */
bool
piojo_bitset_equal_p(const piojo_bitset_t *b1, const piojo_bitset_t *b2)
{
        size_t i;
        PIOJO_ASSERT(b1);
        PIOJO_ASSERT(b2);
        PIOJO_ASSERT(b1->maxbits == b2->maxbits);

        if (b1 == b2){
                return TRUE;
        }

        for (i = 0; i < b1->wcnt; ++i){
                if (b1->set[i] != b2->set[i]){
                        return FALSE;
                }
        }
        return TRUE;
}

/**
 * Returns whether @a bit is set.
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits-1).
 * @param[in] bitset
 * @return @b TRUE if set, @b FALSE otherwise.
 */
bool
piojo_bitset_set_p(size_t bit, const piojo_bitset_t *bitset)
{
        size_t widx, bidx;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bit < bitset->maxbits);

        widx = bit / BITSET_BITS;
        bidx = bit % BITSET_BITS;
        return ((bitset->set[widx] & ((piojo_bitset_word_t) 1 << bidx)) != 0);
}

/**
 * Sets @a bit in bitset
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits-1).
 * @param[out] bitset
 */
void
piojo_bitset_set(size_t bit, piojo_bitset_t *bitset)
{
        size_t widx, bidx;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bit < bitset->maxbits);

        widx = bit / BITSET_BITS;
        bidx = bit % BITSET_BITS;
        bitset->set[widx] |= ((piojo_bitset_word_t) 1 << bidx);
}

/**
 * Toggles @a bit in bitset
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits-1).
 * @param[out] bitset
 */
void
piojo_bitset_toggle(size_t bit, piojo_bitset_t *bitset)
{
        size_t widx, bidx;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bit < bitset->maxbits);

        widx = bit / BITSET_BITS;
        bidx = bit % BITSET_BITS;
        bitset->set[widx] ^= ((piojo_bitset_word_t) 1 << bidx);
}

/**
 * Unsets @a bit in bitset
 * @param[in] bit Bit in bitset (should be between 0 and @a maxbits-1).
 * @param[out] bitset
 */
void
piojo_bitset_unset(size_t bit, piojo_bitset_t *bitset)
{
        size_t widx, bidx;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bit < bitset->maxbits);

        widx = bit / BITSET_BITS;
        bidx = bit % BITSET_BITS;
        bitset->set[widx] &= (BITSET_MASK ^ ((piojo_bitset_word_t) 1 << bidx));
}

/**
 * Calculates the complement of a bitset.
 * @param[in] bitset
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_not(const piojo_bitset_t *bitset, piojo_bitset_t *bout)
{
        size_t i;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(bitset->maxbits == bout->maxbits);

        for (i = 0; i < bout->wcnt - 1; ++i){
                bout->set[i] = ~ bitset->set[i];
        }
        bout->set[i] = (~ bitset->set[i]) & bitset->lastmask;
}

/**
 * Calculates the union of two bitsets.
 * @param[in] b1 bitset.
 * @param[in] b2 bitset.
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_or(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                piojo_bitset_t *bout)
{
        size_t i;
        PIOJO_ASSERT(b1);
        PIOJO_ASSERT(b2);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(b1->maxbits == bout->maxbits &&
                     b2->maxbits == bout->maxbits);

        for (i = 0; i < bout->wcnt - 1; ++i){
                bout->set[i] = b1->set[i] | b2->set[i];
        }
        bout->set[i] = (b1->set[i] | b2->set[i]) & bout->lastmask;
}

/**
 * Calculates the exclusive-or of two bitsets.
 * @param[in] b1 bitset.
 * @param[in] b2 bitset.
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_xor(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                piojo_bitset_t *bout)
{
        size_t i;
        PIOJO_ASSERT(b1);
        PIOJO_ASSERT(b2);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(b1->maxbits == bout->maxbits &&
                     b2->maxbits == bout->maxbits);

        for (i = 0; i < bout->wcnt - 1; ++i){
                bout->set[i] = b1->set[i] ^ b2->set[i];
        }
        bout->set[i] = (b1->set[i] ^ b2->set[i]) & bout->lastmask;
}

/**
 * Calculates the intersection of two bitsets.
 * @param[in] b1 bitset.
 * @param[in] b2 bitset.
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_and(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                 piojo_bitset_t *bout)
{
        size_t i;
        PIOJO_ASSERT(b1);
        PIOJO_ASSERT(b2);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(b1->maxbits == bout->maxbits &&
                     b2->maxbits == bout->maxbits);

        for (i = 0; i < bout->wcnt - 1; ++i){
                bout->set[i] = b1->set[i] & b2->set[i];
        }
        bout->set[i] = (b1->set[i] & b2->set[i]) & bout->lastmask;
}

/**
 * Calculates the difference of two bitsets.
 * @param[in] b1 bitset.
 * @param[in] b2 bitset.
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_diff(const piojo_bitset_t *b1, const piojo_bitset_t *b2,
                  piojo_bitset_t *bout)
{
        size_t i;
        PIOJO_ASSERT(b1);
        PIOJO_ASSERT(b2);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(b1->maxbits == bout->maxbits &&
                     b2->maxbits == bout->maxbits);

        for (i = 0; i < bout->wcnt - 1; ++i){
                bout->set[i] = b1->set[i] & (~ b2->set[i]);
        }
        bout->set[i] = (b1->set[i] & (~ b2->set[i])) & bout->lastmask;
}

/**
 * Shifts all bits to left @a count times. (unsigned shift)
 * @param[in] count
 * @param[in] bitset
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_lshift(size_t count, const piojo_bitset_t *bitset,
                    piojo_bitset_t *bout)
{
        size_t i;
        piojo_bitset_t *b2 = bout;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(bitset->maxbits == bout->maxbits);

        if (count == 0){
                if (bitset != bout){
                        for (i = 0; i < bout->wcnt; ++i){
                                bout->set[i] = bitset->set[i];
                        }
                }
                return;
        }

        if (bout->wcnt == 1){
                bout->set[0] = bitset->set[0] << count;
                return;
        }

        if (bitset == bout){
                b2 = piojo_bitset_alloc_cb(bout->maxbits, bout->allocator);
        }else{
                piojo_bitset_clear(b2);
        }

        i = bout->maxbits - 1;
        do {
                if (count > i){
                        break;
                }
                if (piojo_bitset_set_p(i - count, bitset)){
                        piojo_bitset_set(i, b2);
                }
        }while(i-- > 0);

        if (bitset == bout){
                for (i = 0; i < bout->wcnt; ++i){
                        bout->set[i] = b2->set[i];
                }
                piojo_bitset_free(b2);
        }
}

/**
 * Shifts all bits to right @a count times. (unsigned shift)
 * @param[in] count
 * @param[in] bitset
 * @param[out] bout Result bitset.
 */
void
piojo_bitset_rshift(size_t count, const piojo_bitset_t *bitset,
                    piojo_bitset_t *bout)
{
        size_t i, bidx;
        piojo_bitset_t *b2 = bout;
        PIOJO_ASSERT(bitset);
        PIOJO_ASSERT(bout);
        PIOJO_ASSERT(bitset->maxbits == bout->maxbits);

        if (count == 0){
                if (bitset != bout){
                        for (i = 0; i < bout->wcnt; ++i){
                                bout->set[i] = bitset->set[i];
                        }
                }
                return;
        }

        if (bout->wcnt == 1){
                bout->set[0] = bitset->set[0] >> count;
                return;
        }

        if (bitset == bout){
                b2 = piojo_bitset_alloc_cb(bout->maxbits, bout->allocator);
        }else{
                piojo_bitset_clear(b2);
        }

        for (i = 0; i < bout->maxbits; ++i){
                PIOJO_ASSERT(piojo_safe_addsiz_p(i, count));
                bidx = i + count;
                if (bidx >= bout->maxbits){
                        break;
                }
                if (piojo_bitset_set_p(bidx, bitset)){
                        piojo_bitset_set(i, b2);
                }
        }

        if (bitset == bout){
                for (i = 0; i < bout->wcnt; ++i){
                        bout->set[i] = b2->set[i];
                }
                piojo_bitset_free(b2);
        }
}

/** @}
 * Private functions.
 */

static piojo_bitset_word_t
bit_mask(size_t n)
{
        return (((piojo_bitset_word_t) 1 << n) - (piojo_bitset_word_t) 1);
}

/* Source: http://en.wikipedia.org/wiki/Hamming_weight */
static size_t
popcount(uint64_t x)
{
        /* put count of each 2 bits into those 2 bits */
        x -= (x >> 1) & 0x5555555555555555;

        /* put count of each 4 bits into those 4 bits */
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);

        /* put count of each 8 bits into those 8 bits */
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;

        /* returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... */
        return (x * 0x0101010101010101) >> 56;
}
