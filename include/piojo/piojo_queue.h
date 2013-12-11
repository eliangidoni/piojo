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
 * Piojo Queue API.
 */

/**
 * @file
 * @addtogroup piojoqueue
 */

#ifndef PIOJO_QUEUE_H_
#define PIOJO_QUEUE_H_

#include <piojo/piojo_common.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_queue;
typedef struct piojo_queue piojo_queue_t;
extern const size_t piojo_queue_sizeof;

/** @{ */
/** Queue behavior when it's full. */
typedef enum {
        /** Expand queue dynamically when it's full. */
        PIOJO_QUEUE_DYN_TRUE,
        /** Don't expand it. */
        PIOJO_QUEUE_DYN_FALSE
} piojo_queue_dyn_t;
/** @} */

piojo_queue_t*
piojo_queue_alloc(piojo_queue_dyn_t dyn);

piojo_queue_t*
piojo_queue_alloc_s(piojo_queue_dyn_t dyn, size_t esize);

piojo_queue_t*
piojo_queue_alloc_n(piojo_queue_dyn_t dyn, size_t esize, size_t ecount);

piojo_queue_t*
piojo_queue_alloc_cb(piojo_queue_dyn_t dyn, size_t esize,
                     piojo_alloc_t allocator);

piojo_queue_t*
piojo_queue_alloc_cb_n(piojo_queue_dyn_t dyn, size_t esize, size_t ecount,
                       piojo_alloc_t allocator);

piojo_queue_t*
piojo_queue_copy(const piojo_queue_t *queue);

void
piojo_queue_free(const piojo_queue_t *queue);

void
piojo_queue_clear(piojo_queue_t *queue);

size_t
piojo_queue_size(const piojo_queue_t *queue);

bool
piojo_queue_full_p(const piojo_queue_t *queue);

void
piojo_queue_push(const void *data, piojo_queue_t *queue);

void
piojo_queue_pop(piojo_queue_t *queue);

void*
piojo_queue_peek(const piojo_queue_t *queue);

#ifdef __cplusplus
}
#endif
#endif
