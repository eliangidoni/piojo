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
 */

/**
 * @file
 * @addtogroup piojopiecet Piojo Piece Table
 * @{
 * Piojo Piece Table implementation.
 */

#include <piojo/piojo_piecet.h>
#include <piojo_defs.h>

typedef enum {
        SOURCE_ORIG,
        SOURCE_ADD
} source_t;

typedef struct {
        source_t source;
        size_t offset;
        size_t count;
} piece_t;

struct piojo_piecet_t {
        uint8_t *orig_data;
        size_t orig_count;

        uint8_t *add_data;
        size_t add_count;
        size_t add_cap;

        piece_t *pieces;
        size_t piece_count;
        size_t piece_cap;

        size_t esize;
        size_t total_count;
        piojo_alloc_if allocator;
};

struct piojo_piecet_snapshot_t {
        piece_t *pieces;
        size_t piece_count;
        size_t total_count;
        size_t esize;
        size_t orig_count;
        size_t add_count;
        piojo_alloc_if allocator;
};

/** @hideinitializer Size of piece table in bytes */
const size_t piojo_piecet_sizeof = sizeof(piojo_piecet_t);

static const size_t INITIAL_PIECE_CAP = 8;
static const size_t INITIAL_ADD_CAP = 128;

static void
ensure_piece_cap(size_t min_cap, piojo_piecet_t *piecet);

static void
ensure_add_cap(size_t min_cap, piojo_piecet_t *piecet);

static void
insert_piece_at(size_t idx, piece_t piece, piojo_piecet_t *piecet);

static void
normalize_pieces(piojo_piecet_t *piecet);

static void
locate_pos(size_t pos, const piojo_piecet_t *piecet, size_t *idx,
           size_t *off_in_piece);

static uint8_t*
piece_ptr(const piece_t *piece, const piojo_piecet_t *piecet);

/**
 * Allocates a new piece table.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @return New piece table.
 */
piojo_piecet_t*
piojo_piecet_alloc(size_t esize)
{
        return piojo_piecet_alloc_cb(esize, piojo_alloc_default);
}

/**
 * Allocates a new piece table.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New piece table.
 */
piojo_piecet_t*
piojo_piecet_alloc_cb(size_t esize, piojo_alloc_if allocator)
{
        piojo_piecet_t *piecet;
        PIOJO_ASSERT(esize > 0);

        piecet = (piojo_piecet_t*) allocator.alloc_cb(sizeof(piojo_piecet_t));
        PIOJO_ASSERT(piecet);

        piecet->orig_data = NULL;
        piecet->orig_count = 0;
        piecet->add_data = NULL;
        piecet->add_count = 0;
        piecet->add_cap = 0;
        piecet->piece_count = 0;
        piecet->piece_cap = 0;
        piecet->esize = esize;
        piecet->total_count = 0;
        piecet->allocator = allocator;
        piecet->pieces = NULL;

        ensure_piece_cap(INITIAL_PIECE_CAP, piecet);
        return piecet;
}

/**
 * Allocates a new piece table initialized from existing data.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @param[in] data Initial contiguous data.
 * @param[in] count Number of entries in @a data.
 * @return New piece table.
 */
piojo_piecet_t*
piojo_piecet_alloc_from(size_t esize, const void *data, size_t count)
{
        return piojo_piecet_alloc_cb_from(esize, data, count,
                                          piojo_alloc_default);
}

/**
 * Allocates a new piece table initialized from existing data.
 * @param[in] esize Entry size in bytes.
 * @param[in] data Initial contiguous data.
 * @param[in] count Number of entries in @a data.
 * @param[in] allocator Allocator to be used.
 * @return New piece table.
 */
