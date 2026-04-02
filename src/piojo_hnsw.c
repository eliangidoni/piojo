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
 * @addtogroup piojohnsw Piojo HNSW
 * @{
 * Piojo HNSW implementation.
 */

#include <piojo/piojo_hnsw.h>
#include <piojo/piojo_hash.h>
#include <piojo/piojo_heap.h>
#include <piojo_defs.h>

/* Hard cap on graph depth to 48 levels */
#define MAX_LEVELS 48

/* Neighbor storage per entry per layer. */
typedef struct {
        size_t            *nbrs;  /* indices into entries[] */
        piojo_hnsw_dist_t *dists; /* dist(ref, nbrs[i]) for each neighbor */
        size_t             cnt;   /* current neighbor count */
        size_t             cap;   /* allocated capacity */
} layer_t;

typedef struct {
        piojo_hnsw_id_t      id;
        piojo_hnsw_scalar_t *vector;
        bool                 deleted;
        size_t               level;       /* highest layer this node lives in */
        layer_t             *layers;      /* layers[0..level], allocated on insert */
} entry_t;

/* Heap element: (distance, entries[] index) pair. */
typedef struct {
        piojo_hnsw_dist_t dist;
        size_t            idx;
} cand_t;

/* Reusable per-index workspace for search_layer. */
typedef struct {
        cand_t       *pool;       /* flat pool for heap elements */
        size_t        pool_cap;   /* allocated capacity */
        piojo_heap_t *candidates; /* min-heap: nearest unvisited */
        piojo_heap_t *results;    /* max-heap: best ef results */
        piojo_hash_t *visited;    /* size_t idx -> bool */
        size_t             *tmp_idxs;  /* connect_neighbors */
        piojo_hnsw_dist_t  *tmp_dists; /* connect_neighbors */
        size_t             *rejected;  /* connect_neighbors */
} hnsw_workspace_t;

struct piojo_hnsw_t {
        size_t            dim;
        size_t            m;        /* max neighbors */
        size_t            ef_construction;
        size_t            ecount;   /* active (non-deleted) entries */
        size_t            usedcnt;  /* slots used in entries[] */
        size_t            cap;      /* allocated capacity of entries[] */
        bool              has_ep;   /* TRUE once first element is inserted */
        size_t            ep_idx;   /* entries[] index of the global entry point */
        size_t            max_level;/* current highest layer */
        double            ml;       /* 1/log(m) level probability factor */
        entry_t          *entries;
        piojo_hash_t     *id_index; /* piojo_id_t -> size_t (entries[] index) */
        piojo_hnsw_dist_cb dist;
        piojo_alloc_if    allocator;
        hnsw_workspace_t  ws;
};

/** @hideinitializer Size of HNSW index in bytes */
const size_t piojo_hnsw_sizeof = sizeof(piojo_hnsw_t);

static piojo_hnsw_dist_t
dist_l2(const piojo_hnsw_scalar_t *v1, const piojo_hnsw_scalar_t *v2,
        size_t dim);

static bool
find_idx(piojo_hnsw_id_t id, size_t *idx, const piojo_hnsw_t *hnsw);

static void
reserve(size_t n, piojo_hnsw_t *hnsw);

static void
free_entry_layers(entry_t *e, const piojo_hnsw_t *hnsw);

static void
free_entry(entry_t *e, const piojo_hnsw_t *hnsw);

static void
alloc_entry_layers(entry_t *e, size_t level, const piojo_hnsw_t *hnsw);

static bool
cand_min_leq(piojo_opaque_t e1, piojo_opaque_t e2);

static bool
cand_max_leq(piojo_opaque_t e1, piojo_opaque_t e2);

static void
search_layer(const piojo_hnsw_scalar_t *query,
             const size_t *ep_idxs, size_t ep_count,
             size_t ef, size_t layer,
             piojo_hnsw_t *hnsw,
             size_t *out_idxs, piojo_hnsw_dist_t *out_dists, size_t *out_count);

static void
select_neighbors_heuristic(const size_t *cand_idxs,
                            const piojo_hnsw_dist_t *cand_dists,
                            size_t cand_count,
                            size_t max_selected, const piojo_hnsw_t *hnsw,
                            size_t *out_selected, size_t *out_selected_cnt,
                            size_t *out_rejected, size_t *out_rejected_cnt);

static void
connect_neighbors(size_t new_idx, size_t layer,
                  const size_t *nbr_idxs, size_t nbr_count,
                  piojo_hnsw_t *hnsw);

static void
disconnect_entry_in_layer(layer_t *fl, size_t nbr_to_remove);

static void
disconnect_entry(size_t idx, piojo_hnsw_t *hnsw);

static void
layer_insort(size_t ref_idx, size_t new_idx, layer_t *layer,
             const piojo_hnsw_t *hnsw);

static void
collect_candidates(const layer_t *layer, size_t *tmp_idxs,
                   piojo_hnsw_dist_t *tmp_dists);

static void
connect_entry(size_t entry_idx, piojo_hnsw_t *hnsw);

/**
 * Allocates a new HNSW index.
 * Uses default allocator.
 * @param[in] dim Vector dimension.
 * @param[in] m Maximum number of connections per element.
 * @param[in] ef_construction Construction search width.
 * @param[in] dist Distance function. If @b NULL, squared L2 is used.
 * @return New HNSW index.
 */
piojo_hnsw_t*
piojo_hnsw_alloc(size_t dim, size_t m, size_t ef_construction,
                 piojo_hnsw_dist_cb dist)
{
        return piojo_hnsw_alloc_cb(dim, m, ef_construction,
                                   dist, piojo_alloc_default);
}

/**
 * Allocates a new HNSW index.
 * @param[in] dim Vector dimension.
 * @param[in] m Maximum number of connections per element.
 * @param[in] ef_construction Construction search width.
 * @param[in] dist Distance function. If @b NULL, squared L2 is used.
 * @param[in] allocator Allocator to be used.
 * @return New HNSW index.
 */
piojo_hnsw_t*
piojo_hnsw_alloc_cb(size_t dim, size_t m, size_t ef_construction,
                    piojo_hnsw_dist_cb dist,
                    piojo_alloc_if allocator)
{
        piojo_hnsw_t *hnsw;
        PIOJO_ASSERT(dim > 0);
        PIOJO_ASSERT(m > 1);
        PIOJO_ASSERT(ef_construction > 0);

        hnsw = (piojo_hnsw_t *) allocator.alloc_cb(sizeof(piojo_hnsw_t));
        PIOJO_ASSERT(hnsw);

        hnsw->dim = dim;
        hnsw->m = m;
        hnsw->ef_construction = ef_construction;
        hnsw->ecount = 0;
        hnsw->usedcnt = 0;
        hnsw->cap = 8;
        hnsw->has_ep = FALSE;
        hnsw->ep_idx = 0;
        hnsw->max_level = 0;
        hnsw->ml = 1.0 / log((double)m);
        hnsw->entries = (entry_t *) allocator.alloc_cb(hnsw->cap *
                                                       sizeof(entry_t));
        PIOJO_ASSERT(hnsw->entries);
        hnsw->id_index = piojo_hash_alloc_cb_i32k(sizeof(size_t), allocator);
        PIOJO_ASSERT(hnsw->id_index);
        hnsw->dist = dist ? dist : dist_l2;
        hnsw->allocator = allocator;

        hnsw->ws.pool = NULL;
        hnsw->ws.pool_cap = 0;
        hnsw->ws.candidates = piojo_heap_alloc_cb(cand_min_leq, allocator);
        PIOJO_ASSERT(hnsw->ws.candidates);
        hnsw->ws.results = piojo_heap_alloc_cb(cand_max_leq, allocator);
        PIOJO_ASSERT(hnsw->ws.results);
        hnsw->ws.visited = piojo_hash_alloc_cb_sizk(sizeof(bool), allocator);
        PIOJO_ASSERT(hnsw->ws.visited);

        size_t scratch_cap = m * 2 + 1;
        hnsw->ws.tmp_idxs = (size_t *)
                allocator.alloc_cb(scratch_cap * sizeof(size_t));
        hnsw->ws.tmp_dists = (piojo_hnsw_dist_t *)
                allocator.alloc_cb(scratch_cap * sizeof(piojo_hnsw_dist_t));
        hnsw->ws.rejected = (size_t *)
                allocator.alloc_cb(scratch_cap * sizeof(size_t));
        PIOJO_ASSERT(hnsw->ws.tmp_idxs && hnsw->ws.tmp_dists && hnsw->ws.rejected);

        return hnsw;
}

/**
 * Copies @a hnsw and all its entries.
 * @param[in] hnsw HNSW index being copied.
 * @return New HNSW index.
 */
piojo_hnsw_t*
piojo_hnsw_copy(const piojo_hnsw_t *hnsw)
{
        piojo_hnsw_t *newh;
        PIOJO_ASSERT(hnsw);

        newh = piojo_hnsw_alloc_cb(hnsw->dim,
                                   hnsw->m,
                                   hnsw->ef_construction,
                                   hnsw->dist,
                                   hnsw->allocator);
        PIOJO_ASSERT(newh);

        reserve(hnsw->usedcnt, newh);
        for (size_t idx = 0; idx < hnsw->usedcnt; ++idx) {
                const entry_t *src = &hnsw->entries[idx];
                entry_t *dst = &newh->entries[idx];

                dst->id = src->id;
                dst->deleted = src->deleted;
                dst->level = src->level;

                if (src->deleted) {
                        dst->vector = NULL;
                        dst->layers = NULL;
                        size_t new_idx = idx;
                        piojo_hash_insert(&dst->id, &new_idx, newh->id_index);
                        continue;
                }

                dst->vector = (piojo_hnsw_scalar_t *)
                        newh->allocator.alloc_cb(newh->dim *
                                                 sizeof(piojo_hnsw_scalar_t));
                PIOJO_ASSERT(dst->vector);
                memcpy(dst->vector, src->vector,
                       newh->dim * sizeof(piojo_hnsw_scalar_t));

                /* copy layer neighbor arrays */
                dst->layers = (layer_t *)
                        newh->allocator.alloc_cb((src->level + 1) *
                                                 sizeof(layer_t));
                PIOJO_ASSERT(dst->layers);
                for (size_t l = 0; l <= src->level; ++l) {
                        size_t cap = src->layers[l].cap;
                        dst->layers[l].cnt = src->layers[l].cnt;
                        dst->layers[l].cap = cap;
                        dst->layers[l].nbrs  = NULL;
                        dst->layers[l].dists = NULL;
                        if (cap > 0) {
                                dst->layers[l].nbrs = (size_t *)
                                        newh->allocator.alloc_cb(cap *
                                                                 sizeof(size_t));
                                PIOJO_ASSERT(dst->layers[l].nbrs);
                                memcpy(dst->layers[l].nbrs,
                                       src->layers[l].nbrs,
                                       cap * sizeof(size_t));
                                dst->layers[l].dists = (piojo_hnsw_dist_t *)
                                        newh->allocator.alloc_cb(cap *
                                                          sizeof(piojo_hnsw_dist_t));
                                PIOJO_ASSERT(dst->layers[l].dists);
                                memcpy(dst->layers[l].dists,
                                       src->layers[l].dists,
                                       cap * sizeof(piojo_hnsw_dist_t));
                        }
                }

                size_t new_idx = idx;
                piojo_hash_insert(&dst->id, &new_idx, newh->id_index);
        }
        newh->ecount = hnsw->ecount;
        newh->usedcnt = hnsw->usedcnt;
        newh->has_ep = hnsw->has_ep;
        newh->ep_idx = hnsw->ep_idx;
        newh->max_level = hnsw->max_level;

        return newh;
}

/**
 * Frees @a hnsw and all its entries.
 * @param[in] hnsw HNSW index being freed.
 */
void
piojo_hnsw_free(const piojo_hnsw_t *hnsw)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(hnsw);

        for (size_t idx = 0; idx < hnsw->usedcnt; ++idx) {
                free_entry(&hnsw->entries[idx], hnsw);
        }
        hnsw->allocator.free_cb(hnsw->entries);
        piojo_hash_free(hnsw->id_index);

        piojo_heap_free(hnsw->ws.candidates);
        piojo_heap_free(hnsw->ws.results);
        piojo_hash_free(hnsw->ws.visited);
        if (hnsw->ws.pool) {
                hnsw->allocator.free_cb(hnsw->ws.pool);
        }
        hnsw->allocator.free_cb(hnsw->ws.tmp_idxs);
        hnsw->allocator.free_cb(hnsw->ws.tmp_dists);
        hnsw->allocator.free_cb(hnsw->ws.rejected);

        allocator = hnsw->allocator;
        allocator.free_cb(hnsw);
}

