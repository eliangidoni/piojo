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
 * @addtogroup piojocomb Piojo Combinatoric functions
 * @{
 * Piojo Combinatoric functions.
 */

#include <piojo/piojo_comb.h>
#include <piojo_defs.h>

struct piojo_comb_t {
        piojo_array_t *sample, *cycles;
        size_t n, r, sample_idx;
        piojo_alloc_if allocator;
        bool first_p;
        piojo_array_t* (*next_cb)(piojo_comb_t *comb);
};
/** @hideinitializer Size of combination iterator in bytes */
const size_t piojo_comb_sizeof = sizeof(piojo_comb_t);

static piojo_comb_t*
comb_alloc(size_t n, size_t r, piojo_alloc_if allocator);

static piojo_array_t*
comb_next(piojo_comb_t *comb);

static piojo_array_t*
prod_next(piojo_comb_t *comb);

static piojo_array_t*
comb_with_rep_next(piojo_comb_t *comb);

static piojo_array_t*
perm_next(piojo_comb_t *comb);

/**
 * Allocates a combination iterator.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @return New combination iterator.
 */
piojo_comb_t*
piojo_comb_alloc(size_t n, size_t r)
{
        return piojo_comb_alloc_cb(n, r, piojo_alloc_default);
}

/**
 * Allocates a combination iterator.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @param[in] allocator Allocator to be used.
 * @return New combination iterator.
 */
piojo_comb_t*
piojo_comb_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator)
{
        PIOJO_ASSERT(r <= n);
        piojo_comb_t * c = comb_alloc(n, r, allocator);
        c->next_cb = comb_next;
        for (size_t i=0; i < r; i++) {
                piojo_array_push(&i, c->sample);
        }
        return c;
}

/**
 * Allocates a combination iterator with replacement.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @return New combination iterator.
 */
piojo_comb_t*
piojo_comb_with_rep_alloc(size_t n, size_t r)
{
        return piojo_comb_with_rep_alloc_cb(n, r, piojo_alloc_default);
}

/**
 * Allocates a combination iterator with replacement.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @param[in] allocator Allocator to be used.
 * @return New combination iterator.
 */
piojo_comb_t*
piojo_comb_with_rep_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator)
{
        piojo_comb_t * c = comb_alloc(n, r, allocator);
        c->next_cb = comb_with_rep_next;
        size_t zero = 0;
        for (size_t i=0; i < r; i++) {
                piojo_array_push(&zero, c->sample);
        }
        return c;
}

/**
 * Allocates a permutation iterator.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @return New permutation iterator.
 */
piojo_comb_t*
piojo_comb_perm_alloc(size_t n, size_t r)
{
        return piojo_comb_perm_alloc_cb(n, r, piojo_alloc_default);
}

/**
 * Allocates a permutation iterator.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @param[in] allocator Allocator to be used.
 * @return New permutation iterator.
 */
piojo_comb_t*
piojo_comb_perm_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator)
{
        PIOJO_ASSERT(r <= n);
        piojo_comb_t * c = comb_alloc(n, r, allocator);
        c->next_cb = perm_next;
        for (size_t i=0; i < n; i++) {
                piojo_array_push(&i, c->sample);
        }
        for (size_t i=n; i > n - r; i--) {
                piojo_array_push(&i, c->cycles);
        }
        return c;
}

/**
 * Allocates a product iterator.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @return New product iterator.
 */
piojo_comb_t*
piojo_comb_prod_alloc(size_t n, size_t r)
{
        return piojo_comb_prod_alloc_cb(n, r, piojo_alloc_default);
}

/**
 * Allocates a product iterator.
 * @param[in] n total items.
 * @param[in] r sample size.
 * @param[in] allocator Allocator to be used.
 * @return New product iterator.
 */
piojo_comb_t*
piojo_comb_prod_alloc_cb(size_t n, size_t r, piojo_alloc_if allocator)
{
        piojo_comb_t * c = comb_alloc(n, r, allocator);
        c->next_cb = prod_next;
        size_t zero = 0;
        for (size_t i=0; i < r; i++) {
                piojo_array_push(&zero, c->sample);
        }
        return c;
}

