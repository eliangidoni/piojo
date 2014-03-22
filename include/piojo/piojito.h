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
 * Piojo abbreviated public API.
 */

/**
 * @file
 * @addtogroup piojopiojito Piojito
 * @{
 * Piojo abbreviated public API.
 *
 * Include this file if you want to avoid the 'piojo' prefix from functions.
 * e.g: piojo_array_alloc() is also available as array_alloc()
 */

#ifndef PIOJO_PIOJITO_H_
#define PIOJO_PIOJITO_H_

/* Alloc */
#define alloc_def_alloc piojo_alloc_def_alloc
#define alloc_def_free piojo_alloc_def_free
#define alloc_def_init piojo_alloc_def_init
#define alloc_def_copy piojo_alloc_def_copy
#define alloc_def_finish piojo_alloc_def_finish

/* Array */
#define array_alloc piojo_array_alloc
#define array_alloc_s piojo_array_alloc_s
#define array_alloc_n piojo_array_alloc_n
#define array_alloc_cb piojo_array_alloc_cb
#define array_alloc_cb_n piojo_array_alloc_cb_n
#define array_copy piojo_array_copy
#define array_free piojo_array_free
#define array_clear piojo_array_clear
#define array_size piojo_array_size
#define array_insert piojo_array_insert
#define array_set piojo_array_set
#define array_push piojo_array_push
#define array_index piojo_array_index
#define array_sorted_index piojo_array_sorted_index
#define array_sorted_insert piojo_array_sorted_insert
#define array_delete piojo_array_delete
#define array_pop piojo_array_pop
#define array_at piojo_array_at
#define array_first piojo_array_first
#define array_last piojo_array_last

/* Bitset */
#define bitset_alloc piojo_bitset_alloc
#define bitset_alloc_cb piojo_bitset_alloc_cb
#define bitset_copy piojo_bitset_copy
#define bitset_free piojo_bitset_free
#define bitset_clear piojo_bitset_clear
#define bitset_size piojo_bitset_size
#define bitset_count piojo_bitset_count
#define bitset_empty_p piojo_bitset_empty_p
#define bitset_full_p piojo_bitset_full_p
#define bitset_equal_p piojo_bitset_equal_p
#define bitset_set_p piojo_bitset_set_p
#define bitset_set piojo_bitset_set
#define bitset_toggle piojo_bitset_toggle
#define bitset_unset piojo_bitset_unset
#define bitset_not piojo_bitset_not
#define bitset_or piojo_bitset_or
#define bitset_xor piojo_bitset_xor
#define bitset_and piojo_bitset_and
#define bitset_diff piojo_bitset_diff
#define bitset_lshift piojo_bitset_lshift
#define bitset_rshift piojo_bitset_rshift

/* Diset */
#define diset_alloc piojo_diset_alloc
#define diset_alloc_cb piojo_diset_alloc_cb
#define diset_copy piojo_diset_copy
#define diset_free piojo_diset_free
#define diset_clear piojo_diset_clear
#define diset_insert piojo_diset_insert
#define diset_find piojo_diset_find
#define diset_union piojo_diset_union

/* Graph */
#define graph_alloc piojo_graph_alloc
#define graph_alloc_cb piojo_graph_alloc_cb
#define graph_copy piojo_graph_copy
#define graph_free piojo_graph_free
#define graph_clear piojo_graph_clear
#define graph_insert piojo_graph_insert
#define graph_delete piojo_graph_delete
#define graph_set_vvalue piojo_graph_set_vvalue
#define graph_vvalue piojo_graph_vvalue
#define graph_link piojo_graph_link
#define graph_linked piojo_graph_linked
#define graph_unlink piojo_graph_unlink
#define graph_unlink_all piojo_graph_unlink_all
#define graph_neighbor_cnt piojo_graph_neighbor_cnt
#define graph_neighbor_at piojo_graph_neighbor_at
#define graph_edge_weight piojo_graph_edge_weight
#define graph_vid_eq piojo_graph_vid_eq
#define graph_breadth_first piojo_graph_breadth_first
#define graph_depth_first piojo_graph_depth_first
#define graph_source_path piojo_graph_source_path
#define graph_pair_path piojo_graph_pair_path
#define graph_neg_source_path piojo_graph_neg_source_path
#define graph_min_tree piojo_graph_min_tree
#define graph_a_star piojo_graph_a_star
#define graph_sort piojo_graph_sort