piojo_piecet_t*
piojo_piecet_alloc_cb_from(size_t esize, const void *data, size_t count,
                           piojo_alloc_if allocator)
{
        piojo_piecet_t *piecet;
        PIOJO_ASSERT((count == 0) || data);

        piecet = piojo_piecet_alloc_cb(esize, allocator);
        PIOJO_ASSERT(piecet);

        if (count > 0){
                PIOJO_ASSERT(piojo_safe_mulsiz_p(count, esize));
                piecet->orig_data = (uint8_t*) allocator.alloc_cb(count * esize);
                PIOJO_ASSERT(piecet->orig_data);

                memcpy(piecet->orig_data, data, count * esize);
                piecet->orig_count = count;
                piecet->total_count = count;

                piecet->pieces[0].source = SOURCE_ORIG;
                piecet->pieces[0].offset = 0;
                piecet->pieces[0].count = count;
                piecet->piece_count = 1;
        }

        return piecet;
}

/**
 * Copies @a piecet and all of its data sources and pieces.
 * @param[in] piecet Piece table being copied.
 * @return New piece table.
 */
piojo_piecet_t*
piojo_piecet_copy(const piojo_piecet_t *piecet)
{
        piojo_piecet_t *newpiecet;
        PIOJO_ASSERT(piecet);

        newpiecet = piojo_piecet_alloc_cb(piecet->esize, piecet->allocator);
        PIOJO_ASSERT(newpiecet);

        if (piecet->orig_count > 0){
                PIOJO_ASSERT(piojo_safe_mulsiz_p(piecet->orig_count,
                                                piecet->esize));
                newpiecet->orig_data =
                        (uint8_t*) newpiecet->allocator.alloc_cb(piecet->orig_count
                                                                 * piecet->esize);
                PIOJO_ASSERT(newpiecet->orig_data);
                memcpy(newpiecet->orig_data, piecet->orig_data,
                       piecet->orig_count * piecet->esize);
                newpiecet->orig_count = piecet->orig_count;
        }

        if (piecet->add_count > 0){
                ensure_add_cap(piecet->add_count, newpiecet);
                memcpy(newpiecet->add_data, piecet->add_data,
                       piecet->add_count * piecet->esize);
                newpiecet->add_count = piecet->add_count;
        }

        ensure_piece_cap(piecet->piece_count, newpiecet);
        memcpy(newpiecet->pieces, piecet->pieces,
               piecet->piece_count * sizeof(piece_t));
        newpiecet->piece_count = piecet->piece_count;
        newpiecet->total_count = piecet->total_count;

        return newpiecet;
}

/**
 * Creates snapshot of current logical composition.
 * @param[in] piecet Piece table being snapshotted.
 * @return New snapshot.
 */
piojo_piecet_snapshot_t*
piojo_piecet_snapshot(const piojo_piecet_t *piecet)
{
        piojo_piecet_snapshot_t *snapshot;
        PIOJO_ASSERT(piecet);

        snapshot = (piojo_piecet_snapshot_t*) piecet->allocator.alloc_cb(
                sizeof(piojo_piecet_snapshot_t));
        PIOJO_ASSERT(snapshot);

        snapshot->pieces = NULL;
        snapshot->piece_count = piecet->piece_count;
        snapshot->total_count = piecet->total_count;
        snapshot->esize = piecet->esize;
        snapshot->orig_count = piecet->orig_count;
        snapshot->add_count = piecet->add_count;
        snapshot->allocator = piecet->allocator;

        if (piecet->piece_count > 0){
                PIOJO_ASSERT(piojo_safe_mulsiz_p(piecet->piece_count,
                                                sizeof(piece_t)));
                snapshot->pieces = (piece_t*) piecet->allocator.alloc_cb(
                        piecet->piece_count * sizeof(piece_t));
                PIOJO_ASSERT(snapshot->pieces);

                memcpy(snapshot->pieces, piecet->pieces,
                       piecet->piece_count * sizeof(piece_t));
        }

        return snapshot;
}

/**
 * Restores logical composition from snapshot.
 * @param[in] snapshot Snapshot being restored.
 * @param[out] piecet Piece table being modified.
 */
