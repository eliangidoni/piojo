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

/** @} */
