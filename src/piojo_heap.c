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
#include <piojo/piojo_hash.h>
#include <piojo/piojo_array.h>
#include <piojo_defs.h>

struct piojo_heap_t {
        piojo_array_t *data;
        piojo_hash_t *indices_by_data;
        piojo_heap_leq_cb leq;
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
 * @param[in] leq Entry comparison function.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc(piojo_heap_leq_cb leq)
{
        return piojo_heap_alloc_cb(leq, piojo_alloc_default);
}

/**
 * Allocates a new heap.
 * @param[in] leq Entry comparison function.
 * @param[in] allocator Allocator to be used.
 * @return New heap.
 */
piojo_heap_t*
piojo_heap_alloc_cb(piojo_heap_leq_cb leq, piojo_alloc_if allocator)
{
        piojo_alloc_if ator = piojo_alloc_default;
        piojo_heap_t * h;

        ator.alloc_cb = allocator.alloc_cb;
        ator.realloc_cb = allocator.realloc_cb;
        ator.free_cb = allocator.free_cb;

        h = (piojo_heap_t *) allocator.alloc_cb(sizeof(piojo_heap_t));
        PIOJO_ASSERT(h);

        h->allocator = allocator;
        h->leq = leq;
        h->data = piojo_array_alloc_cb(sizeof(piojo_opaque_t), h->allocator);
        PIOJO_ASSERT(h->data);
        h->indices_by_data = piojo_hash_alloc_cb_eq(sizeof(size_t),
                                                    piojo_opaque_eq,
                                                    sizeof(piojo_opaque_t),
                                                    ator);
        PIOJO_ASSERT(h->indices_by_data);

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
        newh->leq = heap->leq;
        newh->data = piojo_array_copy(heap->data);
        PIOJO_ASSERT(newh->data);
        newh->indices_by_data = piojo_hash_copy(heap->indices_by_data);
        PIOJO_ASSERT(newh->indices_by_data);

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

        piojo_hash_free(heap->indices_by_data);
        piojo_array_free(heap->data);
        allocator = heap->allocator;
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

        piojo_hash_clear(heap->indices_by_data);
        piojo_array_clear(heap->data);
}

/**
 * Expands or shrinks allocated memory for @a ecount entries.
 * @param[in] ecount Must be equal or greater than the current size.
 * @param[out] heap Heap being modified.
 */
void
piojo_heap_resize(size_t ecount, piojo_heap_t *heap)
{
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(ecount >= piojo_array_size(heap->data));

        piojo_array_resize(ecount, heap->data);
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
 * @warning @a data can't be inserted more than once.
 * @param[in] data Entry value.
 * @param[out] heap Heap being modified.
 */
void
piojo_heap_push(piojo_opaque_t data, piojo_heap_t *heap)
{
        size_t idx;
        bool inserted_p;
        PIOJO_ASSERT(heap);

        piojo_array_push(&data, heap->data);

        idx = piojo_heap_size(heap) - 1;
        inserted_p = piojo_hash_insert(&data, &idx,
                                       heap->indices_by_data);
        PIOJO_ASSERT(inserted_p);

        sort_up(idx, heap);
}

/**
 * Decreases existing entry key.
 * @param[in] data Entry value.
 * @param[out] heap Heap being modified.
 */
void
piojo_heap_decrease(piojo_opaque_t data, piojo_heap_t *heap)
{
        size_t *idx;
        PIOJO_ASSERT(heap);

        idx = (size_t*)piojo_hash_search(&data, heap->indices_by_data);
        PIOJO_ASSERT(idx != NULL);

        sort_up(*idx, heap);
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

        piojo_hash_delete(piojo_array_at(lastidx, heap->data),
                          heap->indices_by_data);
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
        PIOJO_ASSERT(heap);
        PIOJO_ASSERT(piojo_heap_size(heap) > 0);

        return *(piojo_opaque_t *)piojo_array_at(0, heap->data);
}

/**
 * Searches entry in heap.
 * @param[in] data Entry value.
 * @param[in] heap
 * @return @b TRUE if entry is in heap, @b FALSE otherwise.
 */
bool
piojo_heap_contain_p(piojo_opaque_t data, const piojo_heap_t *heap)
{
        return (piojo_hash_search(&data, heap->indices_by_data) != NULL);
}

/** @}
 * Private functions.
 */

static void
sort_up(size_t idx, piojo_heap_t *heap)
{
        size_t pidx;
        while (idx > 0){
                pidx = (idx - 1) / 2;
                if (entry_leq(idx, pidx, heap)){
                        swap(idx, pidx, heap);
                        idx = pidx;
                }else{
                        break;
                }
        }
}

static void
sort_down(size_t idx, size_t hsize, piojo_heap_t *heap)
{
        size_t lidx, ridx, *swapidx;
        while (idx < hsize){
                swapidx = NULL;
                lidx = ridx = hsize;
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
                        idx = *swapidx;
                }else{
                        break;
                }
        }
}

static bool
entry_leq(size_t idx1, size_t idx2, piojo_heap_t *heap)
{
        return heap->leq(*(piojo_opaque_t *)piojo_array_at(idx1, heap->data),
                         *(piojo_opaque_t *)piojo_array_at(idx2, heap->data));
}

static void
swap(size_t idx1, size_t idx2, piojo_heap_t *heap)
{
        piojo_opaque_t *e1, *e2;
        piojo_opaque_t tmp;
        const size_t esize = sizeof(piojo_opaque_t);

        e1 = (piojo_opaque_t *)piojo_array_at(idx1, heap->data);
        e2 = (piojo_opaque_t *)piojo_array_at(idx2, heap->data);

        piojo_hash_set(e1, &idx2, heap->indices_by_data);
        piojo_hash_set(e2, &idx1, heap->indices_by_data);

        memcpy(&tmp, e1, esize);
        memcpy(e1, e2, esize);
        memcpy(e2, &tmp, esize);
}
