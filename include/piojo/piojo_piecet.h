/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 G. Elian Gidoni
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
 * Piojo Piece Table API.
 */

/**
 * @file
 * @addtogroup piojopiecet Piojo Piece Table
 */

#ifndef PIOJO_PIECET_H_
#define PIOJO_PIECET_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_piecet_t piojo_piecet_t;
typedef struct piojo_piecet_snapshot_t piojo_piecet_snapshot_t;
extern const size_t piojo_piecet_sizeof;

piojo_piecet_t*
piojo_piecet_alloc(size_t esize);

piojo_piecet_t*
piojo_piecet_alloc_cb(size_t esize, piojo_alloc_if allocator);

piojo_piecet_t*
piojo_piecet_alloc_from(size_t esize, const void *data, size_t count);

piojo_piecet_t*
piojo_piecet_alloc_cb_from(size_t esize, const void *data, size_t count,
                           piojo_alloc_if allocator);

piojo_piecet_t*
piojo_piecet_copy(const piojo_piecet_t *piecet);

void
piojo_piecet_free(const piojo_piecet_t *piecet);

void
piojo_piecet_clear(piojo_piecet_t *piecet);

size_t
piojo_piecet_size(const piojo_piecet_t *piecet);

size_t
piojo_piecet_piece_count(const piojo_piecet_t *piecet);

void
piojo_piecet_insert(size_t pos, const void *data, size_t count,
                    piojo_piecet_t *piecet);

void
piojo_piecet_delete(size_t pos, size_t count, piojo_piecet_t *piecet);

void
piojo_piecet_replace(size_t pos, size_t del_count,
                     const void *data, size_t ins_count,
                     piojo_piecet_t *piecet);

void
piojo_piecet_read(size_t pos, size_t count,
                  void *out, const piojo_piecet_t *piecet);

void
piojo_piecet_flatten(const piojo_piecet_t *piecet, void *out);

void*
piojo_piecet_at(size_t pos, const piojo_piecet_t *piecet);

piojo_piecet_snapshot_t*
piojo_piecet_snapshot(const piojo_piecet_t *piecet);

void
piojo_piecet_restore(const piojo_piecet_snapshot_t *snapshot,
                     piojo_piecet_t *piecet);

void
piojo_piecet_snapshot_free(const piojo_piecet_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif
#endif