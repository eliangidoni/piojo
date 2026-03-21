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
 */

#include <piojo_test.h>
#include <piojo/piojo_piecet.h>

void test_alloc(void)
{
        int data[] = {1, 2, 3, 4};
        int out[4] = {0, 0, 0, 0};
        piojo_piecet_t *piecet;

        piecet = piojo_piecet_alloc(sizeof(int));
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 0);
        piojo_piecet_free(piecet);

        piecet = piojo_piecet_alloc_cb(sizeof(int), my_allocator);
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 0);
        piojo_piecet_free(piecet);

        piecet = piojo_piecet_alloc_from(sizeof(int), data, 4);
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 4);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(data, out, sizeof(data)) == 0);
        piojo_piecet_free(piecet);
}

void test_insert_delete_read(void)
{
        int base[] = {1, 2, 3, 4};
        int ins[] = {9, 8};
        int out[6] = {0, 0, 0, 0, 0, 0};
        int expected1[] = {1, 2, 9, 8, 3, 4};
        int expected2[] = {1, 3, 4};
        piojo_piecet_t *piecet;

        piecet = piojo_piecet_alloc_from(sizeof(int), base, 4);
        piojo_piecet_insert(2, ins, 2, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 6);
        piojo_piecet_read(0, 6, out, piecet);
        PIOJO_ASSERT(memcmp(expected1, out, sizeof(expected1)) == 0);

        piojo_piecet_delete(1, 3, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 3);
        piojo_piecet_read(0, 3, out, piecet);
        PIOJO_ASSERT(memcmp(expected2, out, sizeof(expected2)) == 0);

        piojo_piecet_free(piecet);
}

void test_replace_copy_clear(void)
{
        int base[] = {1, 2, 3, 4, 5};
        int repl[] = {7, 7};
        int expected[] = {1, 7, 7, 5};
        int out[5] = {0, 0, 0, 0, 0};
        int *entry;
        piojo_piecet_t *piecet;
        piojo_piecet_t *copy;

        piecet = piojo_piecet_alloc_cb_from(sizeof(int), base, 5, my_allocator);
        piojo_piecet_replace(1, 3, repl, 2, piecet);

        PIOJO_ASSERT(piojo_piecet_size(piecet) == 4);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(expected, out, sizeof(expected)) == 0);

        entry = (int*) piojo_piecet_at(2, piecet);
        PIOJO_ASSERT(*entry == 7);

        copy = piojo_piecet_copy(piecet);
        PIOJO_ASSERT(copy);
        PIOJO_ASSERT(piojo_piecet_size(copy) == 4);
        piojo_piecet_flatten(copy, out);
        PIOJO_ASSERT(memcmp(expected, out, sizeof(expected)) == 0);

        piojo_piecet_clear(piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 5);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(base, out, sizeof(base)) == 0);

        piojo_piecet_free(copy);
        piojo_piecet_free(piecet);
}

void test_many_edits(void)
{
        piojo_piecet_t *piecet;
        int val;
        size_t i;

        piecet = piojo_piecet_alloc(sizeof(int));
        for (i = 0; i < 1000; ++i){
                val = (int) i;
                piojo_piecet_insert(piojo_piecet_size(piecet), &val, 1, piecet);
        }
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 1000);

        for (i = 0; i < 200; ++i){
                piojo_piecet_delete(100, 1, piecet);
        }
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 800);
        PIOJO_ASSERT(piojo_piecet_piece_count(piecet) > 0);

        piojo_piecet_free(piecet);
}

void test_snapshot_restore(void)
{
        int base[] = {1, 2, 3, 4};
        int ins[] = {9, 8};
        int tail = 7;
        int out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int expected_snap0[] = {1, 2, 3, 4};
        int expected_snap1[] = {1, 2, 9, 8, 3, 4};
        int expected_branch[] = {1, 2, 9, 8, 3, 4, 7};
        piojo_piecet_t *piecet;
        piojo_piecet_snapshot_t *snap0;
        piojo_piecet_snapshot_t *snap1;

        piecet = piojo_piecet_alloc_from(sizeof(int), base, 4);
        snap0 = piojo_piecet_snapshot(piecet);

        piojo_piecet_insert(2, ins, 2, piecet);
        snap1 = piojo_piecet_snapshot(piecet);

        piojo_piecet_delete(1, 3, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 3);

        piojo_piecet_restore(snap1, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 6);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(expected_snap1, out, sizeof(expected_snap1)) == 0);

        piojo_piecet_insert(piojo_piecet_size(piecet), &tail, 1, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 7);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(expected_branch, out, sizeof(expected_branch)) == 0);

        piojo_piecet_restore(snap0, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 4);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(expected_snap0, out, sizeof(expected_snap0)) == 0);

        piojo_piecet_snapshot_free(snap1);
        piojo_piecet_snapshot_free(snap0);
        piojo_piecet_free(piecet);
}

void test_snapshot_empty(void)
{
        int vals[] = {1, 2, 3};
        piojo_piecet_t *piecet;
        piojo_piecet_snapshot_t *snap;

        piecet = piojo_piecet_alloc(sizeof(int));
        snap = piojo_piecet_snapshot(piecet);

        piojo_piecet_insert(0, vals, 3, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 3);

        piojo_piecet_restore(snap, piecet);
        PIOJO_ASSERT(piojo_piecet_size(piecet) == 0);
        PIOJO_ASSERT(piojo_piecet_piece_count(piecet) == 0);

        piojo_piecet_snapshot_free(snap);
        piojo_piecet_free(piecet);
}

void test_snapshot_alloc_cb(void)
{
        int base[] = {4, 5, 6};
        int out[3] = {0, 0, 0};
        piojo_piecet_t *piecet;
        piojo_piecet_snapshot_t *snap;

        piecet = piojo_piecet_alloc_cb_from(sizeof(int), base, 3, my_allocator);
        snap = piojo_piecet_snapshot(piecet);

        piojo_piecet_delete(1, 2, piecet);
        piojo_piecet_restore(snap, piecet);
        piojo_piecet_flatten(piecet, out);
        PIOJO_ASSERT(memcmp(base, out, sizeof(base)) == 0);

        piojo_piecet_snapshot_free(snap);
        piojo_piecet_free(piecet);
}

int main(void)
{
        test_alloc();
        test_insert_delete_read();
        test_replace_copy_clear();
        test_many_edits();
        test_snapshot_restore();
        test_snapshot_empty();
        test_snapshot_alloc_cb();

        assert_allocator_init(0);
        assert_allocator_alloc(0);
        return 0;
}
