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
 * @addtogroup piojoarray Piojo Array
 * @{
 * Piojo Array implementation.
 */

#include <piojo/piojo_array.h>
#include <piojo_defs.h>

struct piojo_array {
        uint8_t *data;
        size_t esize, usedcnt, ecount;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of array in bytes */
const size_t piojo_array_sizeof = sizeof(piojo_array_t);

static void
move_left_from(size_t idx, piojo_array_t *array);

static void
move_right_until(size_t idx, piojo_array_t *array);

static void
expand_array(size_t incr_cnt, piojo_array_t *array);

static void
finish_all(const piojo_array_t *array);

static size_t*
entry_index(const void *data, piojo_cmp_cb cmp,
            const piojo_array_t *array, size_t *idx);

/**
 * Allocates a new array.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc(size_t esize)
{
        return piojo_array_alloc_cb(esize, piojo_alloc_default);
}

/**
 * Allocates a new array.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc_cb(size_t esize, piojo_alloc_if allocator)
{
        piojo_array_t * arr;
        const size_t ecount = DEFAULT_ADT_ECOUNT;

        arr = (piojo_array_t *) allocator.alloc_cb(sizeof(piojo_array_t));
        PIOJO_ASSERT(arr);
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(esize, ecount));

        arr->allocator = allocator;
        arr->esize = esize;
        arr->ecount = ecount;
        arr->usedcnt = 0;
        arr->data = (uint8_t *) arr->allocator.alloc_cb(arr->ecount *
                                                        arr->esize);
        PIOJO_ASSERT(arr->data);

        return arr;
}

/**
 * Copies @a array and all its entries.
 * @param[in] array Array being copied.
 * @return New array.
 */
piojo_array_t*
piojo_array_copy(const piojo_array_t *array)
{
        size_t esize;
        size_t i;
        piojo_array_t *newarray;
        piojo_alloc_if allocator;
        PIOJO_ASSERT(array);

        allocator = array->allocator;
        esize = array->esize;

        newarray = piojo_array_alloc_cb(esize, allocator);
        PIOJO_ASSERT(newarray);

        piojo_array_reserve(array->ecount, newarray);
        newarray->usedcnt = array->usedcnt;

        for (i = 0; i < array->usedcnt; ++i){
                allocator.copy_cb(piojo_array_at(i, array),
                                  esize, piojo_array_at(i, newarray));
        }

        return newarray;
}

/**
 * Frees @a array and all its entries.
 * @param[in] array Array being freed.
 */
void
piojo_array_free(const piojo_array_t *array)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(array);

        allocator = array->allocator;

        finish_all(array);
        allocator.free_cb(array->data);
        allocator.free_cb(array);
}

/**
 * Deletes all entries in @a array.
 * @param[out] array Array being cleared.
 */
void
piojo_array_clear(piojo_array_t *array)
{
        PIOJO_ASSERT(array);

        finish_all(array);
        array->usedcnt = 0;
}

/**
 * Reserves or shrinks allocated memory for @a ecount entries.
 * @param[in] ecount Must be equal or greater than the current size.
 * @param[out] array Array being modified.
 */
void
piojo_array_reserve(size_t ecount, piojo_array_t *array)
{
        size_t size;
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(ecount >= array->usedcnt);

        if (ecount > array->ecount){
                expand_array(ecount - array->ecount, array);
        }else if (ecount < array->ecount){
                /* Shrink to new size. */
                ecount = piojo_maxsiz(ecount, 1);
                size = ecount * array->esize;
                array->data = ((uint8_t *)
                               array->allocator.realloc_cb(array->data, size));
                PIOJO_ASSERT(array->data);
                array->ecount = ecount;
        }
}

/**
 * Returns number of entries.
 * @param[in] array
 * @return Number of entries in @a array.
 */
size_t
piojo_array_size(const piojo_array_t *array)
{
        PIOJO_ASSERT(array);

        return array->usedcnt;
}

/**
 * Inserts a new entry.
 * @param[in] idx Index of entry being inserted (from 0 to array_size)
 * @param[in] data Entry value.
 * @param[out] array Array being modified.
 */
void
piojo_array_insert(size_t idx, const void *data, piojo_array_t *array)
{
        size_t curidx;
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(idx <= array->usedcnt);
        PIOJO_ASSERT(data);

        curidx = idx * array->esize;
        if (idx < array->ecount){
                move_right_until(idx, array);
        }else{
                expand_array(array->ecount / ADT_GROWTH_DENOMINATOR, array);
        }
        array->allocator.init_cb(data, array->esize, &array->data[curidx]);
        ++array->usedcnt;
}

/**
 * Replaces an entry.
 * @param[in] idx Index of entry being replaced (from 0 to array_size - 1)
 * @param[in] data Entry value.
 * @param[out] array Array being modified.
 */
void
piojo_array_set(size_t idx, const void *data, piojo_array_t *array)
{
        size_t curidx;
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(idx < array->usedcnt);
        PIOJO_ASSERT(data);

        curidx = idx * array->esize;
        array->allocator.finish_cb(piojo_array_at(idx, array));
        array->allocator.init_cb(data, array->esize, &array->data[curidx]);
}

/**
 * Inserts a new entry at the end of @a array.
 * @param[in] data Entry value.
 * @param[out] array Array being modified.
 */
void
piojo_array_push(const void *data, piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(data);

        piojo_array_insert(piojo_array_size(array), data, array);
}

/**
 * Searches index of an entry (linear search).
 * @param[in] data Entry value.
 * @param[in] cmp Entry comparison function.
 * @param[in] array Array.
 * @param[out] idx Index of @a data inside @a array.
 * @return @a idx if @a data is present, @b NULL otherwise.
 */
size_t*
piojo_array_index(const void *data, piojo_cmp_cb cmp,
                  const piojo_array_t *array, size_t *idx)
{
        size_t i, cnt;
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(idx);

        cnt = piojo_array_size(array);
        for (i=0; i < cnt; ++i){
                if (cmp(data, piojo_array_at(i, array)) == 0){
                        *idx = i;
                        return idx;
                }
        }
        return NULL;
}

/**
 * Inserts a new entry in order given by @a cmp.
 * @param[in] data Entry value.
 * @param[in] cmp Entry comparison function.
 * @param[out] array Array being modified.
 */
void
piojo_array_sorted_insert(const void *data, piojo_cmp_cb cmp,
                          piojo_array_t *array)
{
        size_t i=0;
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(data);

        if (piojo_array_size(array) > 0){
                entry_index(data, cmp, array, &i);
        }
        piojo_array_insert(i, data, array);
}

/**
 * Searches index of an entry (binary search).
 * @param[in] data Entry value.
 * @param[in] cmp Entry comparison function.
 * @param[in] array Array.
 * @param[out] idx Index of @a data inside @a array.
 * @return @a idx if @a data is present, @b NULL otherwise.
 */
size_t*
piojo_array_sorted_index(const void *data, piojo_cmp_cb cmp,
                         const piojo_array_t *array, size_t *idx)
{
        size_t index;
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(idx);

        if (entry_index(data, cmp, array, &index) != NULL){
                *idx = index;
                return idx;
        }
        return NULL;
}

/**
 * Deletes an entry.
 * @param[in] idx Index of entry being deleted (from 0 to array_size - 1).
 * @param[out] array Non-empty array.
 */
void
piojo_array_delete(size_t idx, piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(idx < array->usedcnt);

        array->allocator.finish_cb(piojo_array_at(idx, array));
        --array->usedcnt;
        move_left_from(idx, array);
}

/**
 * Deletes the last entry.
 * @param[out] array Non-empty array.
 */
void
piojo_array_pop(piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(array->usedcnt > 0);

        piojo_array_delete(array->usedcnt - 1, array);
}

/**
 * Reads an entry.
 * @param[in] idx Index of entry being read (from 0 to array_size - 1).
 * @param[in] array Non-empty array.
 * @return Entry value.
 */
void*
piojo_array_at(size_t idx, const piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(idx < array->usedcnt);

        return &array->data[idx * array->esize];
}

/**
 * Reads first entry.
 * @param[in] array Non-empty array.
 * @return Entry value.
 */
void*
piojo_array_first(const piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(array->usedcnt > 0);

        return &array->data[0];
}

/**
 * Reads last entry.
 * @param[in] array Non-empty array.
 * @return Entry value.
 */
void*
piojo_array_last(const piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(array->usedcnt > 0);

        return &array->data[(array->usedcnt - 1) * array->esize];
}

/** @}
 * Private functions.
 */

static void
move_left_from(size_t idx, piojo_array_t *array)
{
        while (idx < array->usedcnt){
                size_t curidx = idx * array->esize;
                size_t nextidx = (idx + 1) * array->esize;
                memcpy(&array->data[curidx], &array->data[nextidx],
                       array->esize);
                ++idx;
        }
}

static void
move_right_until(size_t idx, piojo_array_t *array)
{
        size_t last = array->usedcnt;
        while (last > idx){
                size_t curidx = last * array->esize;
                size_t previdx = (last - 1) * array->esize;
                memcpy(&array->data[curidx], &array->data[previdx],
                       array->esize);
                --last;
        }
}

static void
expand_array(size_t incr_cnt, piojo_array_t *array)
{
        size_t newcnt, size;
        PIOJO_ASSERT(array->ecount < SIZE_MAX);

        newcnt = incr_cnt;
        PIOJO_ASSERT(piojo_safe_addsiz_p(array->ecount, newcnt));
        newcnt += array->ecount;

        PIOJO_ASSERT(piojo_safe_mulsiz_p(newcnt, array->esize));
        size = newcnt * array->esize;

        array->data = (uint8_t *)array->allocator.realloc_cb(array->data, size);
        PIOJO_ASSERT(array->data);

        array->ecount = newcnt;
}

static void
finish_all(const piojo_array_t *array)
{
        piojo_alloc_if allocator;
        size_t i;
        allocator = array->allocator;
        for (i = 0; i < array->usedcnt; ++i){
                allocator.finish_cb(piojo_array_at(i, array));
        }
}

static size_t*
entry_index(const void *data, piojo_cmp_cb cmp,
            const piojo_array_t *array, size_t *idx)
{
        int cmpval;
        bool found_p = FALSE;
        size_t imin, imax, mid, cnt;

        imin = 0;
        cnt = piojo_array_size(array);
        if (cnt > 0){
                /* Binary search. */
                imax = cnt - 1;
                while (imin <= imax){
                        mid = imin + ((imax - imin) / 2);
                        cmpval = cmp(data, piojo_array_at(mid, array));
                        if (cmpval == 0){
                                imin = mid;
                                found_p = TRUE;
                                break;
                        }else if (cmpval > 0){
                                imin = mid + 1;
                        }else if (imin != mid){
                                imax = mid - 1;
                        }else{
                                break;
                        }
                }
        }

        *idx = imin;
        return (found_p ? idx : NULL);
}
