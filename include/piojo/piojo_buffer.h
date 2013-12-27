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
 * Piojo Buffer API.
 */

#ifndef PIOJO_BUFFER_H_
#define PIOJO_BUFFER_H_

#include <piojo/piojo_common.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_buffer;
typedef struct piojo_buffer piojo_buffer_t;
extern const size_t piojo_buffer_sizeof;

piojo_buffer_t*
piojo_buffer_alloc(void);

piojo_buffer_t*
piojo_buffer_alloc_n(size_t count);

piojo_buffer_t*
piojo_buffer_alloc_cb(piojo_alloc_if allocator);

piojo_buffer_t*
piojo_buffer_alloc_cb_n(size_t count, piojo_alloc_if allocator);

piojo_buffer_t*
piojo_buffer_copy(const piojo_buffer_t *buffer);

void
piojo_buffer_free(const piojo_buffer_t *buffer);

void
piojo_buffer_clear(piojo_buffer_t *buffer);

size_t
piojo_buffer_size(const piojo_buffer_t *buffer);

const uint8_t*
piojo_buffer_data(const piojo_buffer_t *buffer);

void
piojo_buffer_concat(const piojo_buffer_t *buf, piojo_buffer_t *to);

uint64_t
piojo_buffer_readu64(piojo_buffer_t *buffer);

void
piojo_buffer_writeu64(uint64_t val, piojo_buffer_t *buffer);

int64_t
piojo_buffer_readi64(piojo_buffer_t *buffer);

void
piojo_buffer_writei64(int64_t val, piojo_buffer_t *buffer);

uint32_t
piojo_buffer_readu32(piojo_buffer_t *buffer);

void
piojo_buffer_writeu32(uint32_t val, piojo_buffer_t *buffer);

int32_t
piojo_buffer_readi32(piojo_buffer_t *buffer);

void
piojo_buffer_writei32(int32_t val, piojo_buffer_t *buffer);

uint16_t
piojo_buffer_readu16(piojo_buffer_t *buffer);

void
piojo_buffer_writeu16(uint16_t val, piojo_buffer_t *buffer);

int16_t
piojo_buffer_readi16(piojo_buffer_t *buffer);

void
piojo_buffer_writei16(int16_t val, piojo_buffer_t *buffer);

uint8_t
piojo_buffer_readu8(piojo_buffer_t *buffer);

void
piojo_buffer_writeu8(uint8_t val, piojo_buffer_t *buffer);

int8_t
piojo_buffer_readi8(piojo_buffer_t *buffer);

void
piojo_buffer_writei8(int8_t val, piojo_buffer_t *buffer);

uint32_t
piojo_buffer_readstr(uint8_t **str, piojo_buffer_t *buffer);

void
piojo_buffer_writestr(const uint8_t *str, uint32_t len, piojo_buffer_t *buffer);

bool
piojo_buffer_readbool(piojo_buffer_t *buffer);

void
piojo_buffer_writebool(bool val, piojo_buffer_t *buffer);

#ifdef __cplusplus
}
#endif
#endif
