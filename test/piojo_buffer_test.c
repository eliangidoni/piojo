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
#include <piojo/piojo_buffer.h>


void test_buffer()
{
        piojo_buffer_t *buffer, *copy;
        char*str;
        int64_t x64=1;
        int32_t x32=1;
        int16_t x16=1;
        int8_t x8=1;

        buffer = piojo_buffer_alloc_cb(my_allocator);
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 0);

        piojo_buffer_writeu64(x64, buffer);
        piojo_buffer_writei64(x64 - 10, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 16);

        piojo_buffer_writeu32(x32, buffer);
        piojo_buffer_writei32(x32 - 10, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 24);

        piojo_buffer_writeu16(x16, buffer);
        piojo_buffer_writei16(x16 - 10, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 28);

        piojo_buffer_writeu8(x8, buffer);
        piojo_buffer_writei8(x8 - 10, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 30);

        piojo_buffer_writebool(TRUE, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 31);

        piojo_buffer_writestr("test", 5, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 40);

        copy = piojo_buffer_copy(buffer);
        PIOJO_ASSERT(piojo_buffer_size(copy) == 40);

        PIOJO_ASSERT(piojo_buffer_readu64(copy) == (uint64_t)x64);
        PIOJO_ASSERT(piojo_buffer_readi64(copy) == x64 - 10);

        PIOJO_ASSERT(piojo_buffer_readu32(copy) == (uint32_t)x32);
        PIOJO_ASSERT(piojo_buffer_readi32(copy) == x32 - 10);

        PIOJO_ASSERT(piojo_buffer_readu16(copy) == (uint16_t)x16);
        PIOJO_ASSERT(piojo_buffer_readi16(copy) == x16 - 10);

        PIOJO_ASSERT(piojo_buffer_readu8(copy) == (uint8_t)x8);
        PIOJO_ASSERT(piojo_buffer_readi8(copy) == x8 - 10);

        PIOJO_ASSERT(piojo_buffer_readbool(copy) == TRUE);

        PIOJO_ASSERT(piojo_buffer_readstr(&str, copy) == 5);
        PIOJO_ASSERT(strcmp(str, "test") == 0);

        piojo_buffer_clear(copy);
        piojo_buffer_free(copy);
        piojo_buffer_free(buffer);
}

void test_concat()
{
        piojo_buffer_t *buffer, *buffer2;
        char*str;
        int64_t x64=1;
        int32_t x32=1;
        int16_t x16=1;
        int8_t x8=1;

        buffer = piojo_buffer_alloc();
        PIOJO_ASSERT(buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 0);

        piojo_buffer_writeu64(x64, buffer);
        piojo_buffer_writei64(x64 - 10, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 16);

        piojo_buffer_writeu32(x32, buffer);
        piojo_buffer_writei32(x32 - 10, buffer);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 24);

        buffer2 = piojo_buffer_alloc();
        PIOJO_ASSERT(buffer2);
        PIOJO_ASSERT(piojo_buffer_size(buffer2) == 0);

        piojo_buffer_writeu16(x16, buffer2);
        piojo_buffer_writei16(x16 - 10, buffer2);
        PIOJO_ASSERT(piojo_buffer_size(buffer2) == 4);

        piojo_buffer_writeu8(x8, buffer2);
        piojo_buffer_writei8(x8 - 10, buffer2);
        PIOJO_ASSERT(piojo_buffer_size(buffer2) == 6);

        piojo_buffer_writebool(TRUE, buffer2);
        PIOJO_ASSERT(piojo_buffer_size(buffer2) == 7);

        piojo_buffer_writestr("test", 5, buffer2);
        PIOJO_ASSERT(piojo_buffer_size(buffer2) == 16);

        piojo_buffer_concat(buffer2, buffer);
        piojo_buffer_clear(buffer2);
        piojo_buffer_free(buffer2);
        PIOJO_ASSERT(piojo_buffer_size(buffer) == 40);

        PIOJO_ASSERT(piojo_buffer_readu64(buffer) == (uint64_t)x64);
        PIOJO_ASSERT(piojo_buffer_readi64(buffer) == x64 - 10);

        PIOJO_ASSERT(piojo_buffer_readu32(buffer) == (uint32_t)x32);
        PIOJO_ASSERT(piojo_buffer_readi32(buffer) == x32 - 10);

        PIOJO_ASSERT(piojo_buffer_readu16(buffer) == (uint16_t)x16);
        PIOJO_ASSERT(piojo_buffer_readi16(buffer) == x16 - 10);

        PIOJO_ASSERT(piojo_buffer_readu8(buffer) == (uint8_t)x8);
        PIOJO_ASSERT(piojo_buffer_readi8(buffer) == x8 - 10);

        PIOJO_ASSERT(piojo_buffer_readbool(buffer) == TRUE);

        PIOJO_ASSERT(piojo_buffer_readstr(&str, buffer) == 5);
        PIOJO_ASSERT(strcmp(str, "test") == 0);

        piojo_buffer_free(buffer);
}

int main()
{
        test_buffer();
        test_concat();

        assert_allocator_init(0);
        assert_allocator_alloc(0);
        return 0;
}