/**
 * Deletes all entries in @a hnsw.
 * @param[out] hnsw HNSW index being cleared.
 */
void
piojo_hnsw_clear(piojo_hnsw_t *hnsw)
{
        PIOJO_ASSERT(hnsw);

        for (size_t idx = 0; idx < hnsw->usedcnt; ++idx) {
                free_entry(&hnsw->entries[idx], hnsw);
        }
        hnsw->ecount = 0;
        hnsw->usedcnt = 0;
        hnsw->has_ep = FALSE;
        hnsw->ep_idx = 0;
        hnsw->max_level = 0;
        piojo_hash_clear(hnsw->id_index);
}

/**
 * Returns number of entries.
 * @param[in] hnsw
 * @return Number of active entries in @a hnsw.
 */
size_t
piojo_hnsw_size(const piojo_hnsw_t *hnsw)
{
        PIOJO_ASSERT(hnsw);
        return hnsw->ecount;
}

/**
 * Returns configured vector dimension.
 * @param[in] hnsw
 * @return Vector dimension of @a hnsw.
 */
size_t
piojo_hnsw_dim(const piojo_hnsw_t *hnsw)
{
        PIOJO_ASSERT(hnsw);
        return hnsw->dim;
}

/**
 * Returns configured maximum neighbors parameter.
 * @param[in] hnsw
 * @return Configured maximum number of connections per element.
 */