void
piojo_piecet_restore(const piojo_piecet_snapshot_t *snapshot,
                     piojo_piecet_t *piecet)
{
        size_t i;
        size_t restored_total;
        PIOJO_ASSERT(snapshot);
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(snapshot->esize == piecet->esize);
        PIOJO_ASSERT(snapshot->orig_count == piecet->orig_count);
        PIOJO_ASSERT(snapshot->add_count <= piecet->add_count);

        ensure_piece_cap(snapshot->piece_count, piecet);

        restored_total = 0;
        for (i = 0; i < snapshot->piece_count; ++i){
                piece_t piece;

                piece = snapshot->pieces[i];
                PIOJO_ASSERT(piojo_safe_addsiz_p(restored_total, piece.count));
                restored_total += piece.count;

                if (piece.source == SOURCE_ORIG){
                        PIOJO_ASSERT(piojo_safe_addsiz_p(piece.offset,
                                                        piece.count));
                        PIOJO_ASSERT(piece.offset + piece.count
                                     <= piecet->orig_count);
                }else{
                        PIOJO_ASSERT(piece.source == SOURCE_ADD);
                        PIOJO_ASSERT(piojo_safe_addsiz_p(piece.offset,
                                                        piece.count));
                        PIOJO_ASSERT(piece.offset + piece.count
                                     <= piecet->add_count);
                }

                piecet->pieces[i] = piece;
        }

        PIOJO_ASSERT(restored_total == snapshot->total_count);
        piecet->piece_count = snapshot->piece_count;
        piecet->total_count = snapshot->total_count;
}

/**
 * Frees snapshot allocated by piojo_piecet_snapshot.
 * @param[in] snapshot Snapshot being freed.
 */
void
piojo_piecet_snapshot_free(const piojo_piecet_snapshot_t *snapshot)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(snapshot);

        allocator = snapshot->allocator;
        if (snapshot->pieces){
                allocator.free_cb(snapshot->pieces);
        }
        allocator.free_cb(snapshot);
}

/**
 * Frees @a piecet and all its owned buffers.
 * @param[in] piecet Piece table being freed.
 */
void
piojo_piecet_free(const piojo_piecet_t *piecet)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(piecet);

        allocator = piecet->allocator;
        if (piecet->orig_data){
                allocator.free_cb(piecet->orig_data);
        }
        if (piecet->add_data){
                allocator.free_cb(piecet->add_data);
        }
        if (piecet->pieces){
                allocator.free_cb(piecet->pieces);
        }
        allocator.free_cb(piecet);
}

/**
 * Clears all edits and restores original content view.
 * @param[out] piecet Piece table being cleared.
 * @warning All existing snapshots must be freed before calling clear();
 *          restoring a snapshot taken before clear() is undefined behaviour.
 */
void
piojo_piecet_clear(piojo_piecet_t *piecet)
{
        PIOJO_ASSERT(piecet);

        piecet->add_count = 0;
        piecet->total_count = piecet->orig_count;
        if (piecet->orig_count > 0){
                piecet->pieces[0].source = SOURCE_ORIG;
                piecet->pieces[0].offset = 0;
                piecet->pieces[0].count = piecet->orig_count;
                piecet->piece_count = 1;
        }else{
                piecet->piece_count = 0;
        }
}

/**
 * Returns number of entries in current logical content.
 * @param[in] piecet
 * @return Number of entries in @a piecet.
 */
size_t
piojo_piecet_size(const piojo_piecet_t *piecet)
{
        PIOJO_ASSERT(piecet);
        return piecet->total_count;
}

/**
 * Returns number of pieces currently tracked.
 * @param[in] piecet
 * @return Number of pieces in @a piecet.
 */
size_t
piojo_piecet_piece_count(const piojo_piecet_t *piecet)
{
        PIOJO_ASSERT(piecet);
        return piecet->piece_count;
}

/**
 * Inserts entries at position @a pos.
 * @param[in] pos Insertion position.
 * @param[in] data Entries to insert.
 * @param[in] count Number of entries to insert.
 * @param[out] piecet Piece table being modified.
 */
