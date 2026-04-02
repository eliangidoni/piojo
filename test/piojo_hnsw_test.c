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
#include <piojo/piojo_hnsw.h>

void test_alloc(void)
{
	piojo_hnsw_t *hnsw;

	hnsw = piojo_hnsw_alloc(3, 16, 64, NULL);
	PIOJO_ASSERT(hnsw);
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 0);
	PIOJO_ASSERT(piojo_hnsw_dim(hnsw) == 3);
	PIOJO_ASSERT(piojo_hnsw_m(hnsw) == 16);
	PIOJO_ASSERT(piojo_hnsw_ef_construction(hnsw) == 64);
	piojo_hnsw_free(hnsw);

	hnsw = piojo_hnsw_alloc_cb(2, 8, 32, NULL, my_allocator);
	PIOJO_ASSERT(hnsw);
	piojo_hnsw_free(hnsw);

	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

void test_insert_search_delete(void)
{
	piojo_hnsw_t *hnsw;
	float v1[2] = {0, 0};
	float v2[2] = {1, 1};
	const float *v;

	hnsw = piojo_hnsw_alloc(2, 16, 64, NULL);

	PIOJO_ASSERT(piojo_hnsw_insert(10, v1, hnsw));
	PIOJO_ASSERT(! piojo_hnsw_insert(10, v2, hnsw));
	PIOJO_ASSERT(piojo_hnsw_insert(20, v2, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 2);

	v = piojo_hnsw_search(10, hnsw);
	PIOJO_ASSERT(v);
	PIOJO_ASSERT(v[0] == 0 && v[1] == 0);
	PIOJO_ASSERT(piojo_hnsw_contain_p(20, hnsw));
	PIOJO_ASSERT(! piojo_hnsw_contain_p(30, hnsw));

	PIOJO_ASSERT(piojo_hnsw_delete(10, hnsw));
	PIOJO_ASSERT(! piojo_hnsw_delete(10, hnsw));
	PIOJO_ASSERT(! piojo_hnsw_search(10, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 1);

	piojo_hnsw_free(hnsw);

	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

void test_clear(void)
{
	piojo_hnsw_t *hnsw;
	float v1[2] = {0, 0};
	float v2[2] = {2, 2};

	hnsw = piojo_hnsw_alloc_cb(2, 16, 64, NULL, my_allocator);
	piojo_hnsw_insert(1, v1, hnsw);
	piojo_hnsw_insert(2, v2, hnsw);
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 2);

	piojo_hnsw_clear(hnsw);
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 0);
	PIOJO_ASSERT(! piojo_hnsw_contain_p(1, hnsw));

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

void test_copy(void)
{
	piojo_hnsw_t *hnsw, *copy;
	float v1[2] = {1, 2};
	float v2[2] = {3, 4};
	const float *v;

	hnsw = piojo_hnsw_alloc_cb(2, 16, 64, NULL, my_allocator);
	piojo_hnsw_insert(7, v1, hnsw);
	piojo_hnsw_insert(8, v2, hnsw);

	copy = piojo_hnsw_copy(hnsw);
	PIOJO_ASSERT(copy);
	PIOJO_ASSERT(piojo_hnsw_size(copy) == 2);

	piojo_hnsw_delete(7, hnsw);
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 1);
	PIOJO_ASSERT(piojo_hnsw_size(copy) == 2);

	v = piojo_hnsw_search(7, copy);
	PIOJO_ASSERT(v);
	PIOJO_ASSERT(v[0] == 1 && v[1] == 2);

	piojo_hnsw_free(hnsw);
	piojo_hnsw_free(copy);

	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

void test_knn(void)
{
	piojo_hnsw_t *hnsw;
	float v1[2] = {0, 0};
	float v2[2] = {1, 1};
	float v3[2] = {2, 2};
	float q[2] = {0.9f, 0.8f};
	piojo_hnsw_id_t ids[3] = {0, 0, 0};
	piojo_hnsw_dist_t dists[3] = {0, 0, 0};
	size_t found;

	hnsw = piojo_hnsw_alloc(2, 16, 64, NULL);
	piojo_hnsw_insert(11, v1, hnsw);
	piojo_hnsw_insert(22, v2, hnsw);
	piojo_hnsw_insert(33, v3, hnsw);

	found = piojo_hnsw_knn(q, 2, 32, ids, dists, hnsw);
	PIOJO_ASSERT(found == 2);
	PIOJO_ASSERT(ids[0] == 22);
	PIOJO_ASSERT(ids[1] == 11 || ids[1] == 33);
	PIOJO_ASSERT(dists[0] <= dists[1]);

	piojo_hnsw_delete(22, hnsw);
	found = piojo_hnsw_knn(q, 2, 32, ids, dists, hnsw);
	PIOJO_ASSERT(found == 2);
	PIOJO_ASSERT(ids[0] != 22);
	PIOJO_ASSERT(ids[1] != 22);

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

void test_recall(void)
{
	/* Insert N 4D vectors with deterministic values, then verify that
	 * piojo_hnsw_knn achieves >= 80% recall vs brute-force top-K. */
	enum { N = 200, DIM = 4, K = 5, QUERIES = 10 };
	piojo_hnsw_t *hnsw;
	float vecs[N][DIM];
	piojo_hnsw_id_t ids[K];
	piojo_hnsw_dist_t dists[K];
	size_t q, i, j, found;
	int total_hits = 0, total_expected = 0;

	/* Seed for reproducibility. */
	srand(42);

	hnsw = piojo_hnsw_alloc(DIM, 16, 64, NULL);

	/* Generate and insert vectors. */
	for (i = 0; i < N; ++i) {
		for (j = 0; j < DIM; ++j) {
			vecs[i][j] = (float)(rand() % 1000) / 100.0f;
		}
		piojo_hnsw_insert((piojo_hnsw_id_t)i, vecs[i], hnsw);
	}

	/* For each query, compute brute-force K-NN and compare with HNSW. */
	for (q = 0; q < QUERIES; ++q) {
		float query[DIM];
		size_t bf_ids[K];
		float  bf_dists[K];
		size_t bf_found = 0;

		/* Random query vector. */
		for (j = 0; j < DIM; ++j) {
			query[j] = (float)(rand() % 1000) / 100.0f;
		}

		/* Brute-force nearest K. */
		for (i = 0; i < N; ++i) {
			float d = 0;
			size_t k2;
			for (k2 = 0; k2 < DIM; ++k2) {
				float diff = query[k2] - vecs[i][k2];
				d += diff * diff;
			}
			if (bf_found < K) {
				/* Insertion sort. */
				size_t pos = bf_found;
				while (pos > 0 && bf_dists[pos - 1] > d) {
					if (pos < K) {
						bf_ids[pos]   = bf_ids[pos - 1];
						bf_dists[pos] = bf_dists[pos - 1];
					}
					--pos;
				}
				bf_ids[pos]   = (size_t)i;
				bf_dists[pos] = d;
				++bf_found;
			} else if (d < bf_dists[K - 1]) {
				size_t pos = K - 1;
				while (pos > 0 && bf_dists[pos - 1] > d) {
					bf_ids[pos]   = bf_ids[pos - 1];
					bf_dists[pos] = bf_dists[pos - 1];
					--pos;
				}
				bf_ids[pos]   = (size_t)i;
				bf_dists[pos] = d;
			}
		}

		/* HNSW search. */
		found = piojo_hnsw_knn(query, K, 64, ids, dists, hnsw);
		PIOJO_ASSERT(found == K);

		/* Count how many HNSW results appear in brute-force top-K. */
		for (i = 0; i < found; ++i) {
			for (j = 0; j < bf_found; ++j) {
				if ((size_t)ids[i] == bf_ids[j]) {
					++total_hits;
					break;
				}
			}
		}
		total_expected += K;
	}

	/* Require at least 80% recall. */
	PIOJO_ASSERT(total_hits * 100 >= total_expected * 80);

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

/* Test deleted nodes do not consume ef result-budget slots in knn. */
void test_knn_deleted_budget(void)
{
	/* ef_search=5, k=3. Insert 5 "to-be-deleted" close nodes and 3 live
	 * far nodes. Without Phase 2.1 fix the ef=5 result heap fills with
	 * deleted entries and knn returns 0; with the fix it returns 3. */
	enum { DIM = 1, K = 3, EF = 5 };
	piojo_hnsw_t *hnsw;
	piojo_hnsw_id_t ids[K];
	piojo_hnsw_dist_t dists[K];
	size_t found, i;
	float query[DIM];

	query[0] = 0.0f;
	hnsw = piojo_hnsw_alloc(DIM, 4, 200, NULL);

	/* Insert 5 nodes very close to query (will be deleted). */
	for (i = 1; i <= EF; ++i) {
		float v[DIM];
		v[0] = (float)i * 0.1f;
		piojo_hnsw_insert((piojo_hnsw_id_t)i, v, hnsw);
	}
	/* Insert 3 live nodes far from query. */
	for (i = 1; i <= K; ++i) {
		float v[DIM];
		v[0] = (float)(EF + i) * 100.0f;
		piojo_hnsw_insert((piojo_hnsw_id_t)(EF + i), v, hnsw);
	}
	/* Delete the close nodes. */
	for (i = 1; i <= EF; ++i) {
		piojo_hnsw_delete((piojo_hnsw_id_t)i, hnsw);
	}
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == K);

	/* Must return K live results even though ef == number-of-deleted-nodes. */
	found = piojo_hnsw_knn(query, K, EF, ids, dists, hnsw);
	PIOJO_ASSERT(found == K);
	/* No deleted ids in result. */
	for (i = 0; i < found; ++i) {
		PIOJO_ASSERT(ids[i] > (piojo_hnsw_id_t)EF);
	}

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

/* Test delete-then-reinsert leaves no stale/duplicate edges. */
void test_reinsert_no_duplicates(void)
{
	/* Insert 10 nodes, delete id=5, reinsert id=5 multiple times.
	 * knn near id=5's position should return correct, non-duplicate ids. */
	enum { DIM = 2, N = 10 };
	piojo_hnsw_t *hnsw;
	piojo_hnsw_id_t ids[3];
	piojo_hnsw_dist_t dists[3];
	size_t i, found;
	float vecs[N][DIM];
	float q[DIM];

	for (i = 0; i < N; ++i) {
		vecs[i][0] = (float)i;
		vecs[i][1] = (float)i;
	}
	q[0] = vecs[5][0];
	q[1] = vecs[5][1];

	hnsw = piojo_hnsw_alloc(DIM, 4, 64, NULL);
	for (i = 0; i < N; ++i) {
		piojo_hnsw_insert((piojo_hnsw_id_t)i, vecs[i], hnsw);
	}

	/* Delete and reinsert id=5 three times. */
	piojo_hnsw_delete(5, hnsw);
	PIOJO_ASSERT(piojo_hnsw_insert(5, vecs[5], hnsw));
	piojo_hnsw_delete(5, hnsw);
	PIOJO_ASSERT(piojo_hnsw_insert(5, vecs[5], hnsw));
	piojo_hnsw_delete(5, hnsw);
	PIOJO_ASSERT(piojo_hnsw_insert(5, vecs[5], hnsw));

	/* knn at id=5's position: nearest is id=5 itself, no duplicates. */
	found = piojo_hnsw_knn(q, 3, 64, ids, dists, hnsw);
	PIOJO_ASSERT(found == 3);
	PIOJO_ASSERT(ids[0] == 5);
	PIOJO_ASSERT(ids[0] != ids[1]);
	PIOJO_ASSERT(ids[1] != ids[2]);
	PIOJO_ASSERT(ids[0] != ids[2]);

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

/* Test deleting the entry point does not break knn or subsequent inserts. */
void test_delete_entry_point(void)
{
	/* The first-inserted node (id=1) becomes the initial entry point.
	 * Delete it and verify that knn and insert still work correctly. */
	piojo_hnsw_t *hnsw;
	float v1[2] = {0.0f, 0.0f};
	float v2[2] = {1.0f, 0.0f};
	float v3[2] = {0.0f, 1.0f};
	float v4[2] = {1.0f, 1.0f};
	float v5[2] = {5.0f, 5.0f};
	float q[2]  = {0.5f, 0.5f};
	piojo_hnsw_id_t ids[4];
	piojo_hnsw_dist_t dists[4];
	size_t found, i;

	hnsw = piojo_hnsw_alloc(2, 4, 32, NULL);
	piojo_hnsw_insert(1, v1, hnsw); /* becomes ep_idx = 0 */
	piojo_hnsw_insert(2, v2, hnsw);
	piojo_hnsw_insert(3, v3, hnsw);
	piojo_hnsw_insert(4, v4, hnsw);

	/* Delete the original entry point. */
	PIOJO_ASSERT(piojo_hnsw_delete(1, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 3);

	/* knn should return 3 results, none being the deleted id=1. */
	found = piojo_hnsw_knn(q, 3, 32, ids, dists, hnsw);
	PIOJO_ASSERT(found == 3);
	for (i = 0; i < found; ++i) {
		PIOJO_ASSERT(ids[i] != 1);
	}

	/* A new insert after the ep was deleted must also work. */
	PIOJO_ASSERT(piojo_hnsw_insert(5, v5, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 4);

	found = piojo_hnsw_knn(q, 4, 32, ids, dists, hnsw);
	PIOJO_ASSERT(found == 4);
	for (i = 0; i < found; ++i) {
		PIOJO_ASSERT(ids[i] != 1);
	}

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

/* Test: recall >= 80% after 50% deletion among live nodes. */
void test_recall_after_deletion(void)
{
	enum { N = 200, DIM = 4, K = 5, QUERIES = 10 };
	piojo_hnsw_t *hnsw;
	float vecs[N][DIM];
	piojo_hnsw_id_t ids[K];
	piojo_hnsw_dist_t dists[K];
	size_t q, i, j, found;
	int total_hits = 0, total_expected = 0;

	srand(99);
	hnsw = piojo_hnsw_alloc(DIM, 16, 64, NULL);

	/* Insert N vectors. */
	for (i = 0; i < N; ++i) {
		for (j = 0; j < DIM; ++j) {
			vecs[i][j] = (float)(rand() % 1000) / 100.0f;
		}
		piojo_hnsw_insert((piojo_hnsw_id_t)i, vecs[i], hnsw);
	}

	/* Delete all odd-indexed nodes (~50%). */
	for (i = 1; i < N; i += 2) {
		piojo_hnsw_delete((piojo_hnsw_id_t)i, hnsw);
	}
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == N / 2);

	/* Verify no deleted node appears in knn results. */
	{
		float qv[DIM];
		for (j = 0; j < DIM; ++j) qv[j] = 5.0f;
		found = piojo_hnsw_knn(qv, K, 64, ids, dists, hnsw);
		for (i = 0; i < found; ++i) {
			PIOJO_ASSERT((size_t)ids[i] % 2 == 0);
		}
	}

	/* Recall check: HNSW vs brute-force over live (even-indexed) nodes. */
	for (q = 0; q < QUERIES; ++q) {
		float query[DIM];
		size_t bf_ids[K];
		float  bf_dists[K];
		size_t bf_found = 0;

		for (j = 0; j < DIM; ++j) {
			query[j] = (float)(rand() % 1000) / 100.0f;
		}

		/* Brute-force over live nodes. */
		for (i = 0; i < N; i += 2) {
			float d = 0;
			size_t k2;
			for (k2 = 0; k2 < DIM; ++k2) {
				float diff = query[k2] - vecs[i][k2];
				d += diff * diff;
			}
			if (bf_found < K) {
				size_t pos = bf_found;
				while (pos > 0 && bf_dists[pos - 1] > d) {
					if (pos < K) {
						bf_ids[pos]   = bf_ids[pos - 1];
						bf_dists[pos] = bf_dists[pos - 1];
					}
					--pos;
				}
				bf_ids[pos]   = (size_t)i;
				bf_dists[pos] = d;
				++bf_found;
			} else if (d < bf_dists[K - 1]) {
				size_t pos = K - 1;
				while (pos > 0 && bf_dists[pos - 1] > d) {
					bf_ids[pos]   = bf_ids[pos - 1];
					bf_dists[pos] = bf_dists[pos - 1];
					--pos;
				}
				bf_ids[pos]   = (size_t)i;
				bf_dists[pos] = d;
			}
		}

		found = piojo_hnsw_knn(query, K, 64, ids, dists, hnsw);
		PIOJO_ASSERT(found == K);

		for (i = 0; i < found; ++i) {
			for (j = 0; j < bf_found; ++j) {
				if ((size_t)ids[i] == bf_ids[j]) {
					++total_hits;
					break;
				}
			}
		}
		total_expected += K;
	}

	/* Require at least 80% recall over live nodes. */
	PIOJO_ASSERT(total_hits * 100 >= total_expected * 80);

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

void test_delete_sole_entry_point(void)
{
	piojo_hnsw_t *hnsw;
	float v1[2] = {1.0f, 2.0f};
	float v2[2] = {3.0f, 4.0f};
	piojo_hnsw_id_t ids[1];
	piojo_hnsw_dist_t dists[1];
	size_t found;
	float q[2] = {0.0f, 0.0f};

	hnsw = piojo_hnsw_alloc(2, 4, 16, NULL);

	/* Insert a single entry it becomes the entry point. */
	PIOJO_ASSERT(piojo_hnsw_insert(1, v1, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 1);

	/* Delete the sole entry */
	PIOJO_ASSERT(piojo_hnsw_delete(1, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 0);

	/* knn on an empty index must return 0. */
	found = piojo_hnsw_knn(q, 1, 16, ids, dists, hnsw);
	PIOJO_ASSERT(found == 0);

	/* A fresh insert after the reset must work and be findable. */
	PIOJO_ASSERT(piojo_hnsw_insert(2, v2, hnsw));
	PIOJO_ASSERT(piojo_hnsw_size(hnsw) == 1);
	PIOJO_ASSERT(piojo_hnsw_contain_p(2, hnsw));

	found = piojo_hnsw_knn(q, 1, 16, ids, dists, hnsw);
	PIOJO_ASSERT(found == 1);
	PIOJO_ASSERT(ids[0] == 2);

	piojo_hnsw_free(hnsw);
	assert_allocator_alloc(0);
	assert_allocator_init(0);
}

int main(void)
{
	test_alloc();
	test_insert_search_delete();
	test_clear();
	test_copy();
	test_knn();
	test_recall();
	test_knn_deleted_budget();
	test_reinsert_no_duplicates();
	test_delete_entry_point();
	test_recall_after_deletion();
	test_delete_sole_entry_point();

	assert_allocator_init(0);
	assert_allocator_alloc(0);

	return 0;
}