size_t
piojo_hnsw_m(const piojo_hnsw_t *hnsw)
{
        PIOJO_ASSERT(hnsw);
        return hnsw->m;
}

/**
 * Returns configured construction search width.
 * @param[in] hnsw
 * @return Construction search width.
 */
size_t
piojo_hnsw_ef_construction(const piojo_hnsw_t *hnsw)
{
        PIOJO_ASSERT(hnsw);
        return hnsw->ef_construction;
}

/**
 * Inserts a new entry.
 * If @a id exists and is deleted, it is fully reinserted (graph links rebuilt).
 * @param[in] id Entry id.
 * @param[in] vector Entry vector.
 * @param[out] hnsw HNSW index being modified.
 * @return @b TRUE if inserted or reinserted, @b FALSE if @a id already exists.
 */
bool
piojo_hnsw_insert(piojo_hnsw_id_t id, const piojo_hnsw_scalar_t *vector,
                  piojo_hnsw_t *hnsw)
{
        size_t idx;
        piojo_hnsw_scalar_t *newv;
        PIOJO_ASSERT(hnsw);
        PIOJO_ASSERT(vector);

        bool found_p = find_idx(id, &idx, hnsw);
        if (found_p) {
                if (!hnsw->entries[idx].deleted) {
                        return FALSE;
                }
                newv = (piojo_hnsw_scalar_t *)
                        hnsw->allocator.alloc_cb(hnsw->dim * sizeof(piojo_hnsw_scalar_t));
                PIOJO_ASSERT(newv);
                memcpy(newv, vector, hnsw->dim * sizeof(piojo_hnsw_scalar_t));
                hnsw->entries[idx].vector = newv;
                hnsw->entries[idx].deleted = FALSE;
                ++hnsw->ecount;
                connect_entry(idx, hnsw);
                return TRUE;
        }

        if (hnsw->usedcnt == hnsw->cap) {
                reserve(hnsw->cap * 2, hnsw);
        }

        idx = hnsw->usedcnt;
        newv = (piojo_hnsw_scalar_t *)
                hnsw->allocator.alloc_cb(hnsw->dim * sizeof(piojo_hnsw_scalar_t));
        PIOJO_ASSERT(newv);
        memcpy(newv, vector, hnsw->dim * sizeof(piojo_hnsw_scalar_t));

        hnsw->entries[idx].id = id;
        hnsw->entries[idx].vector = newv;
        hnsw->entries[idx].deleted = FALSE;
        hnsw->entries[idx].level = 0;
        hnsw->entries[idx].layers = NULL;
        ++hnsw->usedcnt;
        ++hnsw->ecount;

        piojo_hash_insert(&id, &idx, hnsw->id_index);
        connect_entry(idx, hnsw);

        return TRUE;
}