/**
 * Iterates to the next sample.
 * @param[out] comb combination iterator.
 * @return Indices of the next sample or @b NULL (last sample).
 */
piojo_array_t*
piojo_comb_next(piojo_comb_t *comb)
{
        PIOJO_ASSERT(comb);
        if (comb->first_p) {
                comb->first_p = false;
                return comb->sample;
        }
        return comb->next_cb(comb);
}

/**
 * Frees @a comb and all its samples.
 * @param[in] comb combination iterator being freed.
 */
void
piojo_comb_free(const piojo_comb_t *comb)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(comb);

        allocator = comb->allocator;
        piojo_array_free(comb->sample);
        piojo_array_free(comb->cycles);
        allocator.free_cb(comb);
}

/** @}
 * Private functions.
 */

static piojo_comb_t*
comb_alloc(size_t n, size_t r, piojo_alloc_if allocator)
{
        piojo_comb_t *comb;
        PIOJO_ASSERT(n > 0 && r > 0);

        comb = (piojo_comb_t *) allocator.alloc_cb(sizeof(piojo_comb_t));
        PIOJO_ASSERT(comb);

        comb->n = n;
        comb->r = r;
        comb->allocator = allocator;
        comb->first_p = true;
        comb->sample_idx = r;
        comb->cycles = piojo_array_alloc_cb(sizeof(size_t), allocator);
        comb->sample = piojo_array_alloc_cb(sizeof(size_t), allocator);

        return comb;
}

static piojo_array_t*
comb_next(piojo_comb_t *comb)
{
        while (comb->sample_idx > 0) {
                comb->sample_idx--;
                size_t * si = (size_t *) piojo_array_at(comb->sample_idx, comb->sample);
                if (*si < comb->sample_idx + comb->n - comb->r) {
                        *si += 1;
                        for (size_t i = comb->sample_idx + 1; i < comb->r; i++) {
                                size_t * sj = (size_t *) piojo_array_at(i, comb->sample);
                                *sj = *si + i - comb->sample_idx;
                        }
                        comb->sample_idx = comb->r;
                        return comb->sample;
                }
        }
        return NULL;
}

static piojo_array_t*
prod_next(piojo_comb_t *comb)
{
        while (comb->sample_idx > 0) {
                comb->sample_idx--;
                size_t *idx = (size_t *) piojo_array_at(comb->sample_idx, comb->sample);
                if (*idx < comb->n - 1) {
                        (*idx)++;
                        comb->sample_idx = comb->r;
                        return comb->sample;
                }
                *idx = 0;
        }
        return NULL;
}

static piojo_array_t*
comb_with_rep_next(piojo_comb_t *comb)
{
        while (comb->sample_idx > 0) {
                comb->sample_idx--;
                size_t * si = (size_t *) piojo_array_at(comb->sample_idx, comb->sample);
                if (*si < comb->n - 1) {
                        (*si)++;
                        for (size_t i = comb->sample_idx + 1; i < comb->r; i++) {
                                size_t * sj = (size_t *) piojo_array_at(i, comb->sample);
                                *sj = *si;
                        }
                        comb->sample_idx = comb->r;
                        return comb->sample;
                }
        }
        return NULL;
}

static piojo_array_t*
perm_next(piojo_comb_t *comb)
{
        while (comb->sample_idx > 0) {
                comb->sample_idx--;
                size_t * ci = (size_t *) piojo_array_at(comb->sample_idx, comb->cycles);
                if (--(*ci) > 0) {
                        size_t * si = (size_t *) piojo_array_at(comb->sample_idx, comb->sample);
                        size_t * sj = (size_t *) piojo_array_at(comb->n - (*ci), comb->sample);
                        size_t tmp = *si;
                        *si = *sj;
                        *sj = tmp;
                        comb->sample_idx = comb->r;
                        return comb->sample;
                }
                size_t si = *(size_t *) piojo_array_at(comb->sample_idx, comb->sample);
                piojo_array_delete(comb->sample_idx, comb->sample);
                piojo_array_push(&si, comb->sample);
                *ci = comb->n - comb->sample_idx;
        }
        return NULL;
}


