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
        piojo_queue_dyn_t dyn;
        size_t esize, ecount, widx, ridx, wcnt, rcnt;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of queue in bytes */
const size_t piojo_queue_sizeof = sizeof(piojo_queue_t);

static void
expand_queue(piojo_queue_t *queue);

static void
incr_and_wrap(size_t *idx, size_t *cnt, size_t maxcnt);

static void
finish_all(const piojo_queue_t *queue);

/**
 * Allocates a new queue.
 * Uses default allocator and entry size of @b int.
 * @param[in] dyn Whether the queue should be expanded when full.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc(piojo_queue_dyn_t dyn)
{
        return piojo_queue_alloc_s(dyn, sizeof(int));
}

/**
 * Allocates a new queue.
 * Uses default allocator.
 * @param[in] dyn Whether the queue should be expanded when full.
 * @param[in] esize Entry size in bytes.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc_s(piojo_queue_dyn_t dyn, size_t esize)
{
        return piojo_queue_alloc_n(dyn, esize, DEFAULT_ADT_ECOUNT);
}

/**
 * Allocates a new queue.
 * Uses default allocator.
 * @param[in] dyn Whether the queue should be expanded when full.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc_n(piojo_queue_dyn_t dyn, size_t esize, size_t ecount)
{
        return piojo_queue_alloc_cb_n(dyn, esize, ecount,
                                      piojo_alloc_default);
}

/**
 * Allocates a new queue.
 * @param[in] dyn Whether the queue should be expanded when full.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc_cb(piojo_queue_dyn_t dyn, size_t esize,
                     piojo_alloc_if allocator)
{
        return piojo_queue_alloc_cb_n(dyn, esize, DEFAULT_ADT_ECOUNT,
                                      allocator);
}

/**
 * Allocates a new queue.
 * @param[in] dyn Whether the queue should be expanded when full.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New queue.
 */
piojo_queue_t*
piojo_queue_alloc_cb_n(piojo_queue_dyn_t dyn, size_t esize, size_t ecount,
                       piojo_alloc_if allocator)
{
        piojo_queue_t * q;
        q = (piojo_queue_t *) allocator.alloc_cb(sizeof(piojo_queue_t));
        PIOJO_ASSERT(q);
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(ecount > 0);

        q->allocator = allocator;
        q->widx = q->ridx = q->wcnt = q->rcnt = 0;
        q->esize = esize;
        q->ecount = ecount;
        q->dyn = dyn;
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
        size_t esize, ridx, rcnt;
        piojo_alloc_if allocator;
        PIOJO_ASSERT(queue);

        allocator = queue->allocator;
        esize = queue->esize;

        newq = piojo_queue_alloc_cb_n(queue->dyn, esize, queue->ecount,
                                      allocator);
        PIOJO_ASSERT(newq);
        newq->widx = queue->widx;
        newq->ridx = queue->ridx;
        newq->wcnt = queue->wcnt;
        newq->rcnt = queue->rcnt;

        ridx = queue->ridx;
        rcnt = queue->rcnt;
        while ((size_t)labs(queue->wcnt - rcnt) > 0){
                allocator.copy_cb(&queue->data[ridx * esize],
                                  esize, &newq->data[ridx * esize]);
                incr_and_wrap(&ridx, &rcnt, queue->ecount);
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
        finish_all(queue);
        queue->widx = queue->ridx = queue->wcnt = queue->rcnt = 0;
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
        return labs(queue->wcnt - queue->rcnt);
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
        return (piojo_queue_size(queue) == queue->ecount &&
                queue->dyn == PIOJO_QUEUE_DYN_FALSE);
}

/**
 * Inserts a new entry after the last entry.
 * If @a queue is dynamic then expand it.
 * @warning If @a queue isn't dynamic you should check it's not full.
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

        if (piojo_queue_size(queue) == queue->ecount){
                expand_queue(queue);
        }

        curidx = queue->widx * queue->esize;
        queue->allocator.init_cb(data, queue->esize, &queue->data[curidx]);
        incr_and_wrap(&queue->widx, &queue->wcnt, queue->ecount);
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
        incr_and_wrap(&queue->ridx, &queue->rcnt, queue->ecount);
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
expand_queue(piojo_queue_t *queue)
{
        size_t bycnt = queue->ecount * DEFAULT_ADT_GROWTH_RATIO;
        size_t idx = queue->ridx * queue->esize;
        void *atidx_data = &queue->data[idx];
        size_t atidx_size = (queue->ecount - queue->ridx) * queue->esize;
        size_t size = (queue->ecount + bycnt) * queue->esize;
        uint8_t *expanded = (uint8_t *) queue->allocator.alloc_cb(size);
        PIOJO_ASSERT(expanded);
        PIOJO_ASSERT(queue->ridx == queue->widx);

        memcpy(expanded, atidx_data, atidx_size);
        memcpy(expanded + atidx_size, queue->data, idx);
        queue->allocator.free_cb(queue->data);

        queue->ridx = 0;
        queue->widx = queue->ecount;
        queue->ecount += bycnt;
        queue->data = expanded;
}

static void
incr_and_wrap(size_t *idx, size_t *cnt, size_t maxcnt)
{
        if (++(*idx) == maxcnt){
                *idx = 0;
        }
        ++(*cnt);
}

static void
finish_all(const piojo_queue_t *queue)
{
        size_t ridx, rcnt;
        ridx = queue->ridx;
        rcnt = queue->rcnt;
        while ((size_t)labs(queue->wcnt - rcnt) > 0){
                queue->allocator.finish_cb(&queue->data[ridx * queue->esize]);
                incr_and_wrap(&ridx, &rcnt, queue->ecount);
        }
}