/**
 * Searches entry by id.
 * @param[in] id Entry id.
 * @param[in] hnsw
 * @return Entry vector if @a id exists and is active, @b NULL otherwise.
 */
const piojo_hnsw_scalar_t*
piojo_hnsw_search(piojo_hnsw_id_t id, const piojo_hnsw_t *hnsw)
{
        size_t idx;
        PIOJO_ASSERT(hnsw);

        bool found_p = find_idx(id, &idx, hnsw);
        if (found_p && !hnsw->entries[idx].deleted) {
                return hnsw->entries[idx].vector;
        }
        return NULL;
}

/**
 * Logically deletes an entry by id.
 * @param[in] id Entry id.
 * @param[out] hnsw HNSW index being modified.
 * @return @b TRUE if entry is deleted, @b FALSE otherwise.
 */
bool
piojo_hnsw_delete(piojo_hnsw_id_t id, piojo_hnsw_t *hnsw)
{
        size_t idx;
        PIOJO_ASSERT(hnsw);

        bool found_p = find_idx(id, &idx, hnsw);

        if (!found_p || hnsw->entries[idx].deleted) {
                return FALSE;
        }

        hnsw->entries[idx].deleted = TRUE;
        --hnsw->ecount;

        if (idx != hnsw->ep_idx) {
                disconnect_entry(idx, hnsw);
                free_entry(&hnsw->entries[idx], hnsw);
                return TRUE;
        }
        if (hnsw->ecount == 0) {
                hnsw->has_ep = FALSE;
                disconnect_entry(idx, hnsw);
                free_entry(&hnsw->entries[idx], hnsw);
                return TRUE;
        }
        /* The deleted node was the entry point, find a replacement. */
        size_t new_ep = hnsw->usedcnt; /* sentinel (invalid) */
        size_t lev = hnsw->entries[idx].level;
        for (size_t l = lev + 1; l-- > 0 && new_ep == hnsw->usedcnt; ) {
                layer_t *lyr = &hnsw->entries[idx].layers[l];
                for (size_t n = 0; n < lyr->cnt; ++n) {
                        size_t nbr = lyr->nbrs[n];
                        if (!hnsw->entries[nbr].deleted) {
                                new_ep = nbr;
                                break;
                        }
                }
        }
        /* Else, find the globally highest level among all */
        if (new_ep == hnsw->usedcnt) {
                for (size_t k = 0; k < hnsw->usedcnt; ++k) {
                        if (hnsw->entries[k].deleted) {
                                continue;
                        }
                        if (new_ep == hnsw->usedcnt ||
                                hnsw->entries[k].level > hnsw->entries[new_ep].level) {
                                new_ep = k;
                        }
                }
        }
        hnsw->ep_idx    = new_ep;
        hnsw->max_level = hnsw->entries[new_ep].level;
        disconnect_entry(idx, hnsw);
        free_entry(&hnsw->entries[idx], hnsw);
        return TRUE;
}

/**
 * Checks if entry id is present and active.
 * @param[in] id Entry id.
 * @param[in] hnsw
 * @return @b TRUE if present, @b FALSE otherwise.
 */
bool
piojo_hnsw_contain_p(piojo_hnsw_id_t id, const piojo_hnsw_t *hnsw)
{
        return (piojo_hnsw_search(id, hnsw) != NULL);
}