/* Hash */
#define hash_alloc_intk piojo_hash_alloc_intk
#define hash_alloc_i32k piojo_hash_alloc_i32k
#define hash_alloc_i64k piojo_hash_alloc_i64k
#define hash_alloc_strk piojo_hash_alloc_strk
#define hash_alloc_sizk piojo_hash_alloc_sizk
#define hash_alloc_cb_intk piojo_hash_alloc_cb_intk
#define hash_alloc_cb_i32k piojo_hash_alloc_cb_i32k
#define hash_alloc_cb_i64k piojo_hash_alloc_cb_i64k
#define hash_alloc_cb_strk piojo_hash_alloc_cb_strk
#define hash_alloc_cb_sizk piojo_hash_alloc_cb_sizk
#define hash_alloc_eq piojo_hash_alloc_eq
#define hash_alloc_cb_eq piojo_hash_alloc_cb_eq
#define hash_copy piojo_hash_copy
#define hash_free piojo_hash_free
#define hash_clear piojo_hash_clear
#define hash_size piojo_hash_size
#define hash_insert piojo_hash_insert
#define hash_set piojo_hash_set
#define hash_search piojo_hash_search
#define hash_delete piojo_hash_delete
#define hash_first piojo_hash_first
#define hash_next piojo_hash_next
#define hash_entryk piojo_hash_entryk
#define hash_entryv piojo_hash_entryv

/* Heap */
#define heap_alloc piojo_heap_alloc
#define heap_alloc_n piojo_heap_alloc_n
#define heap_alloc_cb piojo_heap_alloc_cb
#define heap_alloc_cb_n piojo_heap_alloc_cb_n
#define heap_copy piojo_heap_copy
#define heap_free piojo_heap_free
#define heap_clear piojo_heap_clear
#define heap_size piojo_heap_size
#define heap_push piojo_heap_push
#define heap_decrease piojo_heap_decrease
#define heap_pop piojo_heap_pop
#define heap_peek piojo_heap_peek
#define heap_contain_p piojo_heap_contain_p

/* List */
#define list_alloc piojo_list_alloc
#define list_alloc_s piojo_list_alloc_s
#define list_alloc_cb piojo_list_alloc_cb
#define list_copy piojo_list_copy
#define list_free piojo_list_free
#define list_clear piojo_list_clear
#define list_size piojo_list_size
#define list_insert piojo_list_insert
#define list_set piojo_list_set
#define list_prepend piojo_list_prepend
#define list_append piojo_list_append
#define list_delete piojo_list_delete
#define list_first piojo_list_first
#define list_last piojo_list_last
#define list_next piojo_list_next
#define list_prev piojo_list_prev
#define list_entry piojo_list_entry

/* Piojo */
#define opaque_eq piojo_opaque_eq
#define id_eq piojo_id_eq
#define safe_adduint_p piojo_safe_adduint_p
#define safe_addsiz_p piojo_safe_addsiz_p
#define safe_addint_p piojo_safe_addint_p
#define safe_subuint_p piojo_safe_subuint_p
#define safe_subsiz_p piojo_safe_subsiz_p
#define safe_subint_p piojo_safe_subint_p
#define safe_muluint_p piojo_safe_muluint_p
#define safe_mulsiz_p piojo_safe_mulsiz_p
#define maxsiz piojo_maxsiz
#define maxuint piojo_maxuint
#define maxint piojo_maxint
#define minsiz piojo_minsiz
#define minuint piojo_minuint
#define minint piojo_minint
#define clampsiz piojo_clampsiz
#define clampuint piojo_clampuint
#define clampint piojo_clampint