void
piojo_piecet_insert(size_t pos, const void *data, size_t count,
                    piojo_piecet_t *piecet)
{
        piece_t new_piece;
        size_t idx, off;
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(pos <= piecet->total_count);
        PIOJO_ASSERT((count == 0) || data);

        if (count == 0){
                return;
        }

        PIOJO_ASSERT(piojo_safe_addsiz_p(piecet->total_count, count));
        PIOJO_ASSERT(piojo_safe_addsiz_p(piecet->add_count, count));

        ensure_add_cap(piecet->add_count + count, piecet);
        memcpy(piecet->add_data + (piecet->add_count * piecet->esize),
               data, count * piecet->esize);

        new_piece.source = SOURCE_ADD;
        new_piece.offset = piecet->add_count;
        new_piece.count = count;
        piecet->add_count += count;

        locate_pos(pos, piecet, &idx, &off);
        if (idx >= piecet->piece_count){
                insert_piece_at(piecet->piece_count, new_piece, piecet);
        }else if (off == 0){
                insert_piece_at(idx, new_piece, piecet);
        }else{
                piece_t right_piece;
                PIOJO_ASSERT(off < piecet->pieces[idx].count);
                PIOJO_ASSERT(piojo_safe_addsiz_p(piecet->pieces[idx].offset,
                                                off));

                right_piece.source = piecet->pieces[idx].source;
                right_piece.offset = piecet->pieces[idx].offset + off;
                right_piece.count = piecet->pieces[idx].count - off;
                piecet->pieces[idx].count = off;

                insert_piece_at(idx + 1, right_piece, piecet);
                insert_piece_at(idx + 1, new_piece, piecet);
        }

        piecet->total_count += count;
        normalize_pieces(piecet);
}

/**
 * Deletes @a count entries starting at @a pos.
 * @param[in] pos Deletion start position.
 * @param[in] count Number of entries to delete.
 * @param[out] piecet Piece table being modified.
 */
void
piojo_piecet_delete(size_t pos, size_t count, piojo_piecet_t *piecet)
{
        piece_t *new_pieces;
        size_t new_piece_cap;
        size_t new_piece_count;
        size_t i;
        size_t cursor;
        size_t del_end;
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(pos <= piecet->total_count);
        PIOJO_ASSERT(piojo_safe_addsiz_p(pos, count));
        PIOJO_ASSERT(pos + count <= piecet->total_count);

        if (count == 0){
                return;
        }

        del_end = pos + count;
        new_piece_cap = piojo_maxsiz(1, piecet->piece_count + 2);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(new_piece_cap, sizeof(piece_t)));
        new_pieces = (piece_t*) piecet->allocator.alloc_cb(new_piece_cap
                                                           * sizeof(piece_t));
        PIOJO_ASSERT(new_pieces);

        new_piece_count = 0;
        cursor = 0;
        for (i = 0; i < piecet->piece_count; ++i){
                piece_t piece;
                size_t seg_start;
                size_t seg_end;

                piece = piecet->pieces[i];
                seg_start = cursor;
                seg_end = cursor + piece.count;

                if ((seg_end <= pos) || (seg_start >= del_end)){
                        new_pieces[new_piece_count++] = piece;
                }else{
                        size_t left_count;
                        size_t right_count;

                        left_count = (pos > seg_start) ? pos - seg_start : 0;
                        right_count = (seg_end > del_end) ? seg_end - del_end : 0;

                        if (left_count > 0){
                                new_pieces[new_piece_count].source = piece.source;
                                new_pieces[new_piece_count].offset = piece.offset;
                                new_pieces[new_piece_count].count = left_count;
                                ++new_piece_count;
                        }
                        if (right_count > 0){
                                PIOJO_ASSERT(piojo_safe_addsiz_p(
                                                     piece.offset,
                                                     piece.count - right_count));
                                new_pieces[new_piece_count].source = piece.source;
                                new_pieces[new_piece_count].offset =
                                        piece.offset + (piece.count - right_count);
                                new_pieces[new_piece_count].count = right_count;
                                ++new_piece_count;
                        }
                }

                cursor = seg_end;
        }

        piecet->allocator.free_cb(piecet->pieces);
        piecet->pieces = new_pieces;
        piecet->piece_cap = new_piece_cap;
        piecet->piece_count = new_piece_count;
        piecet->total_count -= count;

        normalize_pieces(piecet);
}

