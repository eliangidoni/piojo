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
 * @addtogroup piojoqueue Piojo Queue
 * @{
 * Piojo Queue implementation.
 */

#include <piojo/piojo_queue.h>
#include <piojo_defs.h>

struct piojo_queue {
        uint8_t *data;
        size_t esize, ecount, widx, ridx, usedcnt;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of queue in bytes */
const size_t piojo_queue_sizeof = sizeof(piojo_queue_t);

static void
incr_and_wrap(size_t *idx, size_t maxcnt);

static void
finish_all(const piojo_queue_t *queue);

/**
 * Allocates a new queue.
 * Uses default allocator and entry size of @b int.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc(size_t ecount)
{
        return piojo_queue_alloc_s(sizeof(int), ecount);
}

/**
 * Allocates a new queue.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc_s(size_t esize, size_t ecount)
{
        return piojo_queue_alloc_cb(esize, ecount,
                                    piojo_alloc_default);
}

/**
 * Allocates a new queue.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc_cb(size_t esize, size_t ecount, piojo_alloc_if allocator)
{
        piojo_queue_t * q;
        q = (piojo_queue_t *) allocator.alloc_cb(sizeof(piojo_queue_t));
        PIOJO_ASSERT(q);
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(ecount > 0);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(esize, ecount));

        q->allocator = allocator;
        q->widx = q->ridx = q->usedcnt = 0;
        q->esize = esize;
        q->ecount = ecount;
        q->data = (uint8_t *) allocator.alloc_cb(q->ecount * q->esize);
        PIOJO_ASSERT(q->data);

        return q;
}

/**
 * Copies @a queue and all its entries.
 * @param[in] queue Queue being copied.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_copy(const piojo_queue_t *queue)
{
        piojo_queue_t *newq;
        size_t esize, ridx, i;
        piojo_alloc_if allocator;
        PIOJO_ASSERT(queue);

        allocator = queue->allocator;
        esize = queue->esize;

        newq = piojo_queue_alloc_cb(esize, queue->ecount, allocator);
        PIOJO_ASSERT(newq);
        newq->widx = queue->widx;
        newq->ridx = queue->ridx;
        newq->usedcnt = queue->usedcnt;

        ridx = queue->ridx;
        for (i = 0; i < queue->usedcnt; ++i){
                allocator.copy_cb(&queue->data[ridx * esize],
                                  esize, &newq->data[ridx * esize]);
                incr_and_wrap(&ridx, queue->ecount);
        }

        return newq;
}

/**
 * Frees @a queue and all its entries.
 * @param[in] queue Queue being freed.
 */
void
piojo_queue_free(const piojo_queue_t *queue)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(queue);

        allocator = queue->allocator;

        finish_all(queue);
        allocator.free_cb(queue->data);
        allocator.free_cb(queue);
}

/**
 * Deletes all entries in @a queue.
 * @param[out] queue Queue being cleared.
 */
void
piojo_queue_clear(piojo_queue_t *queue)
{
        PIOJO_ASSERT(queue);
        finish_all(queue);
        queue->widx = queue->ridx = queue->usedcnt = 0;
}

/**
 * Returns number of entries.
 * @param[in] queue
 * @return Number of entries in @a queue.
 */
size_t
piojo_queue_size(const piojo_queue_t *queue)
{
        PIOJO_ASSERT(queue);
        return queue->usedcnt;
}

/**
 * Returns whether @a queue is full.
 * @param[in] queue
 * @return @b TRUE if @a queue is full, @b FALSE otherwise.
 */
bool
piojo_queue_full_p(const piojo_queue_t *queue)
{
        PIOJO_ASSERT(queue);
        return (piojo_queue_size(queue) == queue->ecount);
}

/**
 * Inserts a new entry after the last entry.
 * @warning You should check @a queue is not full.
 * @param[in] data Entry value.
 * @param[out] queue Queue being modified.
 */
void
piojo_queue_push(const void *data, piojo_queue_t *queue)
{
        size_t curidx;
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(! piojo_queue_full_p(queue));

        curidx = queue->widx * queue->esize;
        queue->allocator.init_cb(data, queue->esize, &queue->data[curidx]);
        incr_and_wrap(&queue->widx, queue->ecount);
        ++queue->usedcnt;
}

/**
 * Deletes the first entry.
 * @param[out] queue Non-empty queue.
 */
void
piojo_queue_pop(piojo_queue_t *queue)
{
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) > 0);

        queue->allocator.finish_cb(piojo_queue_peek(queue));
        incr_and_wrap(&queue->ridx, queue->ecount);
        --queue->usedcnt;
}

/**
 * Reads the first entry.
 * @param[in] queue Non-empty queue.
 * @return Entry value.
 */
void*
piojo_queue_peek(const piojo_queue_t *queue)
{
        PIOJO_ASSERT(queue);
        PIOJO_ASSERT(piojo_queue_size(queue) > 0);
        return &queue->data[queue->ridx * queue->esize];
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
finish_all(const piojo_queue_t *queue)
{
        size_t ridx, i;
        ridx = queue->ridx;
        for (i = 0; i < queue->usedcnt; ++i){
                queue->allocator.finish_cb(&queue->data[ridx * queue->esize]);
                incr_and_wrap(&ridx, queue->ecount);
        }
}
