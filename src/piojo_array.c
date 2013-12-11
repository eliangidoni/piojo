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
        piojo_alloc_t allocator;
};
/** @hideinitializer Size of array in bytes */
const size_t piojo_array_sizeof = sizeof(piojo_array_t);

static void
move_left_from(size_t idx, piojo_array_t *array);

static void
move_right_until(size_t idx, piojo_array_t *array);

static void
expand_array(piojo_array_t *array);

static void
finish_all(const piojo_array_t *array);

/**
 * Allocates a new array.
 * Uses default allocator and entry size of @b int.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc(void)
{
        return piojo_array_alloc_s(sizeof(int));
}

/**
 * Allocates a new array.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc_s(size_t esize)
{
        return piojo_array_alloc_n(esize, DEFAULT_ADT_ECOUNT);
}

/**
 * Allocates a new array.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc_n(size_t esize, size_t ecount)
{
        return piojo_array_alloc_cb_n(esize, ecount, piojo_alloc_default);
}

/**
 * Allocates a new array.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc_cb(size_t esize, piojo_alloc_t allocator)
{
        return piojo_array_alloc_cb_n(esize, DEFAULT_ADT_ECOUNT, allocator);
}

/**
 * Allocates a new array.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New array.
 */
piojo_array_t*
piojo_array_alloc_cb_n(size_t esize, size_t ecount, piojo_alloc_t allocator)
{
        piojo_array_t * arr;
        arr = (piojo_array_t *) allocator.alloc_cb(sizeof(piojo_array_t));
        PIOJO_ASSERT(arr);
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(ecount > 0);

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
        piojo_alloc_t allocator;
        PIOJO_ASSERT(array);

        allocator = array->allocator;
        esize = array->esize;

        newarray = piojo_array_alloc_cb_n(esize, array->ecount, allocator);
        PIOJO_ASSERT(newarray);
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
        piojo_alloc_t allocator;
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
                expand_array(array);
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
 * Inserts a new entry after the last entry.
 * @param[in] data Entry value.
 * @param[out] array Array being modified.
 */
void
piojo_array_append(const void *data, piojo_array_t *array)
{
        piojo_array_insert(piojo_array_size(array), data, array);
}

/**
 * Deletes an entry.
 * @param[in] idx Index of entry being deleted (from 0 to array_size - 1).
 * @param[out] array Non-empty array.
 * @return Next entry index.
 */
size_t
piojo_array_delete(size_t idx, piojo_array_t *array)
{
        PIOJO_ASSERT(array);
        PIOJO_ASSERT(idx < array->usedcnt);
        array->allocator.finish_cb(piojo_array_at(idx, array));
        --array->usedcnt;
        move_left_from(idx, array);
        return idx;
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
expand_array(piojo_array_t *array)
{
        size_t newcnt = array->ecount * DEFAULT_ADT_GROWTH_RATIO;
        size_t size = newcnt * array->esize;
        uint8_t *expanded = (uint8_t *) array->allocator.alloc_cb(size);
        PIOJO_ASSERT(expanded);

        memcpy(expanded, array->data, array->ecount * array->esize);
        array->allocator.free_cb(array->data);

        array->ecount = newcnt;
        array->data = expanded;
}

static void
finish_all(const piojo_array_t *array)
{
        piojo_alloc_t allocator;
        size_t i;
        allocator = array->allocator;
        for (i = 0; i < array->usedcnt; ++i){
                allocator.finish_cb(piojo_array_at(i, array));
        }
}