/**
 * Replaces a range with new entries.
 * Equivalent to delete then insert at the same position.
 * @param[in] pos Replacement start position.
 * @param[in] del_count Number of entries to delete.
 * @param[in] data Entries to insert.
 * @param[in] ins_count Number of entries to insert.
 * @param[out] piecet Piece table being modified.
 */
void
piojo_piecet_replace(size_t pos, size_t del_count,
                     const void *data, size_t ins_count,
                     piojo_piecet_t *piecet)
{
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT((ins_count == 0) || data);
        PIOJO_ASSERT(piojo_safe_addsiz_p(pos, del_count));
        PIOJO_ASSERT(pos + del_count <= piecet->total_count);

        if (del_count > 0){
                piojo_piecet_delete(pos, del_count, piecet);
        }
        if (ins_count > 0){
                piojo_piecet_insert(pos, data, ins_count, piecet);
        }
}

/**
 * Reads a logical range into contiguous output buffer.
 * @param[in] pos Read start position.
 * @param[in] count Number of entries to read.
 * @param[out] out Output buffer with room for @a count entries.
 * @param[in] piecet Piece table being read.
 */
void
piojo_piecet_read(size_t pos, size_t count,
                  void *out, const piojo_piecet_t *piecet)
{
        uint8_t *out8;
        size_t i;
        size_t cursor;
        size_t copied;
        size_t end_pos;
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT((count == 0) || out);
        PIOJO_ASSERT(piojo_safe_addsiz_p(pos, count));
        PIOJO_ASSERT(pos + count <= piecet->total_count);

        if (count == 0){
                return;
        }

        out8 = (uint8_t*) out;
        copied = 0;
        end_pos = pos + count;
        cursor = 0;
        for (i = 0; i < piecet->piece_count; ++i){
                piece_t piece;
                size_t seg_start;
                size_t seg_end;

                piece = piecet->pieces[i];
                seg_start = cursor;
                seg_end = cursor + piece.count;

                if ((seg_end > pos) && (seg_start < end_pos)){
                        size_t take_start;
                        size_t take_end;
                        size_t in_piece_off;
                        size_t take_count;

                        take_start = piojo_maxsiz(seg_start, pos);
                        take_end = piojo_minsiz(seg_end, end_pos);
                        in_piece_off = take_start - seg_start;
                        take_count = take_end - take_start;

                        memcpy(out8 + (copied * piecet->esize),
                               piece_ptr(&piece, piecet)
                               + ((in_piece_off) * piecet->esize),
                               take_count * piecet->esize);
                        copied += take_count;
                }

                cursor = seg_end;
                if (copied == count){
                        break;
                }
        }
}

/**
 * Flattens full logical content into contiguous memory.
 * @param[in] piecet Piece table being read.
 * @param[out] out Output buffer with room for piojo_piecet_size entries.
 */
void
piojo_piecet_flatten(const piojo_piecet_t *piecet, void *out)
{
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT((piecet->total_count == 0) || out);

        if (piecet->total_count == 0){
                return;
        }

        piojo_piecet_read(0, piecet->total_count, out, piecet);
}

/**
 * Returns direct pointer to entry at logical position @a pos.
 * @warning Pointer is invalidated by any subsequent insert or replace call,
 *          prefer piojo_piecet_read().
 * @param[in] pos Entry position.
 * @param[in] piecet Piece table being accessed.
 * @return Pointer to entry at @a pos.
 */
void*
piojo_piecet_at(size_t pos, const piojo_piecet_t *piecet)
{
        size_t idx, off;
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(pos < piecet->total_count);

        locate_pos(pos, piecet, &idx, &off);
        PIOJO_ASSERT(idx < piecet->piece_count);
        return piece_ptr(&piecet->pieces[idx], piecet) + (off * piecet->esize);
}

/** @}
 * Private functions.
 */

