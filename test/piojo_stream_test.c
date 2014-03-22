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
 */

#include <piojo_test.h>
#include <piojo/piojo_stream.h>


void test_stream()
{
        piojo_stream_t *stream, *copy;
        char*str;
        int64_t x64=1;
        int32_t x32=1;
        int16_t x16=1;
        int8_t x8=1;

        stream = piojo_stream_alloc_cb(my_allocator);
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 0);

        piojo_stream_writeu64(x64, stream);
        piojo_stream_writei64(x64 - 10, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 16);

        piojo_stream_writeu32(x32, stream);
        piojo_stream_writei32(x32 - 10, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 24);

        piojo_stream_writeu16(x16, stream);
        piojo_stream_writei16(x16 - 10, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 28);

        piojo_stream_writeu8(x8, stream);
        piojo_stream_writei8(x8 - 10, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 30);

        piojo_stream_writebool(TRUE, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 31);

        piojo_stream_writestr((uint8_t*)"test", 5, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 40);

        copy = piojo_stream_copy(stream);
        PIOJO_ASSERT(piojo_stream_size(copy) == 40);

        PIOJO_ASSERT(piojo_stream_readu64(copy) == (uint64_t)x64);
        PIOJO_ASSERT(piojo_stream_readi64(copy) == x64 - 10);

        PIOJO_ASSERT(piojo_stream_readu32(copy) == (uint32_t)x32);
        PIOJO_ASSERT(piojo_stream_readi32(copy) == x32 - 10);

        PIOJO_ASSERT(piojo_stream_readu16(copy) == (uint16_t)x16);
        PIOJO_ASSERT(piojo_stream_readi16(copy) == x16 - 10);

        PIOJO_ASSERT(piojo_stream_readu8(copy) == (uint8_t)x8);
        PIOJO_ASSERT(piojo_stream_readi8(copy) == x8 - 10);

        PIOJO_ASSERT(piojo_stream_readbool(copy) == TRUE);

        PIOJO_ASSERT(piojo_stream_readstr((uint8_t**)&str, copy) == 5);
        PIOJO_ASSERT(strcmp(str, "test") == 0);

        piojo_stream_clear(copy);
        piojo_stream_free(copy);
        piojo_stream_free(stream);
}

void test_concat()
{
        piojo_stream_t *stream, *stream2;
        char*str;
        int64_t x64=1;
        int32_t x32=1;
        int16_t x16=1;
        int8_t x8=1;

        stream = piojo_stream_alloc();
        PIOJO_ASSERT(stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 0);

        piojo_stream_writeu64(x64, stream);
        piojo_stream_writei64(x64 - 10, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 16);

        piojo_stream_writeu32(x32, stream);
        piojo_stream_writei32(x32 - 10, stream);
        PIOJO_ASSERT(piojo_stream_size(stream) == 24);

        stream2 = piojo_stream_alloc();
        PIOJO_ASSERT(stream2);
        PIOJO_ASSERT(piojo_stream_size(stream2) == 0);

        piojo_stream_writeu16(x16, stream2);
        piojo_stream_writei16(x16 - 10, stream2);
        PIOJO_ASSERT(piojo_stream_size(stream2) == 4);

        piojo_stream_writeu8(x8, stream2);
        piojo_stream_writei8(x8 - 10, stream2);
        PIOJO_ASSERT(piojo_stream_size(stream2) == 6);

        piojo_stream_writebool(TRUE, stream2);
        PIOJO_ASSERT(piojo_stream_size(stream2) == 7);

        piojo_stream_writestr((uint8_t*)"test", 5, stream2);
        PIOJO_ASSERT(piojo_stream_size(stream2) == 16);

        piojo_stream_concat(stream2, stream);
        piojo_stream_clear(stream2);
        piojo_stream_free(stream2);
        PIOJO_ASSERT(piojo_stream_size(stream) == 40);

        PIOJO_ASSERT(piojo_stream_readu64(stream) == (uint64_t)x64);
        PIOJO_ASSERT(piojo_stream_readi64(stream) == x64 - 10);

        PIOJO_ASSERT(piojo_stream_readu32(stream) == (uint32_t)x32);
        PIOJO_ASSERT(piojo_stream_readi32(stream) == x32 - 10);

        PIOJO_ASSERT(piojo_stream_readu16(stream) == (uint16_t)x16);
        PIOJO_ASSERT(piojo_stream_readi16(stream) == x16 - 10);

        PIOJO_ASSERT(piojo_stream_readu8(stream) == (uint8_t)x8);
        PIOJO_ASSERT(piojo_stream_readi8(stream) == x8 - 10);

        PIOJO_ASSERT(piojo_stream_readbool(stream) == TRUE);

        PIOJO_ASSERT(piojo_stream_readstr((uint8_t**)&str, stream) == 5);
        PIOJO_ASSERT(strcmp(str, "test") == 0);

        piojo_stream_free(stream);
}

int main()
{
        test_stream();
        test_concat();

        assert_allocator_init(0);
        assert_allocator_alloc(0);
        return 0;
}
