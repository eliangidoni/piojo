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
 * @addtogroup piojobuffer Piojo Buffer
 * @{
 * Piojo Buffer implementation for binary data.
 * Assumes two's complement representation for signed numbers.
 * @warning Reads and writes shouldn't be mixed (unless buffer is new/clear).
 */

#include <piojo/piojo_buffer.h>
#include <piojo/piojo_array.h>
#include <piojo_defs.h>

struct piojo_buffer {
        piojo_array_t *data;
        size_t ridx;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of buffer in bytes */
const size_t piojo_buffer_sizeof = sizeof(piojo_buffer_t);

static void
be64encode(uint64_t val, piojo_buffer_t *buf);

static uint64_t
be64decode(const uint8_t *buf);

static void
be32encode(uint32_t val, piojo_buffer_t *buf);

static uint32_t
be32decode(const uint8_t *buf);

static void
be16encode(uint16_t val, piojo_buffer_t *buf);

static uint16_t
be16decode(const uint8_t *buf);

/**
 * Allocates a new buffer.
 * Uses default allocator.
 * @return New buffer.
 */
piojo_buffer_t*
piojo_buffer_alloc(void)
{
        return piojo_buffer_alloc_n(DEFAULT_ADT_ECOUNT);
}

/**
 * Allocates a new buffer.
 * Uses default allocator.
 * @param[in] count Number of reserved bytes in allocation.
 * @return New buffer.
 */
piojo_buffer_t*
piojo_buffer_alloc_n(size_t count)
{
        return piojo_buffer_alloc_cb_n(count, piojo_alloc_default);
}

/**
 * Allocates a new buffer.
 * @param[in] allocator Allocator to be used.
 * @return New buffer.
 */
piojo_buffer_t*
piojo_buffer_alloc_cb(piojo_alloc_if allocator)
{
        return piojo_buffer_alloc_cb_n(DEFAULT_ADT_ECOUNT, allocator);
}

/**
 * Allocates a new buffer.
 * @param[in] count Number of reserved bytes in allocation.
 * @param[in] allocator Allocator to be used.
 * @return New buffer.
 */
piojo_buffer_t*
piojo_buffer_alloc_cb_n(size_t count, piojo_alloc_if allocator)
{
        piojo_buffer_t * buf;
        buf = (piojo_buffer_t *) allocator.alloc_cb(sizeof(piojo_buffer_t));
        PIOJO_ASSERT(buf);

        buf->allocator = allocator;
        buf->ridx = 0;
        buf->data = piojo_array_alloc_cb_n(sizeof(uint8_t), count,
                                           buf->allocator);
        PIOJO_ASSERT(buf->data);

        return buf;
}

/**
 * Copies @a buffer.
 * @param[in] buffer Buffer being copied.
 * @return New buffer.
 */
piojo_buffer_t*
piojo_buffer_copy(const piojo_buffer_t *buffer)
{
        piojo_alloc_if allocator;
        piojo_buffer_t * newbuffer;
        PIOJO_ASSERT(buffer);

        allocator = buffer->allocator;
        newbuffer = (piojo_buffer_t*)allocator.alloc_cb(sizeof(piojo_buffer_t));
        PIOJO_ASSERT(newbuffer);

        newbuffer->allocator = allocator;
        newbuffer->ridx = buffer->ridx;
        newbuffer->data = piojo_array_copy(buffer->data);
        PIOJO_ASSERT(newbuffer->data);

        return newbuffer;
}

/**
 * Frees @a buffer.
 * @param[in] buffer Buffer being freed.
 */
void
piojo_buffer_free(const piojo_buffer_t *buffer)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(buffer);

        allocator = buffer->allocator;
        piojo_array_free(buffer->data);
        allocator.free_cb(buffer);
}

/**
 * Clears buffer write/read indices.
 * @param[out] buffer Buffer being cleared.
 */
void
piojo_buffer_clear(piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);

        piojo_array_clear(buffer->data);
        buffer->ridx = 0;
}

/**
 * Returns number of bytes written in @a buffer.
 * @param[in] buffer
 * @return Number of bytes written in @a buffer.
 */
size_t
piojo_buffer_size(const piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);
        return piojo_array_size(buffer->data);
}

/**
 * Concatenates two buffers.
 * @param[in] buf Source buffer.
 * @param[out] to Destination buffer.
 */