static void
ensure_piece_cap(size_t min_cap, piojo_piecet_t *piecet)
{
        size_t new_cap;
        PIOJO_ASSERT(piecet);

        if (min_cap <= piecet->piece_cap){
                return;
        }

        new_cap = (piecet->piece_cap > 0) ? piecet->piece_cap : INITIAL_PIECE_CAP;
        while (new_cap < min_cap){
                PIOJO_ASSERT(piojo_safe_addsiz_p(new_cap, new_cap));
                new_cap += new_cap;
        }

        PIOJO_ASSERT(piojo_safe_mulsiz_p(new_cap, sizeof(piece_t)));
        piecet->pieces = (piece_t*) piecet->allocator.realloc_cb(
                piecet->pieces, new_cap * sizeof(piece_t));
        PIOJO_ASSERT(piecet->pieces);
        piecet->piece_cap = new_cap;
}

static void
ensure_add_cap(size_t min_cap, piojo_piecet_t *piecet)
{
        size_t new_cap;
        PIOJO_ASSERT(piecet);

        if (min_cap <= piecet->add_cap){
                return;
        }

        new_cap = (piecet->add_cap > 0) ? piecet->add_cap : INITIAL_ADD_CAP;
        while (new_cap < min_cap){
                PIOJO_ASSERT(piojo_safe_addsiz_p(new_cap, new_cap));
                new_cap += new_cap;
        }

        PIOJO_ASSERT(piojo_safe_mulsiz_p(new_cap, piecet->esize));
        piecet->add_data = (uint8_t*) piecet->allocator.realloc_cb(
                piecet->add_data, new_cap * piecet->esize);
        PIOJO_ASSERT(piecet->add_data);
        piecet->add_cap = new_cap;
}

static void
insert_piece_at(size_t idx, piece_t piece, piojo_piecet_t *piecet)
{
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(idx <= piecet->piece_count);

        ensure_piece_cap(piecet->piece_count + 1, piecet);
        if (idx < piecet->piece_count){
                memmove(&piecet->pieces[idx + 1], &piecet->pieces[idx],
                        (piecet->piece_count - idx) * sizeof(piece_t));
        }
        piecet->pieces[idx] = piece;
        ++piecet->piece_count;
}

static void
normalize_pieces(piojo_piecet_t *piecet)
{
        size_t i;
        size_t w;
        PIOJO_ASSERT(piecet);

        w = 0;
        for (i = 0; i < piecet->piece_count; ++i){
                piece_t piece;

                piece = piecet->pieces[i];
                if (piece.count == 0){
                        continue;
                }

                if (w > 0){
                        PIOJO_ASSERT(piojo_safe_addsiz_p(
                                             piecet->pieces[w - 1].offset,
                                             piecet->pieces[w - 1].count));
                }

                if ((w > 0)
                    && (piecet->pieces[w - 1].source == piece.source)
                    && (piecet->pieces[w - 1].offset + piecet->pieces[w - 1].count
                        == piece.offset)){
                        piecet->pieces[w - 1].count += piece.count;
                }else{
                        piecet->pieces[w++] = piece;
                }
        }
        piecet->piece_count = w;
}

static void
locate_pos(size_t pos, const piojo_piecet_t *piecet, size_t *idx,
           size_t *off_in_piece)
{
        size_t i;
        size_t cursor;
        PIOJO_ASSERT(piecet);
        PIOJO_ASSERT(idx);
        PIOJO_ASSERT(off_in_piece);
        PIOJO_ASSERT(pos <= piecet->total_count);

        cursor = 0;
        for (i = 0; i < piecet->piece_count; ++i){
                size_t next;
                PIOJO_ASSERT(piojo_safe_addsiz_p(cursor, piecet->pieces[i].count));
                next = cursor + piecet->pieces[i].count;
                if (pos < next){
                        *idx = i;
                        *off_in_piece = pos - cursor;
                        return;
                }
                if (pos == next){
                        *idx = i + 1;
                        *off_in_piece = 0;
                        return;
                }
                cursor = next;
        }

        *idx = piecet->piece_count;
        *off_in_piece = 0;
}

static uint8_t*
piece_ptr(const piece_t *piece, const piojo_piecet_t *piecet)
{
        PIOJO_ASSERT(piece);
        PIOJO_ASSERT(piecet);

        if (piece->source == SOURCE_ORIG){
                return piecet->orig_data + (piece->offset * piecet->esize);
        }
        return piecet->add_data + (piece->offset * piecet->esize);
}
