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
 * @addtogroup piojostream Piojo Stream
 * @{
 * Piojo Stream implementation for binary data.
 * Assumes two's complement representation for signed numbers.
 * @warning Reads and writes shouldn't be mixed (unless stream is new/clear).
 */

#include <piojo/piojo_stream.h>
#include <piojo/piojo_array.h>
#include <piojo_defs.h>

struct piojo_stream {
        piojo_array_t *data;
        size_t ridx;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of stream in bytes */
const size_t piojo_stream_sizeof = sizeof(piojo_stream_t);

static void
be64encode(uint64_t val, piojo_stream_t *buf);

static uint64_t
be64decode(const uint8_t *buf);

static void
be32encode(uint32_t val, piojo_stream_t *buf);

static uint32_t
be32decode(const uint8_t *buf);

static void
be16encode(uint16_t val, piojo_stream_t *buf);

static uint16_t
be16decode(const uint8_t *buf);

/**
 * Allocates a new stream.
 * Uses default allocator.
 * @return New stream.
 */
piojo_stream_t*
piojo_stream_alloc(void)
{
        return piojo_stream_alloc_cb(piojo_alloc_default);
}

/**
 * Allocates a new stream.
 * @param[in] allocator Allocator to be used.
 * @return New stream.
 */
piojo_stream_t*
piojo_stream_alloc_cb(piojo_alloc_if allocator)
{
        piojo_stream_t * buf;
        buf = (piojo_stream_t *) allocator.alloc_cb(sizeof(piojo_stream_t));
        PIOJO_ASSERT(buf);

        buf->allocator = allocator;
        buf->ridx = 0;
        buf->data = piojo_array_alloc_cb(sizeof(uint8_t), buf->allocator);
        PIOJO_ASSERT(buf->data);

        return buf;
}

/**
 * Copies @a stream.
 * @param[in] stream Stream being copied.
 * @return New stream.
 */
piojo_stream_t*
piojo_stream_copy(const piojo_stream_t *stream)
{
        piojo_alloc_if allocator;
        piojo_stream_t * newstream;
        PIOJO_ASSERT(stream);

        allocator = stream->allocator;
        newstream = (piojo_stream_t*)allocator.alloc_cb(sizeof(piojo_stream_t));
        PIOJO_ASSERT(newstream);

        newstream->allocator = allocator;
        newstream->ridx = stream->ridx;
        newstream->data = piojo_array_copy(stream->data);
        PIOJO_ASSERT(newstream->data);

        return newstream;
}

/**
 * Frees @a stream.
 * @param[in] stream Stream being freed.
 */
void
piojo_stream_free(const piojo_stream_t *stream)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(stream);

        allocator = stream->allocator;
        piojo_array_free(stream->data);
        allocator.free_cb(stream);
}

/**
 * Clears stream write/read indices.
 * @param[out] stream Stream being cleared.
 */
void
piojo_stream_clear(piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);

        piojo_array_clear(stream->data);
        stream->ridx = 0;
}

/**
 * Reserves memory for @a bytecnt bytes.
 * @param[in] bytecnt Number of bytes, must be equal or greater than
 *            the current size.
 * @param[out] stream Stream being modified.
 */
void
piojo_stream_reserve(size_t bytecnt, piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(bytecnt >= piojo_array_size(stream->data));

        piojo_array_reserve(bytecnt, stream->data);
}

/**
 * Returns number of bytes written in @a stream.
 * @param[in] stream
 * @return Number of bytes written in @a stream.
 */
size_t
piojo_stream_size(const piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);

        return piojo_array_size(stream->data);
}

/**
 * Concatenates two streams.
 * @param[in] stream Source stream.
 * @param[out] to Destination stream.
 */
void
piojo_stream_concat(const piojo_stream_t *stream, piojo_stream_t *to)
{
        uint32_t i, len;
        const uint8_t *str;
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(to);

        len = piojo_stream_size(stream);
        str = piojo_stream_data(stream);
        for (i = 0; i < len; ++i){
                piojo_stream_writeu8((uint8_t)str[i], to);
        }
}

