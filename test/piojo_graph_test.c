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
#include <piojo/piojo_graph.h>

bool veq(piojo_graph_vid_t v1, piojo_graph_vid_t v2)
{
        return (memcmp(&v1,&v2,sizeof(v1)) == 0) ? TRUE : FALSE;
}

bool vvisit (piojo_graph_vid_t v, const piojo_graph_t *graph)
{
        size_t *cnt = (size_t*)piojo_graph_gvalue(graph);
        PIOJO_UNUSED(graph);
        *cnt -= (int) v;
        return FALSE;
}

void test_alloc(void)
{
        piojo_graph_t *graph;

        graph = piojo_graph_alloc(PIOJO_GRAPH_DIR_FALSE);
        PIOJO_ASSERT(graph);
        piojo_graph_free(graph);

        graph = piojo_graph_alloc(PIOJO_GRAPH_DIR_TRUE);
        PIOJO_ASSERT(graph);
        piojo_graph_free(graph);

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, piojo_alloc_default);
        PIOJO_ASSERT(graph);
        piojo_graph_free(graph);
}

void test_copy(void)
{
        piojo_graph_t *graph,*copy;
        piojo_graph_vid_t v=1,v2=2;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_link(0, v, v2, graph);

        copy = piojo_graph_copy(graph);
        PIOJO_ASSERT(copy);

        PIOJO_ASSERT(piojo_graph_linked(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, graph), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 0);

        PIOJO_ASSERT(piojo_graph_linked(v,v2,copy));
        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,copy) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, copy), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, copy) == 0);

        piojo_graph_link(1, v, v2, copy);
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, copy) == 1);
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 0);

        piojo_graph_unlink_all(v, copy);
        PIOJO_ASSERT(! piojo_graph_linked(v,v2,copy));
        PIOJO_ASSERT(piojo_graph_linked(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, graph), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 0);

        piojo_graph_free(copy);
        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1,v2=2;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_link(0, v, v2, graph);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_clear(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1,v2=2;
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_clear(graph);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_insert(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1,v2=2;
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_delete(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1,v2=2;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_delete(v, graph);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_link(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_link(0, v, v2, graph);
        PIOJO_ASSERT(piojo_graph_linked(v,v2,graph));

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_unlink(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_link(0, v, v2, graph);
        PIOJO_ASSERT(piojo_graph_linked(v,v2,graph));
        piojo_graph_unlink(v, v2,graph);
        PIOJO_ASSERT(! piojo_graph_linked(v,v2,graph));

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_unlink_all(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_link(0, v, v2,graph);
        PIOJO_ASSERT(piojo_graph_linked(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_linked(v2,v,graph));

        piojo_graph_unlink_all(v, graph);
        PIOJO_ASSERT(! piojo_graph_linked(v,v2,graph));
        PIOJO_ASSERT(! piojo_graph_linked(v2,v,graph));

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_neighbors_undir(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1,v3=2;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_insert(v3,graph);

        piojo_graph_link(1, v, v2,graph);
        PIOJO_ASSERT(piojo_graph_linked(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_linked(v2,v,graph));

        piojo_graph_link(2, v2, v3,graph);
        PIOJO_ASSERT(piojo_graph_linked(v2,v3,graph));
        PIOJO_ASSERT(piojo_graph_linked(v3,v2,graph));

        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v,graph), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v,graph) == 1);

        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v2,graph) == 2);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v2,graph),v));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v2,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(1, v2,graph),v3));
        PIOJO_ASSERT(piojo_graph_edge_weight(1, v2,graph) == 2);

        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v3,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v3,graph),v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v3,graph) == 2);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_neighbors_dir(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1,v3=2;
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_insert(v3,graph);

        piojo_graph_link(1, v, v2, graph);
        PIOJO_ASSERT(piojo_graph_linked(v,v2, graph));
        PIOJO_ASSERT(! piojo_graph_linked(v2,v, graph));

        piojo_graph_link(2, v2, v3, graph);
        PIOJO_ASSERT(piojo_graph_linked(v2,v3, graph));
        PIOJO_ASSERT(! piojo_graph_linked(v3,v2, graph));

        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v, graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, graph),v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 1);

        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v2, graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v2, graph),v3));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v2, graph) == 2);

        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v3, graph) == 0);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_bfs(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2;
        size_t i,j,cnt=0;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_set_gvalue((piojo_opaque_t)&cnt, graph);
        while (v < 10){
                piojo_graph_insert(v,graph);
                cnt += (size_t) v;
                ++v;
        }

        for (i = 0; i < 10; ++i){
                v = (piojo_graph_vid_t) i;
                for (j = 0; j < 10; ++j){
                        v2 = (piojo_graph_vid_t) j;
                        piojo_graph_link(1, v, v2, graph);
                }
        }
        piojo_graph_breadth_first(0, vvisit, 0, graph);
        PIOJO_ASSERT(cnt == 0);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_dfs(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2;
        size_t i,j,cnt=0;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_set_gvalue((piojo_opaque_t)&cnt, graph);

        while (v < 10){
                piojo_graph_insert(v,graph);
                cnt += (size_t) v;
                ++v;
        }

        for (i = 0; i < 10; ++i){
                v = (piojo_graph_vid_t) i;
                for (j = 0; j < 10; ++j){
                        v2 = (piojo_graph_vid_t) j;
                        piojo_graph_link(1, v, v2, graph);
                }
        }
        piojo_graph_depth_first(0, vvisit, 0, graph);
        PIOJO_ASSERT(cnt == 0);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_source_path(void)
{
        piojo_graph_t *graph;
        piojo_graph_weight_t *w;
        piojo_graph_vid_t v=1,*vp;
        piojo_hash_t *dists, *prevs;

        dists = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));
        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        while (v < 7){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(14, 1, 6, graph);
        piojo_graph_link(9,  1, 3, graph);
        piojo_graph_link(7,  1, 2, graph);
        piojo_graph_link(10, 2, 3, graph);
        piojo_graph_link(15, 4, 2, graph);
        piojo_graph_link(11, 4, 3, graph);
        piojo_graph_link(6,  4, 5, graph);
        piojo_graph_link(9,  6, 5, graph);
        piojo_graph_link(2,  6, 3, graph);

        v = 1;
        piojo_graph_source_path(v, graph, dists, prevs);

        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 0);
        v = 2;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 7);
        v = 3;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 9);
        v = 4;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 20);
        v = 5;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 20);
        v = 6;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 11);

        v = 1;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(vp == NULL);
        v = 2;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);
        v = 3;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);
        v = 4;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 3);
        v = 5;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 6);
        v = 6;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 3);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_pair_path(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1, *vp;
        piojo_hash_t *prevs;

        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        while (v < 7){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(14, 1, 6, graph);
        piojo_graph_link(9,  1, 3, graph);
        piojo_graph_link(7,  1, 2, graph);
        piojo_graph_link(10, 2, 3, graph);
        piojo_graph_link(15, 4, 2, graph);
        piojo_graph_link(11, 4, 3, graph);
        piojo_graph_link(6,  4, 5, graph);
        piojo_graph_link(9,  6, 5, graph);
        piojo_graph_link(2,  6, 3, graph);

        v = 1;
        PIOJO_ASSERT(piojo_graph_pair_path(v, 6, graph, prevs) == 11);

        v = 6;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 3);
        v = 3;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);

        piojo_graph_free(graph);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}