/**
 * Finds up to @a k nearest neighbors of @a query using HNSW graph traversal.
 * Results are sorted by ascending distance.
 * @param[in] query Query vector.
 * @param[in] k Maximum number of neighbors.
 * @param[in] ef_search Search width (candidates explored at layer 0; >= k recommended).
 * @param[out] ids Output neighbor ids.
 * @param[out] dists Optional output distances (may be @b NULL).
 * @param[in] hnsw
 * @return Number of neighbors written.
 */
size_t
piojo_hnsw_knn(const piojo_hnsw_scalar_t *query,
               size_t k,
               size_t ef_search,
               piojo_hnsw_id_t *ids,
               piojo_hnsw_dist_t *dists,
               const piojo_hnsw_t *hnsw)
{
        size_t ep, found, written;
        size_t *res_idxs;
        piojo_hnsw_dist_t *res_dists;
        size_t ef;

        PIOJO_ASSERT(hnsw);
        PIOJO_ASSERT(query);
        if (k == 0) {
                return 0;
        }
        PIOJO_ASSERT(ids);

        if (!hnsw->has_ep || hnsw->ecount == 0) {
                return 0;
        }

        ef = (ef_search > k) ? ef_search : k;

        res_idxs = (size_t *)
                hnsw->allocator.alloc_cb(ef * sizeof(size_t));
        res_dists = (piojo_hnsw_dist_t *)
                hnsw->allocator.alloc_cb(ef * sizeof(piojo_hnsw_dist_t));
        PIOJO_ASSERT(res_idxs && res_dists);

        ep = hnsw->ep_idx;

        for (size_t lc = hnsw->max_level; lc >= 1; --lc) {
                size_t tmp_cnt = 0;
                search_layer(query, &ep, 1, 1, lc, (piojo_hnsw_t *)hnsw,
                             res_idxs, res_dists, &tmp_cnt);
                if (tmp_cnt > 0) {
                        ep = res_idxs[0];
                }
        }

        found = 0;
        /* Search at level 0, full ef range */
        search_layer(query, &ep, 1, ef, 0, (piojo_hnsw_t *)hnsw,
                     res_idxs, res_dists, &found);

        written = 0;
        for (size_t i = 0; i < found && written < k; ++i) {
                size_t ridx = res_idxs[i];
                if (hnsw->entries[ridx].deleted) {
                        continue;
                }
                ids[written] = hnsw->entries[ridx].id;
                if (dists) {
                        dists[written] = res_dists[i];
                }
                ++written;
        }

        hnsw->allocator.free_cb(res_idxs);
        hnsw->allocator.free_cb(res_dists);

        return written;
}

/** @}
 * Private functions.
 */

static piojo_hnsw_dist_t
dist_l2(const piojo_hnsw_scalar_t *v1,
        const piojo_hnsw_scalar_t *v2,
        size_t dim)
{
        piojo_hnsw_dist_t sum = 0;
        for (size_t idx = 0; idx < dim; ++idx) {
                piojo_hnsw_dist_t d = v1[idx] - v2[idx];
                sum += d * d;
        }
        return sum;
}

static bool
find_idx(piojo_hnsw_id_t id, size_t *idx, const piojo_hnsw_t *hnsw)
{
        size_t *p = (size_t *) piojo_hash_search(&id, hnsw->id_index);
        if (p) {
                *idx = *p;
                return TRUE;
        }
        return FALSE;
}

static void
reserve(size_t n, piojo_hnsw_t *hnsw)
{
        if (n <= hnsw->cap) {
                return;
        }
        hnsw->entries = (entry_t *) hnsw->allocator.realloc_cb(
                hnsw->entries, n * sizeof(entry_t));
        PIOJO_ASSERT(hnsw->entries);
        hnsw->cap = n;
}

static void
free_entry_layers(entry_t *e, const piojo_hnsw_t *hnsw)
{
        if (!e->layers) {
                return;
        }
        for (size_t l = 0; l <= e->level; ++l) {
                if (e->layers[l].nbrs) {
                        hnsw->allocator.free_cb(e->layers[l].nbrs);
                }
                if (e->layers[l].dists) {
                        hnsw->allocator.free_cb(e->layers[l].dists);
                }
        }
        hnsw->allocator.free_cb(e->layers);
        e->layers = NULL;
}

static void
free_entry(entry_t *e, const piojo_hnsw_t *hnsw)
{
        if (e->vector) {
                hnsw->allocator.free_cb(e->vector);
                e->vector = NULL;
        }
        free_entry_layers(e, hnsw);
}

static void
alloc_entry_layers(entry_t *e, size_t level, const piojo_hnsw_t *hnsw)
{
        e->level = level;
        e->layers = (layer_t *)
                hnsw->allocator.alloc_cb((level + 1) * sizeof(layer_t));
        PIOJO_ASSERT(e->layers);
        for (size_t l = 0; l <= level; ++l) {
                /* +1 extra slot so insertion can temporarily exceed max before pruning */
                size_t cap = (l == 0) ? (hnsw->m * 2 + 1) : (hnsw->m + 1);
                e->layers[l].nbrs = (size_t *)
                        hnsw->allocator.alloc_cb(cap * sizeof(size_t));
                PIOJO_ASSERT(e->layers[l].nbrs);
                e->layers[l].dists = (piojo_hnsw_dist_t *)
                        hnsw->allocator.alloc_cb(cap * sizeof(piojo_hnsw_dist_t));
                PIOJO_ASSERT(e->layers[l].dists);
                e->layers[l].cnt = 0;
                e->layers[l].cap = cap;
        }
}

