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
 */

/**
 * @file
 * @addtogroup piojodiset Piojo Disjoint-set
 * @{
 * Piojo Disjoint-set implementation.
 */

#include <piojo/piojo_diset.h>
#include <piojo/piojo_hash.h>
#include <piojo_defs.h>

struct piojo_diset_set;
typedef struct piojo_diset_tree piojo_diset_tree_t;
struct piojo_diset_tree {
        size_t rank;            /* Height estimate. */
        piojo_diset_sid_t parent;
};

struct piojo_diset {
        piojo_hash_t *trees;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of diset in bytes */
const size_t piojo_diset_sizeof = sizeof(piojo_diset_t);

static piojo_diset_tree_t*
find_subset(piojo_diset_sid_t set, const piojo_diset_t *diset);

/**
 * Allocates a new diset.
 * Uses default allocator.
 * @return New diset.
 */
piojo_diset_t*
piojo_diset_alloc(void)
{
        return piojo_diset_alloc_cb(piojo_alloc_default);
}

/**
 * Allocates a new diset.
 * @param[in] allocator Allocator to be used.
 * @return New diset.
 */
piojo_diset_t*
piojo_diset_alloc_cb(piojo_alloc_if allocator)
{
        piojo_diset_t * diset;
        piojo_alloc_kv_if ator = piojo_alloc_kv_default;
        size_t esize;

        diset = (piojo_diset_t *) allocator.alloc_cb(sizeof(piojo_diset_t));
        PIOJO_ASSERT(diset);

        ator.alloc_cb = allocator.alloc_cb;
        ator.free_cb = allocator.free_cb;
        esize = sizeof(piojo_diset_tree_t);

        diset->allocator = allocator;
        diset->trees = piojo_hash_alloc_cb_eq(esize, piojo_id_eq,
                                              sizeof(piojo_diset_sid_t), ator);

        return diset;
}

/**
 * Copies @a diset and all its sets.
 * @param[in] diset Diset being copied.
 * @return New diset.
 */
piojo_diset_t*
piojo_diset_copy(const piojo_diset_t *diset)
{
        piojo_alloc_if allocator;
        piojo_diset_t *newdiset;
        PIOJO_ASSERT(diset);

        allocator = diset->allocator;
        newdiset = (piojo_diset_t *) allocator.alloc_cb(sizeof(piojo_diset_t));
        PIOJO_ASSERT(newdiset);

        newdiset->allocator = diset->allocator;
        newdiset->trees = piojo_hash_copy(diset->trees);

        return newdiset;
}

/**
 * Frees @a diset and all its sets.
 * @param[in] diset Diset being freed.
 */
void
piojo_diset_free(const piojo_diset_t *diset)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(diset);

        piojo_hash_free(diset->trees);

        allocator = diset->allocator;
        allocator.free_cb(diset);
}

/**
 * Deletes all sets in @a diset.
 * @param[out] diset Diset being cleared.
 */
void
piojo_diset_clear(piojo_diset_t *diset)
{
        PIOJO_ASSERT(diset);
        piojo_hash_clear(diset->trees);
}

/**
 * Inserts new set to diset.
 * @param[in] set Set.
 * @param[out] diset Diset.
 */
void
piojo_diset_insert(piojo_diset_sid_t set, piojo_diset_t *diset)
{
        piojo_diset_tree_t dtree;
        PIOJO_ASSERT(diset);

        dtree.rank = 0;
        dtree.parent = set;
        piojo_hash_insert(&set, &dtree, diset->trees);
}

/**
 * Finds the subset that @a set is member of.
 * @param[in] set Set.
 * @param[out] diset Diset.
 * @return @a set subset.
 */
piojo_diset_sid_t
piojo_diset_find(piojo_diset_sid_t set, const piojo_diset_t *diset)
{
        piojo_diset_tree_t *dtree;
        PIOJO_ASSERT(diset);

        dtree = find_subset(set, diset);
        return dtree->parent;
}

/**
 * Joins two sets in diset.
 * @param[in] set1 Set.
 * @param[in] set2 Set.
 * @param[out] diset Diset.
 */
void
piojo_diset_union(piojo_diset_sid_t set1, piojo_diset_sid_t set2,
                  piojo_diset_t *diset)
{
        piojo_diset_tree_t *dtree1, *dtree2;
        PIOJO_ASSERT(diset);

        dtree1 = find_subset(set1, diset);
        dtree2 = find_subset(set2, diset);
        if (dtree1->parent != dtree2->parent){
                if (dtree1->rank < dtree2->rank){
                        dtree1->parent = dtree2->parent;
                }else if(dtree1->rank > dtree2->rank){
                        dtree2->parent = dtree1->parent;
                }else{
                        dtree2->parent = dtree1->parent;
                        ++dtree1->rank;
                }
        }
}

/** @}
 * Private functions.
 */

static piojo_diset_tree_t*
find_subset(piojo_diset_sid_t set, const piojo_diset_t *diset)
{
        piojo_diset_tree_t *dtree;
        dtree = (piojo_diset_tree_t*) piojo_hash_search(&set, diset->trees);
        if (dtree->parent == set){
                return dtree;
        }
        return find_subset(dtree->parent, diset);
}