void
piojo_buffer_concat(const piojo_buffer_t *buf, piojo_buffer_t *to)
{
        uint32_t i, len;
        const uint8_t *str;
        PIOJO_ASSERT(buf);
        PIOJO_ASSERT(to);

        len = piojo_buffer_size(buf);
        str = piojo_buffer_data(buf);
        for (i = 0; i < len; ++i){
                piojo_buffer_writeu8((uint8_t)str[i], to);
        }
}

/**
 * Reads a @b uint64_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
uint64_t
piojo_buffer_readu64(piojo_buffer_t *buffer)
{
        uint64_t val;
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(sizeof(uint64_t) <=
                     piojo_buffer_size(buffer) - buffer->ridx);

        val = be64decode(piojo_array_at(buffer->ridx, buffer->data));
        buffer->ridx += sizeof(uint64_t);
        return val;
}

/**
 * Writes a @b uint64_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writeu64(uint64_t val, piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);
        be64encode(val, buffer);
}

/**
 * Reads a @b int64_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
int64_t
piojo_buffer_readi64(piojo_buffer_t *buffer)
{
        uint64_t tmp;
        PIOJO_ASSERT(buffer);

        tmp = piojo_buffer_readu64(buffer);
        return (int64_t) tmp;
}

/**
 * Writes a @b int64_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writei64(int64_t val, piojo_buffer_t *buffer)
{
        uint64_t tmp = (uint64_t) val;
        PIOJO_ASSERT(buffer);

        piojo_buffer_writeu64(tmp, buffer);
}

/**
 * Reads a @b uint32_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
uint32_t
piojo_buffer_readu32(piojo_buffer_t *buffer)
{
        uint32_t val;
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(sizeof(uint32_t) <=
                     piojo_buffer_size(buffer) - buffer->ridx);

        val = be32decode(piojo_array_at(buffer->ridx, buffer->data));
        buffer->ridx += sizeof(uint32_t);
        return val;
}

/**
 * Writes a @b uint32_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writeu32(uint32_t val, piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);
        be32encode(val, buffer);
}

/**
 * Reads a @b int32_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
int32_t
piojo_buffer_readi32(piojo_buffer_t *buffer)
{
        uint32_t tmp;
        PIOJO_ASSERT(buffer);

        tmp = piojo_buffer_readu32(buffer);
        return (int32_t) tmp;
}

/**
 * Writes a @b int32_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writei32(int32_t val, piojo_buffer_t *buffer)
{
        uint32_t tmp = (uint32_t) val;
        PIOJO_ASSERT(buffer);

        piojo_buffer_writeu32(tmp, buffer);
}

/**
 * Reads a @b uint16_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
uint16_t
piojo_buffer_readu16(piojo_buffer_t *buffer)
{
        uint16_t val;
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(sizeof(uint16_t) <=
                     piojo_buffer_size(buffer) - buffer->ridx);

        val = be16decode(piojo_array_at(buffer->ridx, buffer->data));
        buffer->ridx += sizeof(uint16_t);
        return val;
}

/**
 * Writes a @b uint16_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writeu16(uint16_t val, piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);
        be16encode(val, buffer);
}

/**
 * Reads a @b int16_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
int16_t
piojo_buffer_readi16(piojo_buffer_t *buffer)
{
        uint16_t tmp;
        PIOJO_ASSERT(buffer);

        tmp = piojo_buffer_readu16(buffer);
        return (int16_t) tmp;
}

/**
 * Writes a @b int16_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writei16(int16_t val, piojo_buffer_t *buffer)
{
        uint16_t tmp = (uint16_t) val;
        PIOJO_ASSERT(buffer);

        piojo_buffer_writeu16(tmp, buffer);
}

/**
 * Reads a @b uint8_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
uint8_t
piojo_buffer_readu8(piojo_buffer_t *buffer)
{
        uint8_t val;
        PIOJO_ASSERT(buffer);

        val = *(uint8_t*) piojo_array_at(buffer->ridx, buffer->data);
        buffer->ridx += sizeof(uint8_t);
        return val;
}

/**
 * Writes a @b uint8_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writeu8(uint8_t val, piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);
        piojo_array_push(&val, buffer->data);
}

/**
 * Reads a @b int8_t.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
int8_t
piojo_buffer_readi8(piojo_buffer_t *buffer)
{
        uint8_t tmp;
        PIOJO_ASSERT(buffer);

        tmp = piojo_buffer_readu8(buffer);
        return (int8_t) tmp;
}

/**
 * Writes a @b int8_t.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writei8(int8_t val, piojo_buffer_t *buffer)
{
        uint8_t tmp = (uint8_t) val;
        PIOJO_ASSERT(buffer);

        piojo_buffer_writeu8(tmp, buffer);
}

/**
 * Reads a string of bytes.
 * @param[out] str Pointer to string buffer.
 * @param[out] buffer Buffer being read.
 * @return Number of bytes read.
 */