/* nearest candidate at top. */
static bool
cand_min_leq(piojo_opaque_t e1, piojo_opaque_t e2)
{
        return ((cand_t *)e1)->dist <= ((cand_t *)e2)->dist;
}

/* farthest candidate at top. */
static bool
cand_max_leq(piojo_opaque_t e1, piojo_opaque_t e2)
{
        return ((cand_t *)e1)->dist >= ((cand_t *)e2)->dist;
}

/* Callers must allocate out_idxs and out_dists to hold ef elements */
static void
search_layer(const piojo_hnsw_scalar_t *query,
             const size_t *ep_idxs, size_t ep_count,
             size_t ef, size_t layer,
             piojo_hnsw_t *hnsw,
             size_t *out_idxs, piojo_hnsw_dist_t *out_dists, size_t *out_count)
{
        size_t pool_cap, pool_used;
        cand_t *pool;
        piojo_heap_t *candidates; /* nearest first (unvisited) */
        piojo_heap_t *results;    /* farthest first (at most ef elements) */
        piojo_hash_t *visited;    /* size_t idx -> bool */

        /* Grow pool only when needed; retains allocation across calls. */
        if (hnsw->ws.pool_cap < hnsw->usedcnt) {
                hnsw->ws.pool = (cand_t *) hnsw->allocator.realloc_cb(
                        hnsw->ws.pool, hnsw->usedcnt * sizeof(cand_t));
                PIOJO_ASSERT(hnsw->ws.pool);
                hnsw->ws.pool_cap = hnsw->usedcnt;
        }
        pool     = hnsw->ws.pool;
        pool_cap = hnsw->ws.pool_cap;
        pool_used = 0;

        candidates = hnsw->ws.candidates;
        results    = hnsw->ws.results;
        visited    = hnsw->ws.visited;

        /* Seed with entry points. */
        for (size_t i = 0; i < ep_count; ++i) {
                size_t idx = ep_idxs[i];
                PIOJO_ASSERT(!hnsw->entries[idx].deleted);
                piojo_hnsw_dist_t d =
                        hnsw->dist(query, hnsw->entries[idx].vector, hnsw->dim);

                PIOJO_ASSERT(pool_used < pool_cap);
                pool[pool_used].dist = d;
                pool[pool_used].idx  = idx;

                piojo_heap_push((piojo_opaque_t)&pool[pool_used], candidates);
                piojo_heap_push((piojo_opaque_t)&pool[pool_used], results);
                piojo_hash_insert(&idx, NULL, visited);
                ++pool_used;
        }

        while (piojo_heap_size(candidates) > 0) {
                cand_t *c = (cand_t *) piojo_heap_peek(candidates);
                cand_t *f = (cand_t *) piojo_heap_peek(results);
                piojo_heap_pop(candidates);

                /* nearest unvisited is farther than farthest result */
                if (piojo_heap_size(results) >= ef && c->dist > f->dist) {
                        break;
                }

                /* Explore neighbors at this layer */
                entry_t *ce = &hnsw->entries[c->idx];
                if (ce->level < layer){
                        continue;
                }

                layer_t *cl = &ce->layers[layer];
                for (size_t j = 0; j < cl->cnt; ++j) {
                        size_t nbr = cl->nbrs[j];
                        if (!piojo_hash_insert(&nbr, NULL, visited)){
                                continue;
                        }

                        piojo_hnsw_dist_t nd =
                                hnsw->dist(query,
                                        hnsw->entries[nbr].vector,
                                        hnsw->dim);
                        f = (cand_t *) piojo_heap_peek(results);
                        if (piojo_heap_size(results) >= ef && nd > f->dist) {
                                continue;
                        }

                        PIOJO_ASSERT(pool_used < pool_cap);
                        pool[pool_used].dist = nd;
                        pool[pool_used].idx  = nbr;

                        piojo_heap_push(
                                (piojo_opaque_t)&pool[pool_used],
                                candidates);
                        if (!hnsw->entries[nbr].deleted) {
                                piojo_heap_push(
                                        (piojo_opaque_t)&pool[pool_used],
                                        results);
                                if (piojo_heap_size(results) > ef) {
                                        piojo_heap_pop(results);
                                }
                        }
                        ++pool_used;

                }
        }

        /* Copy results (max-heap) into output in reverse for nearest-first. */
        size_t cnt = piojo_heap_size(results);
        *out_count = cnt;
        while (piojo_heap_size(results) > 0) {
                cand_t *r = (cand_t *) piojo_heap_peek(results);
                out_idxs[--cnt]  = r->idx;
                out_dists[cnt]   = r->dist;
                piojo_heap_pop(results);
        }
        piojo_heap_clear(candidates);
        piojo_heap_clear(results);
        piojo_hash_clear(visited);
}

