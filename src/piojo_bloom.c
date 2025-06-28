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
 */

/**
 * @file
 * @addtogroup piojobloom Piojo Bloom Filter
 * @{
 * Piojo Bloom Filter implementation.
 */

#include <piojo/piojo_bloom.h>
#include <piojo/piojo_bitset.h>
#include <piojo_defs.h>

struct piojo_bloom_t {
        size_t eksize, hash_count;
        piojo_bitset_t *bits;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of bloom filter in bytes */
const size_t piojo_bloom_sizeof = sizeof(piojo_bloom_t);

static uint32_t
calc_hash(const unsigned char *buf, size_t len, uint32_t seed);

/**
 * Allocates a new bloom filter.
 * Uses default allocator and key size of @b int32_t.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_i32k(size_t capacity, float false_positive_rate)
{
        return piojo_bloom_alloc_cb_i32k(capacity, false_positive_rate,
                piojo_alloc_default);
}

/**
 * Allocates a new bloom filter.
 * Uses default allocator and key size of @b int64_t.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_i64k(size_t capacity, float false_positive_rate)
{
        return piojo_bloom_alloc_cb_i64k(capacity, false_positive_rate,
                piojo_alloc_default);
}

/**
 * Allocates a new bloom filter.
 * Uses default allocator and key size of @b size_t.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_sizk(size_t capacity, float false_positive_rate)
{
        return piojo_bloom_alloc_cb_sizk(capacity, false_positive_rate,
                piojo_alloc_default);
}

/**
 * Allocates a new bloom filter.
 * Uses key size of @b int32_t.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @param[in] allocator Allocator to be used.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_cb_i32k(size_t capacity, float false_positive_rate,
        piojo_alloc_if allocator)
{
        return piojo_bloom_alloc_cb_eq(capacity, false_positive_rate,
                sizeof(int32_t), allocator);
}

/**
 * Allocates a new bloom filter.
 * Uses key size of @b int64_t.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @param[in] allocator Allocator to be used.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_cb_i64k(size_t capacity, float false_positive_rate,
        piojo_alloc_if allocator)
{
        return piojo_bloom_alloc_cb_eq(capacity, false_positive_rate,
                sizeof(int64_t), allocator);
}

/**
 * Allocates a new bloom filter.
 * Uses key size of @b size_t.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @param[in] allocator Allocator to be used.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_cb_sizk(size_t capacity, float false_positive_rate,
        piojo_alloc_if allocator)
{
        return piojo_bloom_alloc_cb_eq(capacity, false_positive_rate,
                sizeof(size_t), allocator);
}

/**
 * Allocates a new bloom filter.
 * Uses default allocator.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @param[in] eksize Entry key size in bytes.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_eq(size_t capacity, float false_positive_rate, size_t eksize)
{
        return piojo_bloom_alloc_cb_eq(capacity, false_positive_rate, eksize,
                                      piojo_alloc_default);
}

/**
 * Allocates a new bloom filter.
 * @param[in] capacity Bloom filter item capacity.
 * @param[in] false_positive_rate False positive rate.
 * @param[in] eksize Entry key size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New bloom filter.
 */
piojo_bloom_t*
piojo_bloom_alloc_cb_eq(size_t capacity, float false_positive_rate,
        size_t eksize, piojo_alloc_if allocator)
{
        piojo_bloom_t *bloom;
        piojo_bitset_t *bits;
        PIOJO_ASSERT(capacity > 0 && false_positive_rate > 0.0f);

        uint32_t bitsize = (uint32_t) ceil(-(capacity * log(false_positive_rate) /
                                  (log(2.0) * log(2.0))));

        uint32_t hash_count = (uint32_t) ceil(bitsize / capacity * log(2.0));

        bloom = (piojo_bloom_t *) allocator.alloc_cb(sizeof(piojo_bloom_t));
        PIOJO_ASSERT(bloom);

        bits = piojo_bitset_alloc_cb(bitsize, allocator);
        PIOJO_ASSERT(bits);

        bloom->allocator = allocator;
        bloom->bits = bits;
        bloom->hash_count = hash_count;
        bloom->eksize = eksize;
        return bloom;
}

/**
 * Copies @a bloom and all its entries.
 * @param[in] bloom Bloom filter being copied.
 * @return New bloom.
 */
piojo_bloom_t*
piojo_bloom_copy(const piojo_bloom_t *bloom)
{
        piojo_bloom_t *newbloom;
        piojo_bitset_t *bits;
        PIOJO_ASSERT(bloom);

        newbloom = (piojo_bloom_t *) bloom->allocator.alloc_cb(sizeof(piojo_bloom_t));
        PIOJO_ASSERT(newbloom);

        bits = piojo_bitset_copy(bloom->bits);
        PIOJO_ASSERT(bits);

        newbloom->allocator = bloom->allocator;
        newbloom->bits = bits;
        newbloom->hash_count = bloom->hash_count;
        newbloom->eksize = bloom->eksize;
        return newbloom;
}

/**
 * Frees @a bloom and all its entries.
 * @param[in] bloom Bloom filter being freed.
 */
void
piojo_bloom_free(const piojo_bloom_t *bloom)
{
        PIOJO_ASSERT(bloom);
        piojo_bitset_free(bloom->bits);
        bloom->allocator.free_cb(bloom);
}

/**
 * Deletes all entries in @a bloom.
 * @param[out] bloom Bloom filter being cleared.
 */
void
piojo_bloom_clear(piojo_bloom_t *bloom)
{
        PIOJO_ASSERT(bloom);
        piojo_bitset_clear(bloom->bits);
}

/**
 * Inserts a new entry.
 * @param[in] key Entry key.
 * @param[out] bloom Bloom filter being modified.
 */
void
piojo_bloom_insert(const void *key, piojo_bloom_t *bloom)
{
        PIOJO_ASSERT(bloom);
        PIOJO_ASSERT(key);
        for (uint32_t i = 0; i < bloom->hash_count; i++) {
                size_t index = calc_hash((const unsigned char *)key,
                        bloom->eksize, i) % piojo_bitset_size(bloom->bits);
                piojo_bitset_set(index, bloom->bits);
        }
}

/**
 * Searches an entry by key.
 * @param[in] key Entry key.
 * @param[in] bloom Bloom filter.
 * @return TRUE if @a key is (likely) in @a bloom, false otherwise.
 */
bool
piojo_bloom_search(const void *key, const piojo_bloom_t *bloom)
{
        PIOJO_ASSERT(bloom);
        PIOJO_ASSERT(key);
        for (uint32_t i = 0; i < bloom->hash_count; i++) {
                size_t index = calc_hash((const unsigned char *)key,
                        bloom->eksize, i) % piojo_bitset_size(bloom->bits);
                if (!piojo_bitset_set_p(index, bloom->bits)) {
                        return FALSE;
                }
        }
        return TRUE;
}

/** @}
 * Private functions.
 */

/* 32 bit Fowler/Noll/Vo FNV-1a hash on a buffer. */
static uint32_t
calc_hash(const unsigned char *buf, size_t len, uint32_t seed)
{
        uint32_t hval = (uint32_t)0x811c9dc5 + seed;
        unsigned char *bp = (unsigned char *)buf;
        unsigned char *be = bp + len;
        while (bp < be) {
                /* xor the bottom with the current octet */
                hval ^= (uint32_t)*bp++;
                /* multiply by the 32 bit FNV magic prime mod 2^32 */
                hval *= (uint32_t)0x01000193;
        }
        return hval;
}
