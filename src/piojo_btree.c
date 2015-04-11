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
 * @addtogroup piojobtree Piojo B-tree
 * @{
 * Piojo B-tree implementation.
 * Use @a maxchildren >= @b 8 for good performance.
 */

#include <piojo/piojo_btree.h>
#include <piojo_defs.h>

typedef struct {
        void *key, *value;
} kv_t;

typedef struct bnode_t bnode_t;
struct bnode_t {
        bool leaf_p;
        uint8_t ecnt, pidx;
        kv_t *kvs;
        bnode_t **children, *parent;
};

typedef struct {
        uint8_t eidx;
        bnode_t *bnode;
        const piojo_btree_t *tree;
} iter_t;

struct piojo_btree_t {
        bnode_t *root;
        size_t eksize, evsize, ecount, cmin, cmax;
        piojo_cmp_cb cmp_cb;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of tree in bytes */
const size_t piojo_btree_sizeof = sizeof(piojo_btree_t);

static const uint8_t TREE_CHILDREN_MAX = 8;

static bnode_t*
alloc_bnode(const piojo_btree_t *tree);

static void
free_bnode(const bnode_t *bnode, const piojo_btree_t *tree);

static void
split_bnode(piojo_btree_t *tree, size_t pidx, bnode_t *bnode,
            bnode_t *parent);

static void
split_root(piojo_btree_t *tree);

static void
merge_bnodes(piojo_btree_t *tree, size_t pidx, bnode_t *lbnode,
             bnode_t *rbnode, bnode_t *parent);

static void
rotate_left(size_t pidx, bnode_t *lbnode,
            bnode_t *rbnode, bnode_t *parent,
            const piojo_btree_t *tree);

static void
rotate_right(size_t pidx, bnode_t *lbnode,
             bnode_t *rbnode, bnode_t *parent,
             const piojo_btree_t *tree);

static bnode_t*
rebalance_bnode(piojo_btree_t *tree, size_t pidx, bnode_t *bnode,
                bnode_t *parent);

static void
move_child(bnode_t *child, uint8_t toidx, bnode_t *to);

static void
copy_bentry(uint8_t eidx, const bnode_t *bnode, uint8_t toidx,
            const bnode_t *to, const piojo_btree_t *tree);

static void*
entry_key(uint8_t eidx, const bnode_t *bnode,
          const piojo_btree_t *tree);

static void*
entry_val(uint8_t eidx, const bnode_t *bnode,
          const piojo_btree_t *tree);

static void
init_entry(const void *key, const void *data, uint8_t eidx,
           const bnode_t *bnode, const piojo_btree_t *tree);

static void
copy_entry(const void *key, const void *data, uint8_t eidx,
           const bnode_t *bnode, const piojo_btree_t *tree);

static void
search_min(iter_t *from);

static void
search_max(iter_t *from);

static iter_t
insert_node(const void *key, const void *data, insert_t op, piojo_btree_t *tree);

static iter_t
search_node(const void *key, const piojo_btree_t *tree);

static bool
delete_node(const void *key, bool deleted_p,
            bnode_t *bnode, piojo_btree_t *tree);

static size_t
bin_search(const void *key, const piojo_btree_t *tree,
           bnode_t *bnode, bool *found_p);

static int
i32_cmp(const void *e1, const void *e2);

static int
i64_cmp(const void *e1, const void *e2);

static int
siz_cmp(const void *e1, const void *e2);

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_btree_t*
piojo_btree_alloc_i32k(size_t evsize)
{
        return piojo_btree_alloc_cb_i32k(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_btree_t*
piojo_btree_alloc_i64k(size_t evsize)
{
        return piojo_btree_alloc_cb_i64k(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_default);
}

/**
 * Allocates a new tree.
 * Uses default allocator and key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New tree.
 */
piojo_btree_t*
piojo_btree_alloc_sizk(size_t evsize)
{
        return piojo_btree_alloc_cb_sizk(TREE_CHILDREN_MAX, evsize,
                                        piojo_alloc_default);
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
piojo_btree_t*
piojo_btree_alloc_cb_i32k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_btree_alloc_cb_cmp(maxchildren, evsize,
                                       i32_cmp, sizeof(int32_t), allocator);
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
piojo_btree_t*
piojo_btree_alloc_cb_i64k(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_btree_alloc_cb_cmp(maxchildren, evsize,
                                       i64_cmp, sizeof(int64_t), allocator);
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
piojo_btree_t*
piojo_btree_alloc_cb_sizk(uint8_t maxchildren, size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_btree_alloc_cb_cmp(maxchildren, evsize,
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
piojo_btree_t*
piojo_btree_alloc_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize)
{
        return piojo_btree_alloc_cb_cmp(TREE_CHILDREN_MAX, evsize,
                                       keycmp, eksize, piojo_alloc_default);
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
piojo_btree_t*
piojo_btree_alloc_cb_cmp(uint8_t maxchildren, size_t evsize,
                        piojo_cmp_cb keycmp, size_t eksize,
                        piojo_alloc_if allocator)
{
        piojo_btree_t * tree;
        PIOJO_ASSERT(evsize > 0 && eksize > 0);
        PIOJO_ASSERT(maxchildren > 2 && maxchildren < 255 &&
                     maxchildren % 2 == 0);

        tree = (piojo_btree_t *) allocator.alloc_cb(sizeof(piojo_btree_t));
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
piojo_btree_t*
piojo_btree_copy(const piojo_btree_t *tree)
{
        piojo_btree_t *newtree;
        const void *key;
        void *data;
        PIOJO_ASSERT(tree);

        newtree = piojo_btree_alloc_cb_cmp(tree->cmax, tree->evsize,
                                          tree->cmp_cb, tree->eksize,
                                          tree->allocator);
        newtree->ecount = tree->ecount;

        key = piojo_btree_first(tree, &data);
        while (key != NULL){
                insert_node(key, data, INSERT_COPY, newtree);
                key = piojo_btree_next(key, tree, &data);
        }
        return newtree;
}

/**
 * Frees @a tree and all its entries.
 * @param[in] tree Tree being freed.
 */
void
piojo_btree_free(const piojo_btree_t *tree)
{
        piojo_btree_t *t;
        PIOJO_ASSERT(tree);

        t = (piojo_btree_t*) tree;
        piojo_btree_clear(t);
        free_bnode(t->root, t);
        tree->allocator.free_cb(t);
}


/**
 * Deletes all entries in @a tree.
 * @param[out] tree Tree being cleared.
 */
void
piojo_btree_clear(piojo_btree_t *tree)
{
        void *key;
        PIOJO_ASSERT(tree);

        key = tree->allocator.alloc_cb(tree->eksize);
        PIOJO_ASSERT(key);
        while (tree->ecount > 0){
                memcpy(key, entry_key(0, tree->root, tree), tree->eksize);
                delete_node(key, FALSE, tree->root, tree);
                --tree->ecount;
        }
        tree->allocator.free_cb(key);
}

/**
 * Returns number of entries.
 * @param[in] tree
 * @return Number of entries in @a tree.
 */
size_t
piojo_btree_size(const piojo_btree_t *tree)
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
piojo_btree_insert(const void *key, const void *data, piojo_btree_t *tree)
{
        iter_t iter;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(tree->ecount < SIZE_MAX);
        PIOJO_ASSERT(data || tree->evsize == sizeof(bool));

        iter = insert_node(key, data, INSERT_NEW, tree);
        if (iter.bnode == NULL){
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
piojo_btree_set(const void *key, const void *data, piojo_btree_t *tree)
{
        iter_t iter;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(data || tree->evsize == sizeof(bool));

        iter = insert_node(key, data, INSERT_NEW, tree);
        if (iter.bnode == NULL){
                PIOJO_ASSERT(tree->ecount < SIZE_MAX);
                ++tree->ecount;
                return TRUE;
        }

        if (data != NULL){
                memcpy(entry_val(iter.eidx, iter.bnode, tree), data,
                       tree->evsize);
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
piojo_btree_search(const void *key, const piojo_btree_t *tree)
{
        iter_t iter;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        iter = search_node(key, tree);
        if (iter.bnode != NULL){
                return entry_val(iter.eidx, iter.bnode, tree);
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
piojo_btree_delete(const void *key, piojo_btree_t *tree)
{
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        if (delete_node(key, FALSE, tree->root, tree)){
                --tree->ecount;
                return TRUE;
        }
        return FALSE;
}

/**
 * Reads the first key in @a tree (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] data Entry value, can be @b NULL.
 * @return first key or @b NULL if @a tree is empty.
 */
const void*
piojo_btree_first(const piojo_btree_t *tree, void **data)
{
        iter_t iter;
        PIOJO_ASSERT(tree);

        if (tree->ecount > 0){
                iter.tree = tree;
                iter.eidx = 0;
                iter.bnode = tree->root;
                search_min(&iter);
                if (data != NULL){
                        *data = entry_val(iter.eidx, iter.bnode, tree);
                }
                return entry_key(iter.eidx, iter.bnode, tree);
        }
        return NULL;
}

/**
 * Reads the last key in @a tree (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] data Entry value, can be @b NULL.
 * @return last key or @b NULL if @a tree is empty.
 */
const void*
piojo_btree_last(const piojo_btree_t *tree, void **data)
{
        iter_t iter;
        PIOJO_ASSERT(tree);

        if (tree->ecount > 0){
                iter.tree = tree;
                iter.eidx = tree->root->ecnt;
                iter.bnode = tree->root;
                search_max(&iter);
                if (data != NULL){
                        *data = entry_val(iter.eidx, iter.bnode, tree);
                }
                return entry_key(iter.eidx, iter.bnode, tree);
        }
        return NULL;
}


/**
 * Reads the next key (order given by @a keycmp function).
 * @param[in] key
 * @param[in] tree
 * @param[out] data Entry value, can be @b NULL.
 * @return next key or @b NULL if @a key is the last one.
 */
const void*
piojo_btree_next(const void *key, const piojo_btree_t *tree, void **data)
{
        iter_t iter;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        iter = search_node(key, tree);
        PIOJO_ASSERT(iter.bnode != NULL);

        if (! iter.bnode->leaf_p && iter.eidx < iter.bnode->ecnt){
                iter.bnode = iter.bnode->children[iter.eidx + 1];
                iter.eidx = 0;
                search_min(&iter);
        }else if (iter.eidx + 1 < iter.bnode->ecnt){
                ++iter.eidx;
        }else{
                while (iter.bnode->parent != NULL){
                        iter.eidx = iter.bnode->pidx;
                        iter.bnode = iter.bnode->parent;
                        if (iter.eidx < iter.bnode->ecnt){
                                if (data != NULL){
                                        *data = entry_val(iter.eidx, iter.bnode, tree);
                                }
                                return entry_key(iter.eidx, iter.bnode, tree);
                        }
                }
                return NULL;
        }
        if (data != NULL){
                *data = entry_val(iter.eidx, iter.bnode, tree);
        }
        return entry_key(iter.eidx, iter.bnode, tree);
}

/**
 * Reads the previous key (order given by @a keycmp function).
 * @param[in] key
 * @param[in] tree
 * @param[out] data Entry value, can be @b NULL.
 * @return previous key or @b NULL if @a key is the first one.
 */
const void*
piojo_btree_prev(const void *key, const piojo_btree_t *tree, void **data)
{
        iter_t iter;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        iter = search_node(key, tree);
        PIOJO_ASSERT(iter.bnode != NULL);

        if (! iter.bnode->leaf_p && iter.eidx < iter.bnode->ecnt + 1){
                iter.bnode = iter.bnode->children[iter.eidx];
                iter.eidx = iter.bnode->ecnt;
                search_max(&iter);
        }else if (iter.eidx > 0){
                --iter.eidx;
        }else{
                while (iter.bnode->parent != NULL){
                        iter.eidx = iter.bnode->pidx;
                        iter.bnode = iter.bnode->parent;
                        if (iter.eidx > 0){
                                --iter.eidx;
                                if (data != NULL){
                                        *data = entry_val(iter.eidx, iter.bnode, tree);
                                }
                                return entry_key(iter.eidx, iter.bnode, tree);
                        }
                }
                return NULL;
        }
        if (data != NULL){
                *data = entry_val(iter.eidx, iter.bnode, tree);
        }
        return entry_key(iter.eidx, iter.bnode, tree);
}

/** @}
 * Private functions.
 */

static bnode_t*
alloc_bnode(const piojo_btree_t *tree)
{
        piojo_alloc_if ator = tree->allocator;
        bnode_t *bnode;
        size_t kvsize = sizeof(kv_t) * (tree->cmax - 1);
        size_t childrensiz = sizeof(bnode_t*) * tree->cmax;
        size_t bnodesiz = sizeof(bnode_t);

        bnode = ((bnode_t*)
                 ator.alloc_cb(bnodesiz + kvsize + childrensiz));
        PIOJO_ASSERT(bnode);

        bnode->kvs = (kv_t*)((uint8_t*) bnode + bnodesiz);
        bnode->children = ((bnode_t**)
                           ((uint8_t*) bnode->kvs + kvsize));
        bnode->ecnt = bnode->pidx = 0;
        bnode->leaf_p = FALSE;
        bnode->parent = NULL;

        return bnode;
}

static void
free_bnode(const bnode_t *bnode, const piojo_btree_t *tree)
{
        tree->allocator.free_cb(bnode);
}

static void
move_child(bnode_t *child, uint8_t toidx, bnode_t *to)
{
        to->children[toidx] = child;
        child->parent = to;
        child->pidx = toidx;
}

static void
split_root(piojo_btree_t *tree)
{
        bnode_t *newroot;
        PIOJO_ASSERT(tree->root->ecnt == tree->cmax - 1);

        newroot = alloc_bnode(tree);
        newroot->leaf_p = FALSE;
        move_child(tree->root, 0, newroot);

        split_bnode(tree, 0, tree->root, newroot);

        tree->root = newroot;
}

static void
split_bnode(piojo_btree_t *tree, size_t pidx, bnode_t *bnode,
            bnode_t *parent)
{
        size_t i, j, mid = bnode->ecnt / 2;
        bnode_t *rbnode;
        PIOJO_ASSERT(parent->ecnt < tree->cmax - 1);

        /* Copy children/entries greater than median to right bnode. */
        rbnode = alloc_bnode(tree);
        for (i = mid + 1; i < bnode->ecnt; ++i, ++rbnode->ecnt){
                copy_bentry(i, bnode, rbnode->ecnt, rbnode, tree);
        }
        if (! bnode->leaf_p){
                for (j = 0, i = mid + 1; i <= bnode->ecnt; ++i, ++j){
                        move_child(bnode->children[i], j, rbnode);
                }
        }
        rbnode->leaf_p = bnode->leaf_p;

        /* Expand parent bnode. */
        for (i = parent->ecnt; i > pidx; --i){
                copy_bentry(i - 1, parent, i, parent, tree);
        }
        for (i = parent->ecnt + 1; i > pidx + 1; --i){
                move_child(parent->children[i - 1], i, parent);
        }

        /* Move median to parent bnode and update children. */
        copy_bentry(mid, bnode, pidx, parent, tree);
        PIOJO_ASSERT(parent->children[pidx] == bnode);
        move_child(rbnode, pidx + 1, parent);
        ++parent->ecnt;

        bnode->ecnt = mid;
}

static void
merge_bnodes(piojo_btree_t *tree, size_t pidx, bnode_t *lbnode,
             bnode_t *rbnode, bnode_t *parent)
{
        size_t i, ccnt = lbnode->ecnt + 1;

        /* Append parent entry to left bnode. */
        copy_bentry(pidx, parent, lbnode->ecnt, lbnode, tree);
        ++lbnode->ecnt;

        /* Append right bnode to left bnode. */
        for (i = 0; i < rbnode->ecnt; ++i, ++lbnode->ecnt){
                copy_bentry(i, rbnode, lbnode->ecnt, lbnode, tree);
        }
        if (! rbnode->leaf_p){
                for (i = 0; i <= rbnode->ecnt; ++i, ++ccnt){
                        move_child(rbnode->children[i], ccnt, lbnode);
                }
        }

        /* Shrink parent bnode. */
        --parent->ecnt;
        for (i = pidx; i < parent->ecnt; ++i){
                copy_bentry(i + 1, parent, i, parent, tree);
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
rotate_left(size_t pidx, bnode_t *lbnode,
            bnode_t *rbnode, bnode_t *parent,
            const piojo_btree_t *tree)
{
        size_t i;

        /* Append parent entry to left bnode. */
        copy_bentry(pidx, parent, lbnode->ecnt, lbnode, tree);

        /* Append first child from right bnode to left bnode. */
        --rbnode->ecnt;
        if (! lbnode->leaf_p){
                move_child(rbnode->children[0], lbnode->ecnt + 1, lbnode);
                for (i = 0; i <= rbnode->ecnt; ++i){
                        move_child(rbnode->children[i + 1], i, rbnode);
                }
        }

        /* Move first entry from right bnode to parent bnode. */
        copy_bentry(0, rbnode, pidx, parent, tree);
        for (i = 0; i < rbnode->ecnt; ++i){
                copy_bentry(i + 1, rbnode, i, rbnode, tree);
        }

        ++lbnode->ecnt;
}

static void
rotate_right(size_t pidx, bnode_t *lbnode,
             bnode_t *rbnode, bnode_t *parent,
             const piojo_btree_t *tree)
{
        size_t i;

        /* Prepend parent entry to right bnode. */
        for (i = rbnode->ecnt; i > 0; --i){
                copy_bentry(i - 1, rbnode, i, rbnode, tree);
        }
        copy_bentry(pidx, parent, 0, rbnode, tree);
        ++rbnode->ecnt;

        /* Prepend last child from left bnode to right bnode. */
        if (! rbnode->leaf_p){
                for (i = rbnode->ecnt; i > 0; --i){
                        move_child(rbnode->children[i - 1], i, rbnode);
                }
                move_child(lbnode->children[lbnode->ecnt], 0, rbnode);
        }

        /* Move last entry from left bnode to parent bnode. */
        copy_bentry(lbnode->ecnt - 1, lbnode, pidx, parent, tree);

        --lbnode->ecnt;
}

static void
copy_bentry(uint8_t eidx, const bnode_t *bnode, uint8_t toidx,
            const bnode_t *to, const piojo_btree_t *tree)
{
        to->kvs[toidx].key = bnode->kvs[eidx].key;
        to->kvs[toidx].value = bnode->kvs[eidx].value;
}

static void*
entry_key(uint8_t eidx, const bnode_t *bnode,
          const piojo_btree_t *tree)
{
        return bnode->kvs[eidx].key;
}

static void*
entry_val(uint8_t eidx, const bnode_t *bnode,
          const piojo_btree_t *tree)
{
        return bnode->kvs[eidx].value;
}

static void
init_entry(const void *key, const void *data, uint8_t eidx,
           const bnode_t *bnode, const piojo_btree_t *tree)
{
        bool null_p = TRUE;
        piojo_alloc_if ator = tree->allocator;

        if (data == NULL){
                data = &null_p;
        }

        bnode->kvs[eidx].key = ator.alloc_cb(tree->eksize);
        PIOJO_ASSERT(bnode->kvs[eidx].key);
        memcpy(entry_key(eidx, bnode, tree), key, tree->eksize);

        bnode->kvs[eidx].value = ator.alloc_cb(tree->evsize);
        PIOJO_ASSERT(bnode->kvs[eidx].value);
        memcpy(entry_val(eidx, bnode, tree), data, tree->evsize);
}

static void
copy_entry(const void *key, const void *data, uint8_t eidx,
           const bnode_t *bnode, const piojo_btree_t *tree)
{
        piojo_alloc_if ator = tree->allocator;

        bnode->kvs[eidx].key = ator.alloc_cb(tree->eksize);
        PIOJO_ASSERT(bnode->kvs[eidx].key);
        memcpy(entry_key(eidx, bnode, tree), key, tree->eksize);

        bnode->kvs[eidx].value = ator.alloc_cb(tree->evsize);
        PIOJO_ASSERT(bnode->kvs[eidx].value);
        memcpy(entry_val(eidx, bnode, tree), data, tree->evsize);
}

/* Binary search for key. */
static iter_t
search_node(const void *key, const piojo_btree_t *tree)
{
        bool found_p;
        size_t idx;
        iter_t iter;
        bnode_t *bnode = tree->root;

        iter.bnode = NULL;
        while (bnode->ecnt > 0){
                idx = bin_search(key, tree, bnode, &found_p);
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
static iter_t
insert_node(const void *key, const void *data, insert_t op, piojo_btree_t *tree)
{
        int cmpval;
        bool found_p;
        size_t idx=0, j;
        iter_t iter;
        bnode_t *bnode;

        if (tree->root->ecnt == tree->cmax - 1){
                split_root(tree);
        }
        bnode = tree->root;
        iter.bnode = NULL;
        while (bnode->ecnt > 0){
                idx = bin_search(key, tree, bnode, &found_p);
                if (found_p){
                        iter.bnode = bnode;
                        iter.eidx = idx;
                        return iter;
                }else if (bnode->leaf_p){
                        break;
                }
                if (bnode->children[idx]->ecnt == tree->cmax - 1){
                        split_bnode(tree, idx, bnode->children[idx], bnode);
                        cmpval = tree->cmp_cb(key, entry_key(idx, bnode, tree));
                        if (cmpval == 0){
                                iter.bnode = bnode;
                                iter.eidx = idx;
                                return iter;
                        }else if (cmpval > 0){
                                ++idx;
                        }
                }
                bnode = bnode->children[idx];
        }
        PIOJO_ASSERT(bnode->ecnt < tree->cmax - 1);
        for (j = bnode->ecnt; j > idx; --j){
                copy_bentry(j - 1, bnode, j, bnode, tree);
        }
        switch (op){
        case INSERT_NEW:
                init_entry(key, data, idx, bnode, tree);
                break;
        case INSERT_COPY:
                copy_entry(key, data, idx, bnode, tree);
                break;
        default:
                PIOJO_ASSERT(FALSE);
                break;
        }
        ++bnode->ecnt;

        return iter;
}


/*
 * Recursively delete key if it's found.
 * 'deleted_p' is TRUE if the original key was found/deleted.
 */
static bool
delete_node(const void *key, bool deleted_p,
              bnode_t *bnode, piojo_btree_t *tree)
{
        bool found_p;
        size_t i;
        bnode_t *next;
        iter_t iter;

        i = bin_search(key, tree, bnode, &found_p);
        if (found_p){
                /* Key in leaf, shrink the leaf and finish. */
                if (bnode->leaf_p){
                        if (! deleted_p){
                                tree->allocator.free_cb(bnode->kvs[i].key);
                                tree->allocator.free_cb(bnode->kvs[i].value);
                                deleted_p = TRUE;
                        }
                        --bnode->ecnt;
                        for (; i < bnode->ecnt; ++i){
                                copy_bentry(i + 1, bnode, i, bnode, tree);
                        }
                        return deleted_p;
                }

                /* Key in internal node, move prev/next key up and delete it. */
                iter.bnode = bnode;
                if (bnode->children[i]->ecnt >= tree->cmin){
                        if (! deleted_p){
                                tree->allocator.free_cb(bnode->kvs[i].key);
                                tree->allocator.free_cb(bnode->kvs[i].value);
                                deleted_p = TRUE;
                        }
                        iter.eidx = i;
                        search_max(&iter);
                        copy_bentry(iter.eidx, iter.bnode, i, bnode, tree);
                        return delete_node(entry_key(i, bnode, tree), deleted_p,
                                           bnode->children[i], tree);
                }else if (bnode->children[i + 1]->ecnt >= tree->cmin){
                        if (! deleted_p){
                                tree->allocator.free_cb(bnode->kvs[i].key);
                                tree->allocator.free_cb(bnode->kvs[i].value);
                                deleted_p = TRUE;
                        }
                        iter.eidx = i + 1;
                        search_min(&iter);
                        copy_bentry(iter.eidx, iter.bnode, i, bnode, tree);
                        return delete_node(entry_key(i, bnode, tree), deleted_p,
                                           bnode->children[i + 1], tree);
                }

                /* Both node children are key deficient, merge and try again. */
                PIOJO_ASSERT(bnode->children[i]->ecnt == tree->cmin - 1);
                PIOJO_ASSERT(bnode->children[i + 1]->ecnt == tree->cmin - 1);
                next = bnode->children[i];
                merge_bnodes(tree, i, next, bnode->children[i + 1], bnode);
                return delete_node(key, deleted_p, next, tree);
        }else if (! bnode->leaf_p){
                /* Key not in internal node, rebalance and try again. */
                next = bnode->children[i];
                if (next->ecnt < tree->cmin){
                        PIOJO_ASSERT(next->ecnt == tree->cmin - 1);
                        next = rebalance_bnode(tree, i, next, bnode);
                }
                return delete_node(key, deleted_p, next, tree);
        }
        /* Key not in leaf. */
        return deleted_p;
}

/* Returns bnode if it wasn't freed by merge, left sibling otherwise. */
static bnode_t*
rebalance_bnode(piojo_btree_t *tree, size_t pidx, bnode_t *bnode,
                bnode_t *parent)
{
        bnode_t *lsibling=NULL, *rsibling=NULL;
        if (pidx > 0){
                lsibling = parent->children[pidx - 1];
        }
        if (pidx < parent->ecnt){
                rsibling = parent->children[pidx + 1];
        }
        if (lsibling != NULL && lsibling->ecnt >= tree->cmin){
                PIOJO_ASSERT(bnode->ecnt > 0 && bnode->ecnt < tree->cmax - 1);
                PIOJO_ASSERT(lsibling->ecnt > 1);
                rotate_right(pidx - 1, lsibling, bnode, parent, tree);
        }else if (rsibling != NULL && rsibling->ecnt >= tree->cmin){
                PIOJO_ASSERT(bnode->ecnt > 0 && bnode->ecnt < tree->cmax - 1);
                PIOJO_ASSERT(rsibling->ecnt > 1);
                rotate_left(pidx, bnode, rsibling, parent, tree);
        }else if (lsibling != NULL){
                merge_bnodes(tree, pidx - 1, lsibling, bnode, parent);
                return lsibling;
        }else{
                PIOJO_ASSERT(rsibling);
                merge_bnodes(tree, pidx, bnode, rsibling, parent);
        }
        return bnode;
}

static void
search_min(iter_t *from)
{
        bnode_t *bnode = from->bnode;
        size_t idx = from->eidx;

        while (! bnode->leaf_p){
                bnode = bnode->children[idx];
                idx = 0;
        }
        if (bnode->ecnt > 0){
                from->bnode = bnode;
                from->eidx = idx;
        }
}

static void
search_max(iter_t *from)
{
        bnode_t *bnode = from->bnode;
        size_t idx = from->eidx;

        while (! bnode->leaf_p){
                bnode = bnode->children[idx];
                idx = bnode->ecnt;
        }
        if (bnode->ecnt > 0){
                from->eidx = bnode->ecnt - 1;
                from->bnode = bnode;
        }
}

static size_t
bin_search(const void *key, const piojo_btree_t *tree,
           bnode_t *bnode, bool *found_p)
{
        int cmpval;
        size_t mid, imin = 0, imax = bnode->ecnt - 1;

        *found_p = FALSE;
        if (bnode->ecnt > 0){
                while (imin <= imax){
                        mid = imin + ((imax - imin) / 2);
                        cmpval = tree->cmp_cb(key, entry_key(mid, bnode, tree));
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

/*
 * Private compare functions.
 */

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