void test_neg_source_path(void)
{
        piojo_graph_t *graph;
        piojo_graph_weight_t *w;
        piojo_graph_vid_t v=1, *vp;
        piojo_hash_t *dists, *prevs;

        dists = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));
        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        while (v < 6){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(-3, 1, 2, graph);
        piojo_graph_link(1, 2, 3, graph);
        piojo_graph_link(1, 3, 4, graph);
        piojo_graph_link(1, 4, 5, graph);

        v = 1;
        PIOJO_ASSERT(piojo_graph_neg_source_path(v, graph, dists, prevs) ==
                     FALSE);
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 0);
        v = 2;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == -3);
        v = 3;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == -2);
        v = 4;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == -1);
        v = 5;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 0);

        v = 1;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(vp == NULL);
        v = 2;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);
        v = 3;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 2);
        v = 4;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 3);
        v = 5;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 4);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_neg_source_path_2(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1;
        piojo_hash_t *dists, *prevs;

        dists = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));
        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        while (v < 2){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(-3, 1, 1, graph);

        v = 1;
        PIOJO_ASSERT(piojo_graph_neg_source_path(v, graph, dists, prevs) ==
                     TRUE);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_neg_source_path_3(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1;
        piojo_hash_t *dists, *prevs;

        dists = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));
        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        while (v < 3){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(-3, 1, 2, graph);
        piojo_graph_link(-1, 2, 1, graph);

        v = 1;
        PIOJO_ASSERT(piojo_graph_neg_source_path(v, graph, dists, prevs) ==
                     TRUE);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_neg_source_path_4(void)
{
        piojo_graph_t *graph;
        piojo_graph_weight_t *w;
        piojo_graph_vid_t v=1, *vp;
        piojo_hash_t *dists, *prevs;

        dists = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));
        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        while (v < 7){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(14, 1, 6, graph);
        piojo_graph_link(9,  1, 3, graph);
        piojo_graph_link(7,  1, 2, graph);
        piojo_graph_link(10, 2, 3, graph);
        piojo_graph_link(15, 4, 2, graph);
        piojo_graph_link(11, 4, 3, graph);
        piojo_graph_link(6,  4, 5, graph);
        piojo_graph_link(9,  6, 5, graph);
        piojo_graph_link(2,  6, 3, graph);

        v = 1;
        PIOJO_ASSERT(piojo_graph_neg_source_path(v, graph, dists, prevs) ==
                     FALSE);

        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 0);
        v = 2;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 7);
        v = 3;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 9);
        v = 4;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 20);
        v = 5;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 20);
        v = 6;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 11);

        v = 1;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(vp == NULL);
        v = 2;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);
        v = 3;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);
        v = 4;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 3);
        v = 5;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 6);
        v = 6;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 3);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_min_tree(void)
{
        piojo_graph_t *graph, *tree;
        piojo_graph_vid_t v=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        while (v < 8){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(5, 4, 1, graph);
        piojo_graph_link(9, 4, 2, graph);
        piojo_graph_link(15, 4, 5, graph);
        piojo_graph_link(6, 4, 6, graph);

        piojo_graph_link(7, 2, 1, graph);
        piojo_graph_link(8, 2, 3, graph);
        piojo_graph_link(7, 2, 5, graph);

        piojo_graph_link(5, 5, 3, graph);
        piojo_graph_link(8, 5, 6, graph);
        piojo_graph_link(9, 5, 7, graph);

        piojo_graph_link(11, 6, 7, graph);

        tree = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        PIOJO_ASSERT(piojo_graph_min_tree(graph, tree) == 39);

        PIOJO_ASSERT(*piojo_graph_linked(1, 2, tree) == 7);
        PIOJO_ASSERT(*piojo_graph_linked(1, 4, tree) == 5);
        PIOJO_ASSERT(*piojo_graph_linked(2, 5, tree) == 7);
        PIOJO_ASSERT(*piojo_graph_linked(5, 3, tree) == 5);
        PIOJO_ASSERT(*piojo_graph_linked(5, 7, tree) == 9);
        PIOJO_ASSERT(*piojo_graph_linked(4, 6, tree) == 6);

        PIOJO_ASSERT(piojo_graph_linked(2, 3, tree) == NULL);
        PIOJO_ASSERT(piojo_graph_linked(4, 2, tree) == NULL);
        PIOJO_ASSERT(piojo_graph_linked(4, 5, tree) == NULL);
        PIOJO_ASSERT(piojo_graph_linked(6, 5, tree) == NULL);
        PIOJO_ASSERT(piojo_graph_linked(6, 7, tree) == NULL);

        piojo_graph_free(graph);
        piojo_graph_free(tree);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

piojo_graph_weight_t
heuristic (piojo_graph_vid_t from, piojo_graph_vid_t to,
           const piojo_graph_t *graph)
{
        PIOJO_UNUSED(to); PIOJO_UNUSED(graph);
        if(from == 3 || from == 6)
                return 100;
        return 1;
}

void test_a_star(void)
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=1, *vp;
        piojo_hash_t *prevs;

        prevs = piojo_hash_alloc_eq(sizeof(piojo_graph_vid_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        while (v < 7){
                piojo_graph_insert(v,graph);
                ++v;
        }

        piojo_graph_link(14, 1, 6, graph);
        piojo_graph_link(9,  1, 3, graph);
        piojo_graph_link(7,  1, 2, graph);
        piojo_graph_link(10, 2, 3, graph);
        piojo_graph_link(15, 4, 2, graph);
        piojo_graph_link(11, 4, 3, graph);
        piojo_graph_link(6,  4, 5, graph);
        piojo_graph_link(9,  6, 5, graph);
        piojo_graph_link(2,  6, 3, graph);

        v = 1;
        PIOJO_ASSERT(piojo_graph_a_star(v, 5, heuristic, graph, prevs) == 28);

        v = 5;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 4);
        v = 4;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 2);
        v = 2;
        vp = (piojo_graph_vid_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*vp == 1);

        piojo_graph_free(graph);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_sort(void)
{
        piojo_graph_t *graph;
        piojo_array_t *vertices;

        vertices = piojo_array_alloc_cb(sizeof(piojo_graph_vid_t),
                                        piojo_alloc_default);
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);

        piojo_graph_insert(5,graph);
        piojo_graph_insert(7,graph);
        piojo_graph_insert(3,graph);
        piojo_graph_insert(8,graph);
        piojo_graph_insert(11,graph);
        piojo_graph_insert(2,graph);
        piojo_graph_insert(9,graph);
        piojo_graph_insert(10,graph);
        piojo_graph_insert(123,graph);

        piojo_graph_link(100, 7, 11, graph);
        piojo_graph_link(100, 7, 8, graph);
        piojo_graph_link(100, 5, 11, graph);
        piojo_graph_link(100, 3, 8, graph);
        piojo_graph_link(100, 3, 10, graph);
        piojo_graph_link(100, 11, 2, graph);
        piojo_graph_link(100, 11, 9, graph);
        piojo_graph_link(100, 11, 10, graph);
        piojo_graph_link(100, 8, 9, graph);

        PIOJO_ASSERT(piojo_graph_sort(graph, vertices) ==
                     FALSE);

        piojo_graph_free(graph);
        piojo_array_free(vertices);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_sort_2(void)
{
        piojo_graph_t *graph;
        piojo_array_t *vertices;

        vertices = piojo_array_alloc_cb(sizeof(piojo_graph_vid_t),
                                        piojo_alloc_default);
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);

        piojo_graph_insert(5,graph);
        piojo_graph_insert(7,graph);
        piojo_graph_insert(3,graph);
        piojo_graph_insert(8,graph);
        piojo_graph_insert(11,graph);
        piojo_graph_insert(2,graph);
        piojo_graph_insert(9,graph);
        piojo_graph_insert(10,graph);
        piojo_graph_insert(123,graph);

        piojo_graph_link(100, 7, 11, graph);
        piojo_graph_link(100, 7, 8, graph);
        piojo_graph_link(100, 5, 11, graph);
        piojo_graph_link(100, 3, 8, graph);
        piojo_graph_link(100, 3, 10, graph);
        piojo_graph_link(100, 11, 2, graph);
        piojo_graph_link(100, 11, 9, graph);
        piojo_graph_link(100, 11, 10, graph);
        piojo_graph_link(100, 8, 9, graph);
        piojo_graph_link(100, 9, 9, graph);

        PIOJO_ASSERT(piojo_graph_sort(graph, vertices) ==
                     TRUE);

        piojo_graph_free(graph);
        piojo_array_free(vertices);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_strongly_connected(void)
{
        piojo_graph_t *graph;
        piojo_hash_t *scc;
        piojo_graph_vid_t v;
        piojo_graph_weight_t *c1, *c2, *c3, components[4];

        scc = piojo_hash_alloc_eq(sizeof(piojo_graph_weight_t),
                                    piojo_graph_vid_eq,
                                    sizeof(piojo_graph_vid_t));
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);

        piojo_graph_insert(1,graph);
        piojo_graph_insert(2,graph);
        piojo_graph_insert(3,graph);
        piojo_graph_insert(4,graph);
        piojo_graph_insert(5,graph);
        piojo_graph_insert(6,graph);
        piojo_graph_insert(7,graph);
        piojo_graph_insert(8,graph);

        piojo_graph_link(100, 1, 2, graph);
        piojo_graph_link(100, 2, 3, graph);
        piojo_graph_link(100, 3, 1, graph);
        piojo_graph_link(100, 4, 2, graph);
        piojo_graph_link(100, 4, 3, graph);
        piojo_graph_link(100, 4, 5, graph);
        piojo_graph_link(100, 5, 4, graph);
        piojo_graph_link(100, 5, 6, graph);
        piojo_graph_link(100, 6, 3, graph);
        piojo_graph_link(100, 6, 7, graph);
        piojo_graph_link(100, 7, 6, graph);
        piojo_graph_link(100, 8, 5, graph);
        piojo_graph_link(100, 8, 7, graph);
        piojo_graph_link(100, 8, 8, graph);

        piojo_graph_strongly_connected(graph, scc);

        PIOJO_ASSERT(piojo_hash_size(scc) == 8);

        v = 1;
        c1 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        v = 2;
        c2 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        v = 3;
        c3 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        PIOJO_ASSERT(c1 != NULL && c2 != NULL && c3 != NULL && *c1 == *c2 && *c1 == *c3);
        components[0] = *c1;

        v = 4;
        c1 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        v = 5;
        c2 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        PIOJO_ASSERT(c1 != NULL && c2 != NULL && *c1 == *c2);
        components[1] = *c1;

        v = 6;
        c1 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        v = 7;
        c2 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        PIOJO_ASSERT(c1 != NULL && c2 != NULL && *c1 == *c2);
        components[2] = *c1;

        v = 8;
        c1 = (piojo_graph_weight_t *)piojo_hash_search(&v, scc);
        PIOJO_ASSERT(c1 != NULL);
        components[3] = *c1;

        PIOJO_ASSERT(components[0] != components[1] &&
                components[0] != components[2] &&
                components[0] != components[3] &&
                components[1] != components[2] &&
                components[1] != components[3] &&
                components[2] != components[3]);

        piojo_graph_free(graph);
        piojo_hash_free(scc);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

int main(void)
{
        test_alloc();
        test_copy();
        test_free();
        test_clear();
        test_insert();
        test_delete();
        test_link();
        test_unlink();
        test_unlink_all();
        test_neighbors_undir();
        test_neighbors_dir();
        test_bfs();
        test_dfs();
        test_source_path();
        test_pair_path();
        test_neg_source_path();
        test_neg_source_path_2();
        test_neg_source_path_3();
        test_neg_source_path_4();
        test_min_tree();
        test_a_star();
        test_sort();
        test_sort_2();
        test_strongly_connected();

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