/*
 * From the candidates (nearest-first), greedily select each one only
 * if it's closer to the query than to every already selected neighbor.
 * - candidates are not extended by their neighbors before selection
 * - fills remaining slots from rejected candidates (keep pruned connections)
 */
static void
select_neighbors_heuristic(const size_t *cand_idxs,
                            const piojo_hnsw_dist_t *cand_dists,
                            size_t cand_count,
                            size_t max_selected, const piojo_hnsw_t *hnsw,
                            size_t *out_selected, size_t *out_selected_cnt,
                            size_t *out_rejected, size_t *out_rejected_cnt)
{
        size_t selected_cnt = 0, rejected_cnt = 0, i;

        for (i = 0; i < cand_count && selected_cnt < max_selected; ++i) {
                bool cand_closer_p = TRUE;
                for (size_t j = 0; j < selected_cnt && cand_closer_p; ++j) {
                        cand_closer_p = cand_dists[i] <
                                hnsw->dist(hnsw->entries[cand_idxs[i]].vector,
                                           hnsw->entries[out_selected[j]].vector,
                                           hnsw->dim);
                }
                if (cand_closer_p) {
                        out_selected[selected_cnt++] = cand_idxs[i];
                } else if (out_rejected) {
                        out_rejected[rejected_cnt++] = cand_idxs[i];
                }
        }

        /* fill remaining slots with the closest heuristic-rejected candidates. */
        size_t fill_end = 0;
        while (selected_cnt < max_selected && fill_end < rejected_cnt) {
                out_selected[selected_cnt++] = out_rejected[fill_end++];
        }

        *out_selected_cnt = selected_cnt;

        if (out_rejected) {
                /* drop the filled-in entries from out_rejected. */
                size_t out_cnt = 0;
                for (size_t j = fill_end; j < rejected_cnt; ++j) {
                        out_rejected[out_cnt++] = out_rejected[j];
                }
                /* Append candidates not reached because selected_cnt hit max early. */
                while (i < cand_count) {
                        out_rejected[out_cnt++] = cand_idxs[i++];
                }
                *out_rejected_cnt = out_cnt;
        }
}

/* Add bidirectional edges between new_idx and each nbr in nbr_idxs */
static void
connect_neighbors(size_t new_idx, size_t layer,
                  const size_t *nbr_idxs, size_t nbr_count,
                  piojo_hnsw_t *hnsw)
{
        size_t max_nbrs = (layer == 0) ? (hnsw->m * 2) : hnsw->m;
        layer_t *el = &hnsw->entries[new_idx].layers[layer];

        for (size_t i = 0; i < nbr_count; ++i) {
                size_t nbr = nbr_idxs[i];
                layer_t *nl = &hnsw->entries[nbr].layers[layer];

                /* Always add both directions (cap has one extra slot). */
                PIOJO_ASSERT(el->cnt < el->cap);
                layer_insort(new_idx, nbr, el, hnsw);
                layer_insort(nbr, new_idx, nl, hnsw);

                /* Shrink nbr's list if it now exceeds max_nbrs. */
                if (nl->cnt <= max_nbrs) {
                        continue;
                }
                size_t *tmp_idxs = hnsw->ws.tmp_idxs;
                piojo_hnsw_dist_t *tmp_dists = hnsw->ws.tmp_dists;
                size_t *rejected = hnsw->ws.rejected;

                collect_candidates(nl, tmp_idxs, tmp_dists);

                size_t selected_cnt = 0, rejected_cnt = 0;
                size_t tmp_cnt = nl->cnt;
                select_neighbors_heuristic(
                        tmp_idxs, tmp_dists, tmp_cnt,
                        max_nbrs, hnsw,
                        nl->nbrs, &selected_cnt,
                        rejected, &rejected_cnt);
                nl->cnt = selected_cnt;

                /* Fill in nl->dists from the saved tmp arrays */
                for (size_t k = 0; k < selected_cnt; ++k) {
                        for (size_t t = 0; t < tmp_cnt; ++t) {
                                if (tmp_idxs[t] == nl->nbrs[k]) {
                                        nl->dists[k] = tmp_dists[t];
                                        break;
                                }
                        }
                }

                /* Remove the reverse edge for each dropped candidate. */
                for (size_t j = 0; j < rejected_cnt; ++j) {
                        disconnect_entry_in_layer(
                                &hnsw->entries[rejected[j]].layers[layer], nbr);
                }

        }
}

