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
 * @addtogroup piojoalloc Piojo Alloc
 * @{
 * Piojo Alloc/Free implementation.
 */

#include <piojo/piojo_alloc.h>
#include <piojo_defs.h>

/** @hideinitializer Default allocator */
piojo_alloc_if piojo_alloc_default = {
        piojo_alloc_def_alloc,
        piojo_alloc_def_realloc,
        piojo_alloc_def_free,
        piojo_alloc_def_init,
        piojo_alloc_def_copy,
        piojo_alloc_def_finish,
};

/** @hideinitializer Default key-value allocator */
piojo_alloc_kv_if piojo_alloc_kv_default = {
        piojo_alloc_def_alloc,
        piojo_alloc_def_realloc,
        piojo_alloc_def_free,
        piojo_alloc_def_init,
        piojo_alloc_def_copy,
        piojo_alloc_def_finish,
        piojo_alloc_def_init,
        piojo_alloc_def_copy,
        piojo_alloc_def_finish,
};

/**
 * Default alloc. Calls @c malloc().
 * @param[in] size Number of bytes to allocate.
 * @return Pointer to allocated memory.
 */
void* piojo_alloc_def_alloc(size_t size)
{
        return malloc(size);
}

/**
 * Default realloc. Calls @c realloc().
 * @param[in] ptr Pointer to allocated memory.
 * @param[in] size Number of bytes to reallocate.
 * @return Pointer to reallocated memory.
 */
void* piojo_alloc_def_realloc(const void *ptr, size_t size)
{
        return realloc((void *)ptr, size);
}

/**
 * Default free. Calls @c free().
 * @param[in] ptr Pointer to allocated memory.
 */
void piojo_alloc_def_free(const void *ptr)
{
        free((void *)ptr);
}

/**
 * Default init. Calls @c memcpy().
 * @param data Pointer to entry value.
 * @param[in] esize Entry value size in bytes.
 * @param[out] newptr Pointer to new entry.
 */
void piojo_alloc_def_init(const void *data, size_t esize, void *newptr)
{
        memcpy(newptr, data, esize);
}

/**
 * Default copy. Calls @c memcpy().
 * @param[in] ptr Pointer to entry value being copied.
 * @param[in] esize Entry value size in bytes.
 * @param[out] newptr Pointer to new entry.
 */
void piojo_alloc_def_copy(const void *ptr, size_t esize, void *newptr)
{
        memcpy(newptr, ptr, esize);
}

/**
 * Default finish. Does nothing.
 * @param[out] ptr Pointer to entry value being finished/deinitialized.
 */
void piojo_alloc_def_finish(void *ptr)
{
        PIOJO_UNUSED(ptr);
}

/** @} */
