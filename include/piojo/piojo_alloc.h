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
 *
 * Piojo Alloc/Free API.
 */

/**
 * @file
 * @addtogroup piojoalloc
 */

#ifndef PIOJO_ALLOC_H_
#define PIOJO_ALLOC_H_

#include <piojo/piojo.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @{ */

/** Allocator interface. */
typedef struct {
        /** Allocation callback. */
        piojo_alloc_cb alloc_cb;
        /** Free callback. */
        piojo_free_cb free_cb;
        /** Init callback. */
        piojo_init_cb init_cb;
        /** Copy callback. */
        piojo_copy_cb copy_cb;
        /** Finish callback. */
        piojo_finish_cb finish_cb;
} piojo_alloc_if;

/** Key-value allocator interface. */
typedef struct {
        /** Allocation callback. */
        piojo_alloc_cb alloc_cb;
        /** Free callback. */
        piojo_free_cb free_cb;
        /** Value init callback. */
        piojo_init_cb init_cb;
        /** Value copy callback. */
        piojo_copy_cb copy_cb;
        /** Value finish callback. */
        piojo_finish_cb finish_cb;
        /** Key init callback. */
        piojo_init_cb initk_cb;
        /** Key copy callback. */
        piojo_copy_cb copyk_cb;
        /** Key finish callback. */
        piojo_finish_cb finishk_cb;
} piojo_alloc_kv_if;

extern piojo_alloc_if piojo_alloc_default;
extern piojo_alloc_kv_if piojo_alloc_kv_default;

void* piojo_alloc_def_alloc(size_t size);
void piojo_alloc_def_free(const void *ptr);
void piojo_alloc_def_init(const void *data, size_t esize, void *newptr);
void piojo_alloc_def_copy(const void *ptr, size_t esize, void *newptr);
void piojo_alloc_def_finish(void *ptr);

/** @} */

#ifdef __cplusplus
}
#endif
#endif