/* Assumes the entry at entry_idx has its vector set and is in id_index */
static void
connect_entry(size_t entry_idx, piojo_hnsw_t *hnsw)
{
        size_t level, ep, ef;
        double r;
        size_t *res_idxs;
        piojo_hnsw_dist_t *res_dists;
        entry_t *e = &hnsw->entries[entry_idx];

        /* Assign random level. */
        r = (double)rand() / ((double)RAND_MAX + 1.0);
        if (r <= 0.0){
                r = 1e-10;
        }
        level = (size_t)floor(-log(r) * hnsw->ml);
        if (level > MAX_LEVELS) {
                level = MAX_LEVELS;
        }

        alloc_entry_layers(e, level, hnsw);

        if (!hnsw->has_ep) {
                hnsw->ep_idx   = entry_idx;
                hnsw->max_level = level;
                hnsw->has_ep   = TRUE;
                return;
        }

        ef = hnsw->ef_construction;
        res_idxs  = (size_t *)
                hnsw->allocator.alloc_cb(ef * sizeof(size_t));
        res_dists = (piojo_hnsw_dist_t *)
                hnsw->allocator.alloc_cb(ef * sizeof(piojo_hnsw_dist_t));
        PIOJO_ASSERT(res_idxs && res_dists);

        ep = hnsw->ep_idx;

        /* Traverse layers above insertion level greedily (ef=1). */
        for (size_t lc = hnsw->max_level; lc > level; --lc) {
                size_t tmp_cnt = 0;
                search_layer(e->vector, &ep, 1, 1, lc, hnsw,
                             res_idxs, res_dists, &tmp_cnt);
                if (tmp_cnt > 0) {
                        ep = res_idxs[0];
                }
        }

        /* Connect at each insertion layer. */
        size_t start = (level < hnsw->max_level) ? level : hnsw->max_level;
        /* Use a heap-allocated nbrs to support arbitrary m. */
        size_t max_nbrs_0 = hnsw->m * 2;
        size_t *nbrs = (size_t *)
                hnsw->allocator.alloc_cb(max_nbrs_0 * sizeof(size_t));
        PIOJO_ASSERT(nbrs);

        for (size_t lc = start + 1; lc-- > 0; ) {
                size_t res_cnt = 0, nbr_cnt = 0;
                size_t max_nbrs = (lc == 0) ? max_nbrs_0 : hnsw->m;

                search_layer(e->vector, &ep, 1, ef, lc, hnsw,
                                res_idxs, res_dists, &res_cnt);

                select_neighbors_heuristic(
                        res_idxs, res_dists, res_cnt,
                        max_nbrs, hnsw,
                        nbrs, &nbr_cnt,
                        NULL, NULL);

                connect_neighbors(entry_idx, lc, nbrs, nbr_cnt, hnsw);

                if (res_cnt > 0) {
                        ep = res_idxs[0];
                }
        }
        hnsw->allocator.free_cb(nbrs);
        hnsw->allocator.free_cb(res_idxs);
        hnsw->allocator.free_cb(res_dists);

        /* Promote entry point if new node reaches higher layer. */
        if (level > hnsw->max_level) {
                hnsw->ep_idx = entry_idx;
                hnsw->max_level = level;
        }
}

static void
layer_insort(size_t ref_idx, size_t new_idx, layer_t *layer,
             const piojo_hnsw_t *hnsw)
{
        PIOJO_ASSERT(layer->cnt < layer->cap); /* cap = max_nbrs + 1 */
        piojo_hnsw_dist_t d_new = hnsw->dist(
                hnsw->entries[ref_idx].vector,
                hnsw->entries[new_idx].vector,
                hnsw->dim);
        size_t pos = layer->cnt;
        while (pos > 0) {
                if (layer->dists[pos - 1] <= d_new) {
                        break;
                }
                layer->nbrs[pos]  = layer->nbrs[pos - 1];
                layer->dists[pos] = layer->dists[pos - 1];
                --pos;
        }
        layer->nbrs[pos]  = new_idx;
        layer->dists[pos] = d_new;
        ++layer->cnt;
}

static void
disconnect_entry_in_layer(layer_t *fl, size_t nbr_to_remove)
{
        for (size_t k = 0; k < fl->cnt; ++k) {
                if (fl->nbrs[k] == nbr_to_remove) {
                        size_t tail = fl->cnt - k - 1;
                        memmove(&fl->nbrs[k],  &fl->nbrs[k + 1],
                                tail * sizeof(fl->nbrs[0]));
                        memmove(&fl->dists[k], &fl->dists[k + 1],
                                tail * sizeof(fl->dists[0]));
                        --fl->cnt;
                        break;
                }
        }
}

static void
disconnect_entry(size_t idx, piojo_hnsw_t *hnsw)
{
        entry_t *re = &hnsw->entries[idx];
        for (size_t l = 0; l <= re->level; ++l) {
                layer_t *lyr = &re->layers[l];
                for (size_t n = 0; n < lyr->cnt; ++n) {
                        disconnect_entry_in_layer(
                                &hnsw->entries[lyr->nbrs[n]].layers[l], idx);
                }
        }
}

/* Populate tmp_idxs/tmp_dists with the existing neighbors of nbr */
static void
collect_candidates(const layer_t *layer, size_t *tmp_idxs, piojo_hnsw_dist_t *tmp_dists)
{
        /* Distances are cached in layer->dists alongside layer->nbrs. */
        memcpy(tmp_idxs,  layer->nbrs,  layer->cnt * sizeof(layer->nbrs[0]));
        memcpy(tmp_dists, layer->dists, layer->cnt * sizeof(layer->dists[0]));
}
