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
 * @addtogroup piojoheap Piojo Heap
 * @{
 * Piojo Heap implementation.
 */

#include <piojo/piojo_heap.h>
#include <piojo/piojo_array.h>
#include <piojo_defs.h>

struct piojo_heap {
        piojo_array_t *data;
        size_t esize;
        piojo_leq_cb cmp;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of heap in bytes */
const size_t piojo_heap_sizeof = sizeof(piojo_heap_t);

static bool
int_leq(const void *e1, const void *e2);

static void
sort_up(size_t idx, void *tmp, piojo_heap_t *heap);

static void
sort_down(size_t idx, size_t hsize, void *tmp, piojo_heap_t *heap);

static void
swap(void *e1, void *e2, void *tmp, size_t esize);

/**
 * Allocates a new heap (max-heap).
 * Uses default allocator and entry size of @b int.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc()
{
        return piojo_heap_alloc_s(int_leq, sizeof(int));
}

/**
 * Allocates a new heap.
 * Uses default allocator.
 * @param[in] cmp Entry comparison function.
 * @param[in] esize Entry size in bytes.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_s(piojo_leq_cb cmp, size_t esize)
{
        return piojo_heap_alloc_n(cmp, esize, DEFAULT_ADT_ECOUNT);
}

/**
 * Allocates a new heap.
 * Uses default allocator.
 * @param[in] cmp Entry comparison function.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_n(piojo_leq_cb cmp, size_t esize, size_t ecount)
{
        return piojo_heap_alloc_cb_n(cmp, esize, ecount,
                                     piojo_alloc_default);
}

/**
 * Allocates a new heap.
 * @param[in] cmp Entry comparison function.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_cb(piojo_leq_cb cmp, size_t esize,
                    piojo_alloc_if allocator)
{
        return piojo_heap_alloc_cb_n(cmp, esize, DEFAULT_ADT_ECOUNT,
                                     allocator);
}

/**
 * Allocates a new heap.
 * @param[in] cmp Entry comparison function.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_cb_n(piojo_leq_cb cmp, size_t esize, size_t ecount,
                      piojo_alloc_if allocator)
{
        piojo_heap_t * h;
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(ecount > 0);

        h = (piojo_heap_t *) allocator.alloc_cb(sizeof(piojo_heap_t));
        PIOJO_ASSERT(h);

        h->allocator = allocator;
        h->esize = esize;
        h->cmp = cmp;
        h->data = piojo_array_alloc_cb_n(NULL, esize, ecount, h->allocator);
        PIOJO_ASSERT(h->data);

        return h;
}

/**
 * Copies @a heap and all its entries.
 * @param[in] heap Heap being copied.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_copy(const piojo_heap_t *heap)
{
        piojo_heap_t *newh;
        piojo_alloc_if allocator;
        PIOJO_ASSERT(heap);

        allocator = heap->allocator;
        newh = (piojo_heap_t *) allocator.alloc_cb(sizeof(piojo_heap_t));
        PIOJO_ASSERT(newh);

        newh->allocator = allocator;
        newh->esize = heap->esize;
        newh->cmp = heap->cmp;
        newh->data = piojo_array_copy(heap->data);
        PIOJO_ASSERT(newh->data);

        return newh;
}

/**
 * Frees @a heap and all its entries.
 * @param[in] heap Heap being freed.
 */
void
piojo_heap_free(const piojo_heap_t *heap)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(heap);

        allocator = heap->allocator;
        piojo_array_free(heap->data);
        allocator.free_cb(heap);
}

/**
 * Deletes all entries in @a heap.
 * @param[out] heap Heap being cleared.
 */
void
piojo_heap_clear(piojo_heap_t *heap)
{
        PIOJO_ASSERT(heap);

        piojo_array_clear(heap->data);
}

/**
 * Returns number of entries.
 * @param[in] heap
 * @return Number of entries in @a heap.
 */
size_t
piojo_heap_size(const piojo_heap_t *heap)
{
        PIOJO_ASSERT(heap);

        return piojo_array_size(heap->data);
}

/**
 * Inserts a new entry.
 * @param[in] data Entry value.
 * @param[out] heap Heap being modified.
 */
void
piojo_heap_push(const void *data, piojo_heap_t *heap)
{
        void *tmp;
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(data);

        tmp = heap->allocator.alloc_cb(heap->esize);
        PIOJO_ASSERT(tmp);

        piojo_array_append(data, heap->data);
        sort_up(piojo_heap_size(heap) - 1, tmp, heap);

        heap->allocator.free_cb(tmp);
}

/**
 * Deletes the root (min or max) entry.
 * @param[out] heap Non-empty heap.
 */
void
piojo_heap_pop(piojo_heap_t *heap)
{
        void *tmp;
        size_t lastidx;
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) > 0);

        tmp = heap->allocator.alloc_cb(heap->esize);
        PIOJO_ASSERT(tmp);

        lastidx = piojo_array_size(heap->data) - 1;
        swap(piojo_array_at(0, heap->data),
             piojo_array_at(lastidx, heap->data),
             tmp, heap->esize);

        piojo_array_delete(lastidx, heap->data);
        sort_down(0, lastidx, tmp, heap);

        heap->allocator.free_cb(tmp);
}

/**
 * Reads the root (min or max) entry.
 * @param[in] heap Non-empty heap.
 * @return Entry value.
 */
void*
piojo_heap_peek(const piojo_heap_t *heap)
{
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) > 0);

        return piojo_array_at(0, heap->data);
}

/** @}
 * Private functions.
 */

static bool
int_leq(const void *e1, const void *e2)
{
        int v1 = *(int*) e1;
        int v2 = *(int*) e2;
        if (v1 <= v2){
                return TRUE;
        }
        return FALSE;
}

static void
sort_up(size_t idx, void *tmp, piojo_heap_t *heap)
{
        void *parent, *elem;
        size_t pidx;

        if (idx > 0){
                pidx = (idx - 1) / 2;
                parent = piojo_array_at(pidx, heap->data);
                elem = piojo_array_at(idx, heap->data);
                if (heap->cmp(parent, elem)){
                        swap(parent, elem, tmp, heap->esize);
                        sort_up(pidx, tmp, heap);
                }
        }
}

static void
sort_down(size_t idx, size_t hsize, void *tmp, piojo_heap_t *heap)
{
        void *swape=NULL, *elem;
        size_t lidx, ridx, *swapidx=NULL;

        if (idx < hsize){
                elem = piojo_array_at(idx, heap->data);
                lidx = (idx * 2) + 1;
                ridx = lidx + 1;
                if (ridx < hsize){
                        swapidx = &lidx;
                        if (heap->cmp(piojo_array_at(lidx, heap->data),
                                      piojo_array_at(ridx, heap->data))){
                                swapidx = &ridx;
                        }
                }else if (lidx < hsize){
                        swapidx = &lidx;
                }

                if (swapidx){
                        swape = piojo_array_at(*swapidx, heap->data);
                }
                if (swape && heap->cmp(elem, swape)){
                        swap(swape, elem, tmp, heap->esize);
                        sort_down(*swapidx, hsize, tmp, heap);
                }
        }
}

static void
swap(void *e1, void *e2, void *tmp, size_t esize)
{
        memcpy(tmp, e1, esize);
        memcpy(e1, e2, esize);
        memcpy(e2, tmp, esize);
}