uint32_t
piojo_buffer_readstr(uint8_t **str, piojo_buffer_t *buffer)
{
        uint32_t len;
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(str);

        len = piojo_buffer_readu32(buffer);
        *str = (uint8_t *) piojo_array_at(buffer->ridx, buffer->data);
        buffer->ridx += len;
        return len;
}

/**
 * Writes a string of bytes.
 * @param[in] str String of bytes.
 * @param[in] len Number of bytes.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writestr(const uint8_t *str, uint32_t len, piojo_buffer_t *buffer)
{
        uint32_t i;
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(str);

        piojo_buffer_writeu32(len, buffer);
        for (i = 0; i < len; ++i){
                piojo_buffer_writeu8((uint8_t)str[i], buffer);
        }
}

/**
 * Reads a @b bool.
 * @param[in] buffer Buffer being read.
 * @return Value read.
 */
bool
piojo_buffer_readbool(piojo_buffer_t *buffer)
{
        uint8_t tmp;
        PIOJO_ASSERT(buffer);

        tmp = piojo_buffer_readu8(buffer);
        return (tmp == 1 ? TRUE : FALSE);
}

/**
 * Writes a @b bool.
 * @param[in] val Value.
 * @param[out] buffer Buffer being written.
 */
void
piojo_buffer_writebool(bool val, piojo_buffer_t *buffer)
{
        uint8_t tmp = (val ? 1 : 0);
        PIOJO_ASSERT(buffer);

        piojo_buffer_writeu8(tmp, buffer);
}

/**
 * Returns pointer to buffer data.
 * @param[in] buffer Buffer.
 * @return Pointer to buffer data.
 */
const uint8_t*
piojo_buffer_data(const piojo_buffer_t *buffer)
{
        PIOJO_ASSERT(buffer);
        return (const uint8_t*) piojo_array_at(0, buffer->data);
}

/** @}
 * Private functions.
 */

static void
be64encode(uint64_t val, piojo_buffer_t *buf)
{
        piojo_buffer_writeu8((val >> 56) & 0xff, buf);
        piojo_buffer_writeu8((val >> 48) & 0xff, buf);
        piojo_buffer_writeu8((val >> 40) & 0xff, buf);
        piojo_buffer_writeu8((val >> 32) & 0xff, buf);
        piojo_buffer_writeu8((val >> 24) & 0xff, buf);
        piojo_buffer_writeu8((val >> 16) & 0xff, buf);
        piojo_buffer_writeu8((val >> 8)  & 0xff, buf);
        piojo_buffer_writeu8(val & 0xff, buf);
}

static uint64_t
be64decode(const uint8_t *buf)
{
        return ((uint64_t)(buf[7]<<0) | ((uint64_t)buf[6]<<8) |
                ((uint64_t)buf[5]<<16) | ((uint64_t)buf[4]<<24) |
                ((uint64_t)buf[3]<<32) | ((uint64_t)buf[2]<<40) |
                ((uint64_t)buf[1]<<48) | ((uint64_t)buf[0]<<56));
}

static void
be32encode(uint32_t val, piojo_buffer_t *buf)
{
        piojo_buffer_writeu8((val >> 24) & 0xff, buf);
        piojo_buffer_writeu8((val >> 16) & 0xff, buf);
        piojo_buffer_writeu8((val >> 8)  & 0xff, buf);
        piojo_buffer_writeu8(val & 0xff, buf);
}

static uint32_t
be32decode(const uint8_t *buf)
{
        return ((buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24));
}

static void
be16encode(uint16_t val, piojo_buffer_t *buf)
{
        piojo_buffer_writeu8((val >> 8)  & 0xff, buf);
        piojo_buffer_writeu8(val & 0xff, buf);
}

static uint16_t
be16decode(const uint8_t *buf)
{
        return ((buf[1]<<0) | (buf[0]<<8));
}