/**
 * Reads a @b uint64_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
uint64_t
piojo_stream_readu64(piojo_stream_t *stream)
{
        uint64_t val;
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(sizeof(uint64_t) <=
                     piojo_stream_size(stream) - stream->ridx);

        val = be64decode(piojo_array_at(stream->ridx, stream->data));
        stream->ridx += sizeof(uint64_t);
        return val;
}

/**
 * Writes a @b uint64_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writeu64(uint64_t val, piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);
        be64encode(val, stream);
}

/**
 * Reads a @b int64_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
int64_t
piojo_stream_readi64(piojo_stream_t *stream)
{
        uint64_t tmp;
        PIOJO_ASSERT(stream);

        tmp = piojo_stream_readu64(stream);
        return (int64_t) tmp;
}

/**
 * Writes a @b int64_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writei64(int64_t val, piojo_stream_t *stream)
{
        uint64_t tmp = (uint64_t) val;
        PIOJO_ASSERT(stream);

        piojo_stream_writeu64(tmp, stream);
}

/**
 * Reads a @b uint32_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
uint32_t
piojo_stream_readu32(piojo_stream_t *stream)
{
        uint32_t val;
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(sizeof(uint32_t) <=
                     piojo_stream_size(stream) - stream->ridx);

        val = be32decode(piojo_array_at(stream->ridx, stream->data));
        stream->ridx += sizeof(uint32_t);
        return val;
}

/**
 * Writes a @b uint32_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writeu32(uint32_t val, piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);
        be32encode(val, stream);
}

/**
 * Reads a @b int32_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
int32_t
piojo_stream_readi32(piojo_stream_t *stream)
{
        uint32_t tmp;
        PIOJO_ASSERT(stream);

        tmp = piojo_stream_readu32(stream);
        return (int32_t) tmp;
}

/**
 * Writes a @b int32_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writei32(int32_t val, piojo_stream_t *stream)
{
        uint32_t tmp = (uint32_t) val;
        PIOJO_ASSERT(stream);

        piojo_stream_writeu32(tmp, stream);
}

/**
 * Reads a @b uint16_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
uint16_t
piojo_stream_readu16(piojo_stream_t *stream)
{
        uint16_t val;
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(sizeof(uint16_t) <=
                     piojo_stream_size(stream) - stream->ridx);

        val = be16decode(piojo_array_at(stream->ridx, stream->data));
        stream->ridx += sizeof(uint16_t);
        return val;
}

/**
 * Writes a @b uint16_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writeu16(uint16_t val, piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);
        be16encode(val, stream);
}

/**
 * Reads a @b int16_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
int16_t
piojo_stream_readi16(piojo_stream_t *stream)
{
        uint16_t tmp;
        PIOJO_ASSERT(stream);

        tmp = piojo_stream_readu16(stream);
        return (int16_t) tmp;
}

/**
 * Writes a @b int16_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writei16(int16_t val, piojo_stream_t *stream)
{
        uint16_t tmp = (uint16_t) val;
        PIOJO_ASSERT(stream);

        piojo_stream_writeu16(tmp, stream);
}

/**
 * Reads a @b uint8_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
uint8_t
piojo_stream_readu8(piojo_stream_t *stream)
{
        uint8_t val;
        PIOJO_ASSERT(stream);

        val = *(uint8_t*) piojo_array_at(stream->ridx, stream->data);
        stream->ridx += sizeof(uint8_t);
        return val;
}

/**
 * Writes a @b uint8_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writeu8(uint8_t val, piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);
        piojo_array_push(&val, stream->data);
}

/**
 * Reads a @b int8_t.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
int8_t
piojo_stream_readi8(piojo_stream_t *stream)
{
        uint8_t tmp;
        PIOJO_ASSERT(stream);

        tmp = piojo_stream_readu8(stream);
        return (int8_t) tmp;
}

/**
 * Writes a @b int8_t.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writei8(int8_t val, piojo_stream_t *stream)
{
        uint8_t tmp = (uint8_t) val;
        PIOJO_ASSERT(stream);

        piojo_stream_writeu8(tmp, stream);
}

/**
 * Reads a string of bytes.
 * @param[out] str Pointer to string stream.
 * @param[out] stream Stream being read.
 * @return Number of bytes read.
 */
uint32_t
piojo_stream_readstr(uint8_t **str, piojo_stream_t *stream)
{
        uint32_t len;
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(str);

        len = piojo_stream_readu32(stream);
        *str = (uint8_t *) piojo_array_at(stream->ridx, stream->data);
        stream->ridx += len;
        return len;
}

/**
 * Writes a string of bytes.
 * @param[in] str String of bytes.
 * @param[in] len Number of bytes.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writestr(const uint8_t *str, uint32_t len, piojo_stream_t *stream)
{
        uint32_t i;
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(str);

        piojo_stream_writeu32(len, stream);
        for (i = 0; i < len; ++i){
                piojo_stream_writeu8((uint8_t)str[i], stream);
        }
}

/**
 * Reads a @b bool.
 * @param[in] stream Stream being read.
 * @return Value read.
 */
bool
piojo_stream_readbool(piojo_stream_t *stream)
{
        uint8_t tmp;
        PIOJO_ASSERT(stream);

        tmp = piojo_stream_readu8(stream);
        return (tmp == 1 ? TRUE : FALSE);
}

/**
 * Writes a @b bool.
 * @param[in] val Value.
 * @param[out] stream Stream being written.
 */
void
piojo_stream_writebool(bool val, piojo_stream_t *stream)
{
        uint8_t tmp = (val ? 1 : 0);
        PIOJO_ASSERT(stream);

        piojo_stream_writeu8(tmp, stream);
}

/**
 * Returns pointer to stream data.
 * @param[in] stream Stream.
 * @return Pointer to stream data.
 */
const uint8_t*
piojo_stream_data(const piojo_stream_t *stream)
{
        PIOJO_ASSERT(stream);
        return (const uint8_t*) piojo_array_at(0, stream->data);
}

/** @}
 * Private functions.
 */

static void
be64encode(uint64_t val, piojo_stream_t *buf)
{
        piojo_stream_writeu8((val >> 56) & 0xff, buf);
        piojo_stream_writeu8((val >> 48) & 0xff, buf);
        piojo_stream_writeu8((val >> 40) & 0xff, buf);
        piojo_stream_writeu8((val >> 32) & 0xff, buf);
        piojo_stream_writeu8((val >> 24) & 0xff, buf);
        piojo_stream_writeu8((val >> 16) & 0xff, buf);
        piojo_stream_writeu8((val >> 8)  & 0xff, buf);
        piojo_stream_writeu8(val & 0xff, buf);
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
be32encode(uint32_t val, piojo_stream_t *buf)
{
        piojo_stream_writeu8((val >> 24) & 0xff, buf);
        piojo_stream_writeu8((val >> 16) & 0xff, buf);
        piojo_stream_writeu8((val >> 8)  & 0xff, buf);
        piojo_stream_writeu8(val & 0xff, buf);
}

static uint32_t
be32decode(const uint8_t *buf)
{
        return ((buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24));
}

static void
be16encode(uint16_t val, piojo_stream_t *buf)
{
        piojo_stream_writeu8((val >> 8)  & 0xff, buf);
        piojo_stream_writeu8(val & 0xff, buf);
}

static uint16_t
be16decode(const uint8_t *buf)
{
        return ((buf[1]<<0) | (buf[0]<<8));
}