/* Ring */
#define ring_alloc piojo_ring_alloc
#define ring_alloc_s piojo_ring_alloc_s
#define ring_alloc_cb piojo_ring_alloc_cb
#define ring_copy piojo_ring_copy
#define ring_free piojo_ring_free
#define ring_clear piojo_ring_clear
#define ring_size piojo_ring_size
#define ring_full_p piojo_ring_full_p
#define ring_push piojo_ring_push
#define ring_pop piojo_ring_pop
#define ring_peek piojo_ring_peek

/* Stream */
#define stream_alloc piojo_stream_alloc
#define stream_alloc_n piojo_stream_alloc_n
#define stream_alloc_cb piojo_stream_alloc_cb
#define stream_alloc_cb_n piojo_stream_alloc_cb_n
#define stream_copy piojo_stream_copy
#define stream_free piojo_stream_free
#define stream_clear piojo_stream_clear
#define stream_size piojo_stream_size
#define stream_data piojo_stream_data
#define stream_concat piojo_stream_concat
#define stream_readu64 piojo_stream_readu64
#define stream_writeu64 piojo_stream_writeu64
#define stream_readi64 piojo_stream_readi64
#define stream_writei64 piojo_stream_writei64
#define stream_readu32 piojo_stream_readu32
#define stream_writeu32 piojo_stream_writeu32
#define stream_readi32 piojo_stream_readi32
#define stream_writei32 piojo_stream_writei32
#define stream_readu16 piojo_stream_readu16
#define stream_writeu16 piojo_stream_writeu16
#define stream_readi16 piojo_stream_readi16
#define stream_writei16 piojo_stream_writei16
#define stream_readu8 piojo_stream_readu8
#define stream_writeu8 piojo_stream_writeu8
#define stream_readi8 piojo_stream_readi8
#define stream_writei8 piojo_stream_writei8
#define stream_readstr piojo_stream_readstr
#define stream_writestr piojo_stream_writestr
#define stream_readbool piojo_stream_readbool
#define stream_writebool piojo_stream_writebool

/* Tree */
#define tree_alloc_intk piojo_tree_alloc_intk
#define tree_alloc_uintk piojo_tree_alloc_uintk
#define tree_alloc_i32k piojo_tree_alloc_i32k
#define tree_alloc_u32k piojo_tree_alloc_u32k
#define tree_alloc_i64k piojo_tree_alloc_i64k
#define tree_alloc_u64k piojo_tree_alloc_u64k
#define tree_alloc_strk piojo_tree_alloc_strk
#define tree_alloc_sizk piojo_tree_alloc_sizk
#define tree_alloc_cb_intk piojo_tree_alloc_cb_intk
#define tree_alloc_cb_uintk piojo_tree_alloc_cb_uintk
#define tree_alloc_cb_i32k piojo_tree_alloc_cb_i32k
#define tree_alloc_cb_u32k piojo_tree_alloc_cb_u32k
#define tree_alloc_cb_i64k piojo_tree_alloc_cb_i64k
#define tree_alloc_cb_u64k piojo_tree_alloc_cb_u64k
#define tree_alloc_cb_strk piojo_tree_alloc_cb_strk
#define tree_alloc_cb_sizk piojo_tree_alloc_cb_sizk
#define tree_alloc_cmp piojo_tree_alloc_cmp
#define tree_alloc_cb_cmp piojo_tree_alloc_cb_cmp
#define tree_copy piojo_tree_copy
#define tree_free piojo_tree_free
#define tree_clear piojo_tree_clear
#define tree_size piojo_tree_size
#define tree_insert piojo_tree_insert
#define tree_set piojo_tree_set
#define tree_search piojo_tree_search
#define tree_delete piojo_tree_delete
#define tree_first piojo_tree_first
#define tree_last piojo_tree_last
#define tree_next piojo_tree_next
#define tree_prev piojo_tree_prev
#define tree_entryk piojo_tree_entryk
#define tree_entryv piojo_tree_entryv

/** @} */

#endif
