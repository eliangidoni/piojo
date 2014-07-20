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
 * Piojo Stream API.
 */

#ifndef PIOJO_STREAM_H_
#define PIOJO_STREAM_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_stream_t piojo_stream_t;
extern const size_t piojo_stream_sizeof;

piojo_stream_t*
piojo_stream_alloc(void);

piojo_stream_t*
piojo_stream_alloc_cb(piojo_alloc_if allocator);

piojo_stream_t*
piojo_stream_copy(const piojo_stream_t *stream);

void
piojo_stream_free(const piojo_stream_t *stream);

void
piojo_stream_clear(piojo_stream_t *stream);

void
piojo_stream_resize(size_t bytecnt, piojo_stream_t *stream);

size_t
piojo_stream_size(const piojo_stream_t *stream);

const uint8_t*
piojo_stream_data(const piojo_stream_t *stream);

void
piojo_stream_concat(const piojo_stream_t *stream, piojo_stream_t *to);

uint64_t
piojo_stream_readu64(piojo_stream_t *stream);

void
piojo_stream_writeu64(uint64_t val, piojo_stream_t *stream);

int64_t
piojo_stream_readi64(piojo_stream_t *stream);

void
piojo_stream_writei64(int64_t val, piojo_stream_t *stream);

uint32_t
piojo_stream_readu32(piojo_stream_t *stream);

void
piojo_stream_writeu32(uint32_t val, piojo_stream_t *stream);

int32_t
piojo_stream_readi32(piojo_stream_t *stream);

void
piojo_stream_writei32(int32_t val, piojo_stream_t *stream);

uint16_t
piojo_stream_readu16(piojo_stream_t *stream);

void
piojo_stream_writeu16(uint16_t val, piojo_stream_t *stream);

int16_t
piojo_stream_readi16(piojo_stream_t *stream);

void
piojo_stream_writei16(int16_t val, piojo_stream_t *stream);

uint8_t
piojo_stream_readu8(piojo_stream_t *stream);

void
piojo_stream_writeu8(uint8_t val, piojo_stream_t *stream);

int8_t
piojo_stream_readi8(piojo_stream_t *stream);

void
piojo_stream_writei8(int8_t val, piojo_stream_t *stream);

uint32_t
piojo_stream_readstr(uint8_t **str, piojo_stream_t *stream);

void
piojo_stream_writestr(const uint8_t *str, uint32_t len, piojo_stream_t *stream);

bool
piojo_stream_readbool(piojo_stream_t *stream);

void
piojo_stream_writebool(bool val, piojo_stream_t *stream);

#ifdef __cplusplus
}
#endif
#endif
