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
 * @addtogroup piojoring Piojo Ring
 * @{
 * Piojo Ring/Circular buffer implementation.
 */

#include <piojo/piojo_ring.h>
#include <piojo_defs.h>

struct piojo_ring_t {
        uint8_t *data;
        size_t esize, ecount, widx, ridx, usedcnt;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of ring in bytes */
const size_t piojo_ring_sizeof = sizeof(piojo_ring_t);

static void
incr_and_wrap(size_t *idx, size_t maxcnt);

static void
finish_all(const piojo_ring_t *ring);

/**
 * Allocates a new ring.
 * Uses default allocator and entry size of @b int.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New ring.
 */
piojo_ring_t*
piojo_ring_alloc(size_t ecount)
{
        return piojo_ring_alloc_s(sizeof(int), ecount);
}

/**
 * Allocates a new ring.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New ring.
 */
piojo_ring_t*
piojo_ring_alloc_s(size_t esize, size_t ecount)
{
        return piojo_ring_alloc_cb(esize, ecount,
                                    piojo_alloc_default);
}

/**
 * Allocates a new ring.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New ring.
 */
piojo_ring_t*
piojo_ring_alloc_cb(size_t esize, size_t ecount, piojo_alloc_if allocator)
{
        piojo_ring_t * r;
        r = (piojo_ring_t *) allocator.alloc_cb(sizeof(piojo_ring_t));
        PIOJO_ASSERT(r);
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(ecount > 0);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(esize, ecount));

        r->allocator = allocator;
        r->widx = r->ridx = r->usedcnt = 0;
        r->esize = esize;
        r->ecount = ecount;
        r->data = (uint8_t *) allocator.alloc_cb(r->ecount * r->esize);
        PIOJO_ASSERT(r->data);

        return r;
}

/**
 * Copies @a ring and all its entries.
 * @param[in] ring Ring being copied.
 * @return New ring.
 */
piojo_ring_t*
piojo_ring_copy(const piojo_ring_t *ring)
{
        piojo_ring_t *newr;
        size_t esize, ridx, i;
        piojo_alloc_if allocator;
        PIOJO_ASSERT(ring);

        allocator = ring->allocator;
        esize = ring->esize;

        newr = piojo_ring_alloc_cb(esize, ring->ecount, allocator);
        PIOJO_ASSERT(newr);
        newr->widx = ring->widx;
        newr->ridx = ring->ridx;
        newr->usedcnt = ring->usedcnt;

        ridx = ring->ridx;
        for (i = 0; i < ring->usedcnt; ++i){
                allocator.copy_cb(&ring->data[ridx * esize],
                                  esize, &newr->data[ridx * esize]);
                incr_and_wrap(&ridx, ring->ecount);
        }

        return newr;
}

/**
 * Frees @a ring and all its entries.
 * @param[in] ring Ring being freed.
 */
void
piojo_ring_free(const piojo_ring_t *ring)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(ring);

        allocator = ring->allocator;

        finish_all(ring);
        allocator.free_cb(ring->data);
        allocator.free_cb(ring);
}

/**
 * Deletes all entries in @a ring.
 * @param[out] ring Ring being cleared.
 */
void
piojo_ring_clear(piojo_ring_t *ring)
{
        PIOJO_ASSERT(ring);
        finish_all(ring);
        ring->widx = ring->ridx = ring->usedcnt = 0;
}

/**
 * Returns number of entries.
 * @param[in] ring
 * @return Number of entries in @a ring.
 */
size_t
piojo_ring_size(const piojo_ring_t *ring)
{
        PIOJO_ASSERT(ring);
        return ring->usedcnt;
}

/**
 * Returns whether @a ring is full.
 * @param[in] ring
 * @return @b TRUE if @a ring is full, @b FALSE otherwise.
 */
bool
piojo_ring_full_p(const piojo_ring_t *ring)
{
        PIOJO_ASSERT(ring);
        return (piojo_ring_size(ring) == ring->ecount);
}

/**
 * Inserts a new entry after the last entry.
 * @warning You should check @a ring is not full.
 * @param[in] data Entry value.
 * @param[out] ring Ring being modified.
 */
void
piojo_ring_push(const void *data, piojo_ring_t *ring)
{
        size_t curidx;
        PIOJO_ASSERT(ring);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(! piojo_ring_full_p(ring));

        curidx = ring->widx * ring->esize;
        ring->allocator.init_cb(data, ring->esize, &ring->data[curidx]);
        incr_and_wrap(&ring->widx, ring->ecount);
        ++ring->usedcnt;
}

/**
 * Deletes the next entry.
 * @param[out] ring Non-empty ring.
 */
void
piojo_ring_pop(piojo_ring_t *ring)
{
        PIOJO_ASSERT(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) > 0);

        ring->allocator.finish_cb(piojo_ring_peek(ring));
        incr_and_wrap(&ring->ridx, ring->ecount);
        --ring->usedcnt;
}

/**
 * Reads the next entry.
 * @param[in] ring Non-empty ring.
 * @return Entry value.
 */
void*
piojo_ring_peek(const piojo_ring_t *ring)
{
        PIOJO_ASSERT(ring);
        PIOJO_ASSERT(piojo_ring_size(ring) > 0);
        return &ring->data[ring->ridx * ring->esize];
}

/** @}
 * Private functions.
 */

static void
incr_and_wrap(size_t *idx, size_t maxcnt)
{
        if (++(*idx) == maxcnt){
                *idx = 0;
        }
}

static void
finish_all(const piojo_ring_t *ring)
{
        size_t ridx, i;
        ridx = ring->ridx;
        for (i = 0; i < ring->usedcnt; ++i){
                ring->allocator.finish_cb(&ring->data[ridx * ring->esize]);
                incr_and_wrap(&ridx, ring->ecount);
        }
}
