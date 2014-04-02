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
 * @addtogroup piojotree Piojo B-tree
 * @{
 * Piojo B-tree implementation.
 * Use @a maxchildren >= @b 8 for good performance.
 */

#include <piojo/piojo_tree.h>
#include <piojo/piojo_list.h>
#include <piojo_defs.h>

typedef struct {
        void *key, *data;
} piojo_tree_entry_t;

struct piojo_tree_bnode;
typedef struct piojo_tree_bnode piojo_tree_bnode_t;
struct piojo_tree_bnode {
        bool leaf_p;
        uint8_t ecnt, pidx;
        piojo_tree_bnode_t **children, *parent;
        piojo_tree_entry_t *entries;
};

typedef struct {
        uint8_t eidx;
        piojo_tree_bnode_t *bnode;
} piojo_tree_iter_t;

struct piojo_tree {
        piojo_tree_bnode_t *root;
        size_t eksize, evsize, ecount, cmin, cmax;
        piojo_cmp_cb cmp_cb;
        piojo_alloc_kv_if allocator;
};
/** @hideinitializer Size of tree in bytes */
const size_t piojo_tree_sizeof = sizeof(piojo_tree_t);

static const uint8_t TREE_CHILDREN_MAX = 8;

static piojo_tree_bnode_t*
alloc_bnode(const piojo_tree_t *tree);

static void
free_bnode(const piojo_tree_bnode_t *bnode, const piojo_tree_t *tree);

static void
split_bnode(piojo_tree_t *tree, size_t pidx, piojo_tree_bnode_t *bnode,
            piojo_tree_bnode_t *parent);

static void
split_root(piojo_tree_t *tree);

static void
merge_bnodes(piojo_tree_t *tree, size_t pidx, piojo_tree_bnode_t *lbnode,
             piojo_tree_bnode_t *rbnode, piojo_tree_bnode_t *parent);

static void
rotate_left(size_t pidx, piojo_tree_bnode_t *lbnode,
            piojo_tree_bnode_t *rbnode, piojo_tree_bnode_t *parent);

static void
rotate_right(size_t pidx, piojo_tree_bnode_t *lbnode,
             piojo_tree_bnode_t *rbnode, piojo_tree_bnode_t *parent);

static piojo_tree_bnode_t*
rebalance_bnode(piojo_tree_t *tree, size_t pidx, piojo_tree_bnode_t *bnode,
                piojo_tree_bnode_t *parent);

static void
move_child(piojo_tree_bnode_t *child, uint8_t toidx, piojo_tree_bnode_t *to);

static piojo_tree_entry_t
init_entry(const void *key, const void *data, const piojo_tree_t *tree);

static piojo_tree_entry_t
copy_entry(const void *key, const void *data, const piojo_tree_t *tree);

static void
finish_entry(const piojo_tree_t *tree, piojo_tree_entry_t *kv);

static void
finish_all(const piojo_tree_t *tree);

static piojo_tree_entry_t*
search_min(piojo_tree_iter_t *from);

static piojo_tree_entry_t*
search_max(piojo_tree_iter_t *from);

static piojo_tree_entry_t*
insert_node(piojo_tree_entry_t kv, insert_t op, piojo_tree_t *tree);

static piojo_tree_iter_t
search_node(const void *key, const piojo_tree_t *tree);

static bool
delete_node(const void *key, piojo_tree_bnode_t *bnode, piojo_tree_t *tree);

static bool
delete_node_2(const void *key, piojo_tree_entry_t todelete,
              piojo_tree_bnode_t *bnode, piojo_tree_t *tree);

static size_t
bin_search(const void *key, piojo_cmp_cb cmp_cb, piojo_tree_bnode_t *bnode,
           bool *found_p);

static int
u32_cmp(const void *e1, const void *e2);

static int
i32_cmp(const void *e1, const void *e2);

static int
u64_cmp(const void *e1, const void *e2);

static int
i64_cmp(const void *e1, const void *e2);

static int
int_cmp(const void *e1, const void *e2);

static int
uint_cmp(const void *e1, const void *e2);

static int
ptr_cmp(const void *e1, const void *e2);

static int
siz_cmp(const void *e1, const void *e2);

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b int.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_intk(size_t evsize)
{
        return piojo_tree_alloc_cb_intk(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b unsigned @b int.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_uintk(size_t evsize)
{
        return piojo_tree_alloc_cb_uintk(TREE_CHILDREN_MAX, evsize,
                                         piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_i32k(size_t evsize)
{
        return piojo_tree_alloc_cb_i32k(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b uint32_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_u32k(size_t evsize)
{
        return piojo_tree_alloc_cb_u32k(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_i64k(size_t evsize)
{
        return piojo_tree_alloc_cb_i64k(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b uint64_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_u64k(size_t evsize)
{
        return piojo_tree_alloc_cb_u64k(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b uintptr_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_ptrk(size_t evsize)
{
        return piojo_tree_alloc_cb_ptrk(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_sizk(size_t evsize)
{
        return piojo_tree_alloc_cb_sizk(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * Uses key size of @b int.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_intk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       int_cmp, sizeof(int), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b unsigned @b int.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_uintk(uint8_t maxchildren, size_t evsize,
                          piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       uint_cmp, sizeof(unsigned int),
                                       allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b int32_t.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_i32k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       i32_cmp, sizeof(int32_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b uint32_t.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_u32k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       u32_cmp, sizeof(uint32_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b int64_t.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_i64k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       i64_cmp, sizeof(int64_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b uint64_t.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_u64k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       u64_cmp, sizeof(uint64_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b uintptr_t.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_ptrk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       ptr_cmp, sizeof(uintptr_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b size_t.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_sizk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_kv_if allocator)
{
        return piojo_tree_alloc_cb_cmp(maxchildren, evsize,
                                       siz_cmp, sizeof(size_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses default allocator.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] keycmp Entry key comparison function.
 * @param[in] eksize Entry key size.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize)
{
        return piojo_tree_alloc_cb_cmp(TREE_CHILDREN_MAX, evsize,
                                       keycmp, eksize, piojo_alloc_kv_default);
}

/**
 * Allocates a new tree.
 * @param[in] maxchildren Maximum children in each node (from 4 to 254,
 *            and multiple of 2).
 * @param[in] evsize Entry value size in bytes.
 * @param[in] keycmp Entry key comparison function.
 * @param[in] eksize Entry key size.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_cmp(uint8_t maxchildren, size_t evsize,
                        piojo_cmp_cb keycmp, size_t eksize,
                        piojo_alloc_kv_if allocator)
{
        piojo_tree_t * tree;
        PIOJO_ASSERT(sizeof(piojo_tree_node_t) >= sizeof(piojo_tree_iter_t));
        PIOJO_ASSERT(evsize > 0 && eksize > 0);
        PIOJO_ASSERT(maxchildren > 2 && maxchildren < 255 &&
                     maxchildren % 2 == 0);

        tree = (piojo_tree_t *) allocator.alloc_cb(sizeof(piojo_tree_t));
        PIOJO_ASSERT(tree);

        tree->allocator = allocator;
        tree->eksize = eksize;
        tree->evsize = evsize;
        tree->cmin = maxchildren / 2;
        tree->cmax = maxchildren;
        tree->ecount = 0;
        tree->cmp_cb = keycmp;
        tree->root = alloc_bnode(tree);
        tree->root->leaf_p = TRUE;

        return tree;
}

/**
 * Copies @a tree and all its entries.
 * @param[in] tree Tree being copied.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_copy(const piojo_tree_t *tree)
{
        piojo_tree_t *newtree;
        piojo_tree_entry_t kv;
        piojo_tree_node_t iter, *next;
        PIOJO_ASSERT(tree);

        newtree = piojo_tree_alloc_cb_cmp(tree->cmax, tree->evsize,
                                          tree->cmp_cb, tree->eksize,
                                          tree->allocator);
        newtree->ecount = tree->ecount;

        next = piojo_tree_first(tree, &iter);
        while (next != NULL){
                kv.key = (void*) piojo_tree_entryk(next);
                kv.data = (void*) piojo_tree_entryv(next);
                insert_node(kv, INSERT_COPY, newtree);
                next = piojo_tree_next(next);
        }

        return newtree;
}

/**
 * Frees @a tree and all its entries.
 * @param[in] tree Tree being freed.
 */
void
piojo_tree_free(const piojo_tree_t *tree)
{
        PIOJO_ASSERT(tree);

        finish_all(tree);

        tree->allocator.free_cb(tree);
}


/**
 * Deletes all entries in @a tree.
 * @param[out] tree Tree being cleared.
 */
void
piojo_tree_clear(piojo_tree_t *tree)
{
        PIOJO_ASSERT(tree);

        finish_all(tree);

        tree->root = alloc_bnode(tree);
        tree->root->leaf_p = TRUE;
        tree->ecount = 0;
}

/**
 * Returns number of entries.
 * @param[in] tree
 * @return Number of entries in @a tree.
 */
size_t
piojo_tree_size(const piojo_tree_t *tree)
{
        PIOJO_ASSERT(tree);
        return tree->ecount;
}

/**
 * Inserts a new entry.
 * If @a data is @b NULL, the value is replaced with @b TRUE (useful for sets).
 * @param[in] key Entry key.
 * @param[in] data Entry value.
 * @param[out] tree Tree being modified.
 * @return @b TRUE if inserted, @b FALSE if @a key is duplicate.
 */
bool
piojo_tree_insert(const void *key, const void *data, piojo_tree_t *tree)
{
        piojo_tree_entry_t kv;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(tree->ecount < SIZE_MAX);
        PIOJO_ASSERT(data || tree->evsize == sizeof(bool));

        kv.key = (void*) key;
        kv.data = (void*) data;
        if (insert_node(kv, INSERT_NEW, tree) == NULL){
                ++tree->ecount;
                return TRUE;
        }
        return FALSE;
}

/**
 * Replaces or inserts an entry.
 * If @a data is @b NULL, the value is replaced with @b TRUE (useful for sets).
 * @param[in] key Entry key.
 * @param[in] data Entry value.
 * @param[out] tree Tree being modified.
 * @return @b TRUE if @a key is new, @b FALSE otherwise.
 */
bool
piojo_tree_set(const void *key, const void *data, piojo_tree_t *tree)
{
        piojo_tree_entry_t kv, *oldkv;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(data || tree->evsize == sizeof(bool));

        kv.key = (void*) key;
        kv.data = (void*) data;
        oldkv = insert_node(kv, INSERT_NEW, tree);
        if (oldkv == NULL){
                PIOJO_ASSERT(tree->ecount < SIZE_MAX);
                ++tree->ecount;
                return TRUE;
        }

        if (data != NULL){
                tree->allocator.finish_cb(oldkv->data);
                tree->allocator.init_cb(data, tree->evsize, oldkv->data);
        }
        return FALSE;
}

/**
 * Searches an entry by key.
 * @param[in] key Entry key.
 * @param[in] tree
 * @return Entry value or @b NULL if key doesn't exist.
 */
void*
piojo_tree_search(const void *key, const piojo_tree_t *tree)
{
        piojo_tree_iter_t iter;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        iter = search_node(key, tree);
        if (iter.bnode != NULL){
                return iter.bnode->entries[iter.eidx].data;
        }
        return NULL;
}

/**
 * Deletes an entry by key.
 * @param[in] key Entry key.
 * @param[out] tree
 * @return TRUE if deleted, FALSE if @a key doesn't exist.
 */
bool
piojo_tree_delete(const void *key, piojo_tree_t *tree)
{
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        if (delete_node(key, tree->root, tree)){
                --tree->ecount;
                return TRUE;
        }
        return FALSE;
}

/**
 * Reads the first node in @a tree (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] node
 * @return @a node or @b NULL if @a tree is empty.
 */
piojo_tree_node_t*
piojo_tree_first(const piojo_tree_t *tree, piojo_tree_node_t *node)
{
        piojo_tree_iter_t iter, *nitr;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(node);

        if (tree->ecount > 0){
                nitr = (piojo_tree_iter_t*) node->opaque;
                iter.eidx = 0;
                iter.bnode = tree->root;
                search_min(&iter);
                *nitr = iter;
                return node;
        }
        return NULL;
}

/**
 * Reads the last node in @a tree (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] node
 * @return @a node or @b NULL if @a tree is empty.
 */
piojo_tree_node_t*
piojo_tree_last(const piojo_tree_t *tree, piojo_tree_node_t *node)
{
        piojo_tree_iter_t iter, *nitr;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(node);

        if (tree->ecount > 0){
                nitr = (piojo_tree_iter_t*) node->opaque;
                iter.eidx = tree->root->ecnt;
                iter.bnode = tree->root;
                search_max(&iter);
                *nitr = iter;
                return node;
        }
        return NULL;
}

/**
 * Reads the next node (order given by @a keycmp function).
 * @param[out] node
 * @return Next node or @b NULL if @a node is the last one.
 */
piojo_tree_node_t*
piojo_tree_next(piojo_tree_node_t *node)
{
        piojo_tree_iter_t *iter;
        PIOJO_ASSERT(node);

        iter = (piojo_tree_iter_t*) node->opaque;
        if (! iter->bnode->leaf_p && iter->eidx < iter->bnode->ecnt){
                iter->bnode = iter->bnode->children[iter->eidx + 1];
                iter->eidx = 0;
                search_min(iter);
        }else if (iter->eidx + 1 < iter->bnode->ecnt){
                ++iter->eidx;
        }else{
                while (iter->bnode->parent != NULL){
                        iter->eidx = iter->bnode->pidx;
                        iter->bnode = iter->bnode->parent;
                        if (iter->eidx < iter->bnode->ecnt){
                                return node;
                        }
                }
                return NULL;
        }
        return node;
}

/**
 * Reads the previous node (order given by @a keycmp function).
 * @param[out] node
 * @return Next node or @b NULL if @a node is the first one.
 */
piojo_tree_node_t*
piojo_tree_prev(piojo_tree_node_t *node)
{
        piojo_tree_iter_t *iter;
        PIOJO_ASSERT(node);

        iter = (piojo_tree_iter_t*) node->opaque;
        if (! iter->bnode->leaf_p && iter->eidx < iter->bnode->ecnt + 1){
                iter->bnode = iter->bnode->children[iter->eidx];
                iter->eidx = iter->bnode->ecnt;
                search_max(iter);
        }else if (iter->eidx > 0){
                --iter->eidx;
        }else{
                while (iter->bnode->parent != NULL){
                        iter->eidx = iter->bnode->pidx;
                        iter->bnode = iter->bnode->parent;
                        if (iter->eidx > 0){
                                --iter->eidx;
                                return node;
                        }
                }
                return NULL;
        }
        return node;
}

/**
 * Reads node entry key.
 * @param[in] node
 * @return Entry key.
 */
const void*
piojo_tree_entryk(const piojo_tree_node_t *node)
{
        piojo_tree_iter_t *iter;
        PIOJO_ASSERT(node);

        iter = (piojo_tree_iter_t*) node->opaque;
        return iter->bnode->entries[iter->eidx].key;
}

/**
 * Reads node entry value.
 * @param[in] node
 * @return Entry value.
 */
void*
piojo_tree_entryv(const piojo_tree_node_t *node)
{
        piojo_tree_iter_t *iter;
        PIOJO_ASSERT(node);

        iter = (piojo_tree_iter_t*) node->opaque;
        return iter->bnode->entries[iter->eidx].data;
}

/** @}
 * Private functions.
 */

static piojo_tree_bnode_t*
alloc_bnode(const piojo_tree_t *tree)
{
        piojo_alloc_kv_if ator = tree->allocator;
        piojo_tree_bnode_t *bnode;
        size_t entriessiz = sizeof(piojo_tree_entry_t) * (tree->cmax - 1);
        size_t childrensiz = sizeof(piojo_tree_bnode_t*) * tree->cmax;
        size_t bnodesiz = sizeof(piojo_tree_bnode_t);

        bnode = ((piojo_tree_bnode_t*)
                 ator.alloc_cb(bnodesiz + entriessiz + childrensiz));
        PIOJO_ASSERT(bnode);

        bnode->entries = ((piojo_tree_entry_t*)
                          ((uint8_t*) bnode + bnodesiz));
        bnode->children = ((piojo_tree_bnode_t**)
                           ((uint8_t*) bnode->entries + entriessiz));
        bnode->ecnt = bnode->pidx = 0;
        bnode->leaf_p = FALSE;
        bnode->parent = NULL;

        return bnode;
}

static void
free_bnode(const piojo_tree_bnode_t *bnode, const piojo_tree_t *tree)
{
        tree->allocator.free_cb(bnode);
}

static void
move_child(piojo_tree_bnode_t *child, uint8_t toidx, piojo_tree_bnode_t *to)
{
        to->children[toidx] = child;
        child->parent = to;
        child->pidx = toidx;
}

static void
split_root(piojo_tree_t *tree)
{
        piojo_tree_bnode_t *newroot;
        PIOJO_ASSERT(tree->root->ecnt == tree->cmax - 1);

        newroot = alloc_bnode(tree);
        newroot->leaf_p = FALSE;
        move_child(tree->root, 0, newroot);

        split_bnode(tree, 0, tree->root, newroot);

        tree->root = newroot;
}

static void
split_bnode(piojo_tree_t *tree, size_t pidx, piojo_tree_bnode_t *bnode,
            piojo_tree_bnode_t *parent)
{
        size_t i, j, mid = bnode->ecnt / 2;
        piojo_tree_bnode_t *rbnode;
        PIOJO_ASSERT(parent->ecnt < tree->cmax - 1);

        /* Copy children/entries greater than median to right bnode. */
        rbnode = alloc_bnode(tree);
        for (i = mid + 1; i < bnode->ecnt; ++i, ++rbnode->ecnt){
                rbnode->entries[rbnode->ecnt] = bnode->entries[i];
        }
        if (! bnode->leaf_p){
                for (j = 0, i = mid + 1; i <= bnode->ecnt; ++i, ++j){
                        move_child(bnode->children[i], j, rbnode);
                }
        }
        rbnode->leaf_p = bnode->leaf_p;

        /* Expand parent bnode. */
        for (i = parent->ecnt; i > pidx; --i){
                parent->entries[i] = parent->entries[i - 1];
        }
        for (i = parent->ecnt + 1; i > pidx + 1; --i){
                move_child(parent->children[i - 1], i, parent);
        }

        /* Move median to parent bnode and update children. */
        parent->entries[pidx] = bnode->entries[mid];
        PIOJO_ASSERT(parent->children[pidx] == bnode);
        move_child(rbnode, pidx + 1, parent);
        ++parent->ecnt;

        bnode->ecnt = mid;
}

static void
merge_bnodes(piojo_tree_t *tree, size_t pidx, piojo_tree_bnode_t *lbnode,
             piojo_tree_bnode_t *rbnode, piojo_tree_bnode_t *parent)
{
        size_t i, ccnt = lbnode->ecnt + 1;

        /* Append parent entry to left bnode. */
        lbnode->entries[lbnode->ecnt] = parent->entries[pidx];
        ++lbnode->ecnt;

        /* Append right bnode to left bnode. */
        for (i = 0; i < rbnode->ecnt; ++i, ++lbnode->ecnt){
                lbnode->entries[lbnode->ecnt] = rbnode->entries[i];
        }
        if (! rbnode->leaf_p){
                for (i = 0; i <= rbnode->ecnt; ++i, ++ccnt){
                        move_child(rbnode->children[i], ccnt, lbnode);
                }
        }

        /* Shrink parent bnode. */
        --parent->ecnt;
        for (i = pidx; i < parent->ecnt; ++i){
                parent->entries[i] = parent->entries[i + 1];
        }
        for (i = pidx + 1; i <= parent->ecnt; ++i){
                move_child(parent->children[i + 1], i, parent);
        }
        PIOJO_ASSERT(parent->children[pidx] == lbnode);

        /* Make left bnode the new root when the root is empty. */
        if (tree->root == parent && parent->ecnt == 0){
                free_bnode(tree->root, tree);
                tree->root = lbnode;
        }

        /* Free right bnode. */
        free_bnode(rbnode, tree);
}

static void
rotate_left(size_t pidx, piojo_tree_bnode_t *lbnode,
            piojo_tree_bnode_t *rbnode, piojo_tree_bnode_t *parent)
{
        size_t i;

        /* Append parent entry to left bnode. */
        lbnode->entries[lbnode->ecnt] = parent->entries[pidx];

        /* Append first child from right bnode to left bnode. */
        --rbnode->ecnt;
        if (! lbnode->leaf_p){
                move_child(rbnode->children[0], lbnode->ecnt + 1, lbnode);
                for (i = 0; i <= rbnode->ecnt; ++i){
                        move_child(rbnode->children[i + 1], i, rbnode);
                }
        }

        /* Move first entry from right bnode to parent bnode. */
        parent->entries[pidx] = rbnode->entries[0];
        for (i = 0; i < rbnode->ecnt; ++i){
                rbnode->entries[i] = rbnode->entries[i + 1];
        }

        ++lbnode->ecnt;
}

static void
rotate_right(size_t pidx, piojo_tree_bnode_t *lbnode,
             piojo_tree_bnode_t *rbnode, piojo_tree_bnode_t *parent)
{
        size_t i;

        /* Prepend parent entry to right bnode. */
        for (i = rbnode->ecnt; i > 0; --i){
                rbnode->entries[i] = rbnode->entries[i - 1];
        }
        rbnode->entries[0] = parent->entries[pidx];
        ++rbnode->ecnt;

        /* Prepend last child from left bnode to right bnode. */
        if (! rbnode->leaf_p){
                for (i = rbnode->ecnt; i > 0; --i){
                        move_child(rbnode->children[i - 1], i, rbnode);
                }
                move_child(lbnode->children[lbnode->ecnt], 0, rbnode);
        }

        /* Move last entry from left bnode to parent bnode. */
        parent->entries[pidx] = lbnode->entries[lbnode->ecnt - 1];

        --lbnode->ecnt;
}

static piojo_tree_entry_t
init_entry(const void *key, const void *data, const piojo_tree_t *tree)
{
        bool null_p = TRUE;
        piojo_tree_entry_t kv;
        piojo_alloc_kv_if ator = tree->allocator;
        size_t ksize = tree->eksize;

        if (data == NULL){
                data = &null_p;
        }

        PIOJO_ASSERT(piojo_safe_addsiz_p(ksize, tree->evsize));
        kv.key = ator.alloc_cb(ksize + tree->evsize);
        PIOJO_ASSERT(kv.key);
        ator.initk_cb(key, ksize, kv.key);

        kv.data = (uint8_t*)kv.key + ksize;
        ator.init_cb(data, tree->evsize, kv.data);

        return kv;
}

static piojo_tree_entry_t
copy_entry(const void *key, const void *data, const piojo_tree_t *tree)
{
        piojo_tree_entry_t kv;
        piojo_alloc_kv_if ator = tree->allocator;
        size_t ksize = tree->eksize;

        kv.key = ator.alloc_cb(ksize + tree->evsize);
        PIOJO_ASSERT(kv.key);
        ator.copyk_cb(key, ksize, kv.key);

        kv.data = (uint8_t*)kv.key + ksize;
        ator.copy_cb(data, tree->evsize, kv.data);

        return kv;
}

static void
finish_entry(const piojo_tree_t *tree, piojo_tree_entry_t *kv)
{
        piojo_alloc_kv_if ator = tree->allocator;

        ator.finishk_cb(kv->key);
        ator.finish_cb(kv->data);
        ator.free_cb(kv->key);
        kv->key = kv->data = NULL;
}

static void
finish_all(const piojo_tree_t *tree)
{
        piojo_alloc_kv_if allocator;
        piojo_alloc_if qator = piojo_alloc_default;
        piojo_list_t *q;
        piojo_list_node_t *qnode;
        piojo_tree_bnode_t *bnode;
        size_t i;

        allocator = tree->allocator;
        qator.alloc_cb = allocator.alloc_cb;
        qator.realloc_cb = allocator.realloc_cb;
        qator.free_cb = allocator.free_cb;

        /* Breadth-first traversal and free */
        q = piojo_list_alloc_cb(sizeof(piojo_tree_bnode_t*), qator);
        bnode = tree->root;
        piojo_list_append(&bnode, q);
        while (piojo_list_size(q) > 0){
                qnode = piojo_list_first(q);
                bnode = *(piojo_tree_bnode_t**) piojo_list_entry(qnode);
                piojo_list_delete(qnode, q);

                for (i = 0; i < bnode->ecnt; ++i){
                        finish_entry(tree, &bnode->entries[i]);
                }

                for (i = 0; ! bnode->leaf_p && i <= bnode->ecnt; ++i){
                        piojo_list_append(&bnode->children[i], q);
                }

                free_bnode(bnode, tree);
        }
        piojo_list_free(q);
}

/* Binary search for key. */
static piojo_tree_iter_t
search_node(const void *key, const piojo_tree_t *tree)
{
        bool found_p;
        size_t idx;
        piojo_tree_iter_t iter;
        piojo_tree_bnode_t *bnode = tree->root;

        iter.bnode = NULL;
        while (bnode->ecnt > 0){
                idx = bin_search(key, tree->cmp_cb, bnode, &found_p);
                if (found_p){
                        iter.bnode = bnode;
                        iter.eidx = idx;
                        break;
                }else if (bnode->leaf_p){
                        break;
                }
                bnode = bnode->children[idx];
        }
        return iter;
}

/* Similar to search_node() but split bnodes before traversing them. */
static piojo_tree_entry_t*
insert_node(piojo_tree_entry_t kv, insert_t op, piojo_tree_t *tree)
{
        int cmpval;
        bool found_p;
        size_t idx=0, j;
        piojo_tree_entry_t *e;
        piojo_tree_bnode_t *bnode;

        if (tree->root->ecnt == tree->cmax - 1){
                split_root(tree);
        }
        bnode = tree->root;
        while (bnode->ecnt > 0){
                idx = bin_search(kv.key, tree->cmp_cb, bnode, &found_p);
                if (found_p){
                        return &bnode->entries[idx];
                }else if (bnode->leaf_p){
                        break;
                }
                if (bnode->children[idx]->ecnt == tree->cmax - 1){
                        split_bnode(tree, idx, bnode->children[idx], bnode);
                        e = &bnode->entries[idx];
                        cmpval = tree->cmp_cb(kv.key, e->key);
                        if (cmpval == 0){
                                return e;
                        }else if (cmpval > 0){
                                ++idx;
                        }
                }
                bnode = bnode->children[idx];
        }
        switch (op){
        case INSERT_NEW:
                kv = init_entry(kv.key, kv.data, tree);
                break;
        case INSERT_COPY:
                kv = copy_entry(kv.key, kv.data, tree);
                break;
        default:
                PIOJO_ASSERT(FALSE);
                break;
        }

        PIOJO_ASSERT(bnode->ecnt < tree->cmax - 1);
        for (j = bnode->ecnt; j > idx; --j){
                bnode->entries[j] = bnode->entries[j - 1];
        }
        bnode->entries[idx] = kv;
        ++bnode->ecnt;

        return NULL;
}

static bool
delete_node(const void *key, piojo_tree_bnode_t *bnode, piojo_tree_t *tree)
{
        piojo_tree_entry_t kv;
        kv.key = NULL;
        return delete_node_2(key, kv, bnode, tree);
}

/*
 * Recursively delete key if it's found.
 * Pass the original key (first entry) to be deleted as parameter.
 */
static bool
delete_node_2(const void *key, piojo_tree_entry_t todelete,
              piojo_tree_bnode_t *bnode, piojo_tree_t *tree)
{
        bool found_p;
        size_t i;
        piojo_tree_bnode_t *next;
        piojo_tree_iter_t iter;

        i = bin_search(key, tree->cmp_cb, bnode, &found_p);
        if (found_p){
                if (todelete.key == NULL){
                        todelete = bnode->entries[i];
                }

                /* Key in leaf, shrink the leaf and finish. */
                if (bnode->leaf_p){
                        --bnode->ecnt;
                        for (; i < bnode->ecnt; ++i){
                                bnode->entries[i] = bnode->entries[i + 1];
                        }
                        finish_entry(tree, &todelete);
                        return TRUE;
                }

                /* Key in internal node, move prev/next key up and delete it. */
                iter.bnode = bnode;
                if (bnode->children[i]->ecnt >= tree->cmin){
                        iter.eidx = i;
                        bnode->entries[i] = *search_max(&iter);
                        return delete_node_2(bnode->entries[i].key, todelete,
                                             bnode->children[i], tree);
                }else if (bnode->children[i + 1]->ecnt >= tree->cmin){
                        iter.eidx = i + 1;
                        bnode->entries[i] = *search_min(&iter);
                        return delete_node_2(bnode->entries[i].key, todelete,
                                             bnode->children[i + 1], tree);
                }

                /* Both node children are key deficient, merge and try again. */
                PIOJO_ASSERT(bnode->children[i]->ecnt == tree->cmin - 1);
                PIOJO_ASSERT(bnode->children[i + 1]->ecnt == tree->cmin - 1);
                next = bnode->children[i];
                merge_bnodes(tree, i, next, bnode->children[i + 1], bnode);
                return delete_node_2(key, todelete, next, tree);
        }else if (! bnode->leaf_p){
                /* Key not in internal node, rebalance and try again. */
                next = bnode->children[i];
                if (next->ecnt < tree->cmin){
                        PIOJO_ASSERT(next->ecnt == tree->cmin - 1);
                        next = rebalance_bnode(tree, i, next, bnode);
                }
                return delete_node_2(key, todelete, next, tree);
        }
        /* Key not in leaf. */
        return FALSE;
}

/* Returns bnode if it wasn't freed by merge, left sibling otherwise. */
static piojo_tree_bnode_t*
rebalance_bnode(piojo_tree_t *tree, size_t pidx, piojo_tree_bnode_t *bnode,
                piojo_tree_bnode_t *parent)
{
        piojo_tree_bnode_t *lsibling=NULL, *rsibling=NULL;
        if (pidx > 0){
                lsibling = parent->children[pidx - 1];
        }
        if (pidx < parent->ecnt){
                rsibling = parent->children[pidx + 1];
        }
        if (lsibling != NULL && lsibling->ecnt >= tree->cmin){
                PIOJO_ASSERT(bnode->ecnt > 0 && bnode->ecnt < tree->cmax - 1);
                PIOJO_ASSERT(lsibling->ecnt > 1);
                rotate_right(pidx - 1, lsibling, bnode, parent);
        }else if (rsibling != NULL && rsibling->ecnt >= tree->cmin){
                PIOJO_ASSERT(bnode->ecnt > 0 && bnode->ecnt < tree->cmax - 1);
                PIOJO_ASSERT(rsibling->ecnt > 1);
                rotate_left(pidx, bnode, rsibling, parent);
        }else if (lsibling != NULL){
                merge_bnodes(tree, pidx - 1, lsibling, bnode, parent);
                return lsibling;
        }else{
                PIOJO_ASSERT(rsibling);
                merge_bnodes(tree, pidx, bnode, rsibling, parent);
        }
        return bnode;
}

static piojo_tree_entry_t*
search_min(piojo_tree_iter_t *from)
{
        piojo_tree_bnode_t *bnode = from->bnode;
        size_t idx = from->eidx;
        piojo_tree_entry_t *kv = NULL;

        while (! bnode->leaf_p){
                bnode = bnode->children[idx];
                idx = 0;
        }
        if (bnode->ecnt > 0){
                from->bnode = bnode;
                from->eidx = idx;
                kv = &bnode->entries[idx];
        }
        return kv;
}

static piojo_tree_entry_t*
search_max(piojo_tree_iter_t *from)
{
        piojo_tree_bnode_t *bnode = from->bnode;
        size_t idx = from->eidx;
        piojo_tree_entry_t *kv = NULL;

        while (! bnode->leaf_p){
                bnode = bnode->children[idx];
                idx = bnode->ecnt;
        }
        if (bnode->ecnt > 0){
                from->eidx = bnode->ecnt - 1;
                from->bnode = bnode;
                kv = &bnode->entries[from->eidx];
        }
        return kv;
}

static size_t
bin_search(const void *key, piojo_cmp_cb cmp_cb, piojo_tree_bnode_t *bnode,
           bool *found_p)
{
        int cmpval;
        size_t mid, imin = 0, imax = bnode->ecnt - 1;
        piojo_tree_entry_t *e;

        *found_p = FALSE;
        if (bnode->ecnt > 0){
                while (imin <= imax){
                        mid = imin + ((imax - imin) / 2);
                        e = &bnode->entries[mid];
                        cmpval = cmp_cb(key, e->key);
                        if (cmpval == 0){
                                *found_p = TRUE;
                                imin = mid;
                                break;
                        }else if (cmpval > 0){
                                imin = mid + 1;
                        }else if (imin != mid){
                                imax = mid - 1;
                        }else{
                                break;
                        }
                }
        }
        return imin;
}

#ifdef PIOJO_DEBUG
void
print_tree(piojo_tree_t*tree)
{
        piojo_list_t*q=piojo_list_alloc_s(sizeof(piojo_tree_bnode_t*));
        piojo_list_node_t *qnode;
        piojo_tree_bnode_t*bnode,*b2;
        piojo_tree_entry_t*node;
        piojo_tree_node_t iter;
        size_t idx;
        piojo_list_append(&tree->root, q);
        while (piojo_list_size(q) > 0){
                qnode = piojo_list_first(q);
                bnode = *(piojo_tree_bnode_t**)piojo_list_entry(qnode);
                piojo_list_delete(qnode, q);
                printf("\nNode %p (%u)(p:%p/%u leaf:%u) : ", (void*)bnode,
                       bnode->ecnt,
                       (void*)bnode->parent,bnode->pidx,bnode->leaf_p);
                if (bnode->leaf_p){
                        for (idx=0; idx<bnode->ecnt;++idx){
                                node = &bnode->entries[idx];
                                printf("%d=%d , ", *(int*)node->key,
                                       *(int*)node->data);
                        }
                }else{
                        for (idx=0; idx<bnode->ecnt;++idx){
                                b2 = bnode->children[idx];
                                printf("[ %p ] ", (void*)b2);
                                piojo_list_append(&b2,q);
                                node = &bnode->entries[idx];
                                printf("%d=%d ", *(int*)node->key,
                                       *(int*)node->data);
                        }
                        b2 = bnode->children[idx];
                        printf("[ %p ]", (void*)b2);
                        piojo_list_append(&b2,q);
                }
                if (bnode->ecnt == tree->cmax - 1){
                        printf(" @@@ FULL");
                }
        }
        if (tree->ecount > 0){
                piojo_tree_first(tree, &iter);
                printf("\nMIN: %d = %d ", *(int*)piojo_tree_entryk(&iter),
                       *(int*)piojo_tree_entryv(&iter));
                piojo_tree_last(tree, &iter);
                printf("MAX: %d = %d\n", *(int*)piojo_tree_entryk(&iter),
                       *(int*)piojo_tree_entryv(&iter));
        }
        piojo_list_free(q);
}
#endif /* PIOJO_DEBUG */

/*
 * Private compare functions.
 */

static int
u32_cmp(const void *e1, const void *e2)
{
        uint32_t v1 = *(uint32_t*) e1;
        uint32_t v2 = *(uint32_t*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
i32_cmp(const void *e1, const void *e2)
{
        int32_t v1 = *(int32_t*) e1;
        int32_t v2 = *(int32_t*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
u64_cmp(const void *e1, const void *e2)
{
        uint64_t v1 = *(uint64_t*) e1;
        uint64_t v2 = *(uint64_t*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
i64_cmp(const void *e1, const void *e2)
{
        int64_t v1 = *(int64_t*) e1;
        int64_t v2 = *(int64_t*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
int_cmp(const void *e1, const void *e2)
{
        int v1 = *(int*) e1;
        int v2 = *(int*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
uint_cmp(const void *e1, const void *e2)
{
        unsigned int v1 = *(unsigned int*) e1;
        unsigned int v2 = *(unsigned int*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
ptr_cmp(const void *e1, const void *e2)
{
        uintptr_t v1 = *(uintptr_t*) e1;
        uintptr_t v2 = *(uintptr_t*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}

static int
siz_cmp(const void *e1, const void *e2)
{
        size_t v1 = *(size_t*) e1;
        size_t v2 = *(size_t*) e2;
        if (v1 > v2){
                return 1;
        }else if (v1 < v2){
                return -1;
        }
        return 0;
}
