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
 * Piojo private definitions.
 */

#ifndef PIOJO_DEFS_H_
#define PIOJO_DEFS_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include <piojo/piojo.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Note that this enforces a read when 'x' is volatile. */
#define PIOJO_UNUSED(x) (void)(x)

#define PIOJO_CHECK(cond, msg)                                          \
        do{                                                             \
                if (! (cond)){                                          \
                        fprintf(stderr, "Aborting %s:%u: " msg "\n",    \
                                __FILE__,__LINE__);                     \
                        abort();                                        \
                }                                                       \
        } while(0)

#ifndef PIOJO_DEBUG
#define PIOJO_ASSERT(cond) do{} while(0)
#define PIOJO_FAIL_IF(cond) do{} while(0)
#else
#define PIOJO_ASSERT(cond) do{ assert(cond); } while(0)
#define PIOJO_FAIL_IF(cond) do{ assert(! (cond)); } while(0)
#endif

static const uint32_t DEFAULT_ADT_ECOUNT = 128;
static const float DEFAULT_ADT_GROWTH_RATIO = 1.5f;

typedef enum {
        INSERT_NEW,
        INSERT_COPY,
        INSERT_PTR
} insert_t;

#ifdef __cplusplus
}
#endif
#endif
