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

bool vvisit (piojo_graph_vid_t v, const piojo_graph_t *graph, void *data)
{
        size_t *cnt = (size_t*)data;
        PIOJO_UNUSED(graph);
        *cnt -= (int) v;
        return FALSE;
}

void test_alloc()
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

void test_copy()
{
        piojo_graph_t *graph,*copy;
        piojo_graph_vid_t v=1,v2=2;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_link(0, v, v2, graph);

        copy = piojo_graph_copy(graph);
        PIOJO_ASSERT(copy);

        PIOJO_ASSERT(piojo_graph_link_p(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, graph), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 0);

        PIOJO_ASSERT(piojo_graph_link_p(v,v2,copy));
        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,copy) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, copy), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, copy) == 0);

        piojo_graph_link(1, v, v2, copy);
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, copy) == 1);
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 0);

        piojo_graph_unlink_all(v, copy);
        PIOJO_ASSERT(! piojo_graph_link_p(v,v2,copy));
        PIOJO_ASSERT(piojo_graph_link_p(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_neighbor_cnt(v,graph) == 1);
        PIOJO_ASSERT(veq(piojo_graph_neighbor_at(0, v, graph), v2));
        PIOJO_ASSERT(piojo_graph_edge_weight(0, v, graph) == 0);

        piojo_graph_free(copy);
        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_free()
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

void test_clear()
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

void test_insert()
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

void test_delete()
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

void test_link()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_link(0, v, v2, graph);
        PIOJO_ASSERT(piojo_graph_link_p(v,v2,graph));

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_unlink()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_link(0, v, v2, graph);
        PIOJO_ASSERT(piojo_graph_link_p(v,v2,graph));
        piojo_graph_unlink(v, v2,graph);
        PIOJO_ASSERT(! piojo_graph_link_p(v,v2,graph));

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_unlink_all()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);

        piojo_graph_link(0, v, v2,graph);
        PIOJO_ASSERT(piojo_graph_link_p(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_link_p(v2,v,graph));

        piojo_graph_unlink_all(v, graph);
        PIOJO_ASSERT(! piojo_graph_link_p(v,v2,graph));
        PIOJO_ASSERT(! piojo_graph_link_p(v2,v,graph));

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_neighbors_undir()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1,v3=2;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_insert(v3,graph);

        piojo_graph_link(1, v, v2,graph);
        PIOJO_ASSERT(piojo_graph_link_p(v,v2,graph));
        PIOJO_ASSERT(piojo_graph_link_p(v2,v,graph));

        piojo_graph_link(2, v2, v3,graph);
        PIOJO_ASSERT(piojo_graph_link_p(v2,v3,graph));
        PIOJO_ASSERT(piojo_graph_link_p(v3,v2,graph));

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

void test_neighbors_dir()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2=1,v3=2;
        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_TRUE, my_allocator);
        piojo_graph_insert(v,graph);
        piojo_graph_insert(v2,graph);
        piojo_graph_insert(v3,graph);

        piojo_graph_link(1, v, v2, graph);
        PIOJO_ASSERT(piojo_graph_link_p(v,v2, graph));
        PIOJO_ASSERT(! piojo_graph_link_p(v2,v, graph));

        piojo_graph_link(2, v2, v3, graph);
        PIOJO_ASSERT(piojo_graph_link_p(v2,v3, graph));
        PIOJO_ASSERT(! piojo_graph_link_p(v3,v2, graph));

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

void test_bfs()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2;
        size_t i,j,cnt=0;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
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
        piojo_graph_breadth_first(0, vvisit, &cnt, 0, graph);
        PIOJO_ASSERT(cnt == 0);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_dfs()
{
        piojo_graph_t *graph;
        piojo_graph_vid_t v=0,v2;
        size_t i,j,cnt=0;

        graph = piojo_graph_alloc_cb(PIOJO_GRAPH_DIR_FALSE, my_allocator);
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
        piojo_graph_depth_first(0, vvisit, &cnt, 0, graph);
        PIOJO_ASSERT(cnt == 0);

        piojo_graph_free(graph);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_source_path()
{
        piojo_graph_t *graph;
        piojo_graph_weight_t *w;
        piojo_graph_vid_t v=1;
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
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(w == NULL);
        v = 2;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 1);
        v = 3;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 1);
        v = 4;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 3);
        v = 5;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 6);
        v = 6;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 3);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

void test_pair_path()
{
        piojo_graph_t *graph;
        piojo_graph_weight_t *w;
        piojo_graph_vid_t v=1;
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
        piojo_graph_pair_path(v, 6, graph, dists, prevs);

        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 0);
        v = 6;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, dists);
        PIOJO_ASSERT(*w == 11);

        v = 6;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 3);
        v = 3;
        w = (piojo_graph_weight_t *)piojo_hash_search(&v, prevs);
        PIOJO_ASSERT(*w == 1);

        piojo_graph_free(graph);
        piojo_hash_free(dists);
        piojo_hash_free(prevs);
        assert_allocator_alloc(0);
        assert_allocator_init(0);
}

int main()
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

        assert_allocator_init(0);
        assert_allocator_alloc(0);

        return 0;
}
