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
 * Piojo HNSW API.
 */

/**
 * @file
 * @addtogroup piojohnsw
 */

#ifndef PIOJO_HNSW_H_
#define PIOJO_HNSW_H_

#include <piojo/piojo.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct piojo_hnsw_t piojo_hnsw_t;
extern const size_t piojo_hnsw_sizeof;

/** @{ */
/** HNSW scalar type. */
typedef float piojo_hnsw_scalar_t;

/** HNSW distance type. */
typedef float piojo_hnsw_dist_t;

/** HNSW item id. */
typedef piojo_id_t piojo_hnsw_id_t;

/** Returns distance between @a v1 and @a v2. */
typedef piojo_hnsw_dist_t
(*piojo_hnsw_dist_cb) (const piojo_hnsw_scalar_t *v1,
					   const piojo_hnsw_scalar_t *v2,
					   size_t dim);
/** @} */

piojo_hnsw_t*
piojo_hnsw_alloc(size_t dim, size_t m, size_t ef_construction,
				 piojo_hnsw_dist_cb dist);

piojo_hnsw_t*
piojo_hnsw_alloc_cb(size_t dim, size_t m, size_t ef_construction,
					piojo_hnsw_dist_cb dist,
					piojo_alloc_if allocator);

piojo_hnsw_t*
piojo_hnsw_copy(const piojo_hnsw_t *hnsw);

void
piojo_hnsw_free(const piojo_hnsw_t *hnsw);

void
piojo_hnsw_clear(piojo_hnsw_t *hnsw);

size_t
piojo_hnsw_size(const piojo_hnsw_t *hnsw);

size_t
piojo_hnsw_dim(const piojo_hnsw_t *hnsw);

size_t
piojo_hnsw_m(const piojo_hnsw_t *hnsw);

size_t
piojo_hnsw_ef_construction(const piojo_hnsw_t *hnsw);

bool
piojo_hnsw_insert(piojo_hnsw_id_t id, const piojo_hnsw_scalar_t *vector,
				  piojo_hnsw_t *hnsw);

const piojo_hnsw_scalar_t*
piojo_hnsw_search(piojo_hnsw_id_t id, const piojo_hnsw_t *hnsw);

bool
piojo_hnsw_delete(piojo_hnsw_id_t id, piojo_hnsw_t *hnsw);

bool
piojo_hnsw_contain_p(piojo_hnsw_id_t id, const piojo_hnsw_t *hnsw);

size_t
piojo_hnsw_knn(const piojo_hnsw_scalar_t *query,
			   size_t k,
			   size_t ef_search,
			   piojo_hnsw_id_t *ids,
			   piojo_hnsw_dist_t *dists,
			   const piojo_hnsw_t *hnsw);

#ifdef __cplusplus
}
#endif
#endif
