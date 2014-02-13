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
 * Piojo Heap (min-heap) implementation.
 */

#include <piojo/piojo_heap.h>
#include <piojo/piojo_array.h>
#include <piojo_defs.h>

typedef struct {
        piojo_opaque_t data;
        piojo_heap_key_t key;
} piojo_heap_entry_t;

struct piojo_heap {
        piojo_array_t *data;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of heap in bytes */
const size_t piojo_heap_sizeof = sizeof(piojo_heap_t);

static void
sort_up(size_t idx, piojo_heap_t *heap);

static void
sort_down(size_t idx, size_t hsize, piojo_heap_t *heap);

static bool
entry_leq(size_t idx1, size_t idx2, piojo_heap_t *heap);

static void
swap(size_t idx1, size_t idx2, piojo_heap_t *heap);

/**
 * Allocates a new heap.
 * Uses default allocator.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc(void)
{
        return piojo_heap_alloc_n(DEFAULT_ADT_ECOUNT);
}

/**
 * Allocates a new heap.
 * Uses default allocator.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_n(size_t ecount)
{
        return piojo_heap_alloc_cb_n(ecount, piojo_alloc_default);
}

/**
 * Allocates a new heap.
 * @param[in] allocator Allocator to be used.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_cb(piojo_alloc_if allocator)
{
        return piojo_heap_alloc_cb_n(DEFAULT_ADT_ECOUNT, allocator);
}

/**
 * Allocates a new heap.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_cb_n(size_t ecount, piojo_alloc_if allocator)
{
        piojo_heap_t * h;
        PIOJO_ASSERT(ecount > 0);

        h = (piojo_heap_t *) allocator.alloc_cb(sizeof(piojo_heap_t));
        PIOJO_ASSERT(h);

        h->allocator = allocator;
        h->data = piojo_array_alloc_cb_n(NULL, sizeof(piojo_heap_entry_t),
                                         ecount, h->allocator);
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
 * @param[in] key Entry key.
 * @param[out] heap Heap being modified.
 */
void
piojo_heap_push(piojo_opaque_t data, piojo_heap_key_t key, piojo_heap_t *heap)
{
        piojo_heap_entry_t entry;
        PIOJO_ASSERT(heap);

        entry.key = key;
        entry.data = data;
        piojo_array_append(&entry, heap->data);

        sort_up(piojo_heap_size(heap) - 1, heap);
}

/**
 * Deletes the minimum entry according to key.
 * @param[out] heap Non-empty heap.
 */
void
piojo_heap_pop(piojo_heap_t *heap)
{
        size_t lastidx;
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) > 0);

        lastidx = piojo_array_size(heap->data) - 1;
        if (lastidx > 0){
                swap(0, lastidx, heap);
        }

        piojo_array_delete(lastidx, heap->data);

        sort_down(0, lastidx, heap);
}

/**
 * Reads the minimum entry according to key.
 * @param[in] heap Non-empty heap.
 * @return Entry value.
 */
piojo_opaque_t
piojo_heap_peek(const piojo_heap_t *heap)
{
        piojo_heap_entry_t *entry;
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) > 0);

        entry = (piojo_heap_entry_t *)piojo_array_at(0, heap->data);
        return entry->data;
}

/** @}
 * Private functions.
 */

static void
sort_up(size_t idx, piojo_heap_t *heap)
{
        size_t pidx;
        if (idx > 0){
                pidx = (idx - 1) / 2;
                if (entry_leq(idx, pidx, heap)){
                        swap(idx, pidx, heap);
                        sort_up(pidx, heap);
                }
        }
}

static void
sort_down(size_t idx, size_t hsize, piojo_heap_t *heap)
{
        size_t lidx=hsize, ridx=hsize, *swapidx=NULL;

        if (idx < hsize){
                if (piojo_safe_mulsiz_p(idx, 2) &&
                    piojo_safe_addsiz_p(idx * 2, 1)){
                        lidx = (idx * 2) + 1;
                        if (piojo_safe_addsiz_p(lidx, 1)){
                                ridx = lidx + 1;
                        }
                }
                if (ridx < hsize){
                        swapidx = &lidx;
                        if (entry_leq(ridx, lidx, heap)){
                                swapidx = &ridx;
                        }
                }else if (lidx < hsize){
                        swapidx = &lidx;
                }
                if (swapidx && entry_leq(*swapidx, idx, heap)){
                        swap(*swapidx, idx, heap);
                        sort_down(*swapidx, hsize, heap);
                }
        }
}

static bool
entry_leq(size_t idx1, size_t idx2, piojo_heap_t *heap)
{
        piojo_heap_entry_t *e1, *e2;
        e1 = (piojo_heap_entry_t *)piojo_array_at(idx1, heap->data);
        e2 = (piojo_heap_entry_t *)piojo_array_at(idx2, heap->data);
        return (e1->key <= e2->key);
}

static void
swap(size_t idx1, size_t idx2, piojo_heap_t *heap)
{
        void *e1, *e2;
        piojo_heap_entry_t tmp;
        const size_t esize = sizeof(piojo_heap_entry_t);

        e1 = piojo_array_at(idx1, heap->data);
        e2 = piojo_array_at(idx2, heap->data);

        memcpy(&tmp, e1, esize);
        memcpy(e1, e2, esize);
        memcpy(e2, &tmp, esize);
}
