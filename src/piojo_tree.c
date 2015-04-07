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
 * @addtogroup piojotree Piojo Red-Black Tree
 * @{
 * Piojo Red-Black Tree implementation.
 */

#include <piojo/piojo_tree.h>
#include <piojo_defs.h>

typedef enum {
        COLOR_BLACK,
        COLOR_RED
} color_t;

typedef struct rbnode_t rbnode_t;
struct rbnode_t {
        void *key, *value;
        rbnode_t *parent, *right, *left;
        color_t color;
};

struct piojo_tree_t {
        rbnode_t *root, *nil;
        size_t eksize, evsize, ecount;
        piojo_cmp_cb cmp_cb;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of tree in bytes */
const size_t piojo_tree_sizeof = sizeof(piojo_tree_t);

static rbnode_t*
alloc_rbnode(const piojo_tree_t *tree);

static void
free_rbnode(const rbnode_t *node, const piojo_tree_t *tree);

static void
init_rbnode(const void *key, const void *data, const rbnode_t *node,
            const piojo_tree_t *tree);

static void
copy_rbnode(const void *key, const void *data, const rbnode_t *node,
            const piojo_tree_t *tree);

static void
rotate_left(rbnode_t *node, piojo_tree_t *tree);

static void
rotate_right(rbnode_t *node, piojo_tree_t *tree);

static rbnode_t*
insert_node(const void *key, const void *data, insert_t op, piojo_tree_t *tree);

static void
fix_insert(rbnode_t *node, piojo_tree_t *tree);

static rbnode_t*
search_node(const void *key, const piojo_tree_t *tree);

static rbnode_t*
search_max(rbnode_t *node, const piojo_tree_t *tree);

static rbnode_t*
search_min(rbnode_t *node, const piojo_tree_t *tree);

static rbnode_t*
next_node(rbnode_t *node, const piojo_tree_t *tree);

static rbnode_t*
prev_node(rbnode_t *node, const piojo_tree_t *tree);

static bool
delete_node(const void *key, piojo_tree_t *tree);

static void
fix_delete(rbnode_t *node, piojo_tree_t *tree);

static int
i32_cmp(const void *e1, const void *e2);

static int
i64_cmp(const void *e1, const void *e2);

static int
int_cmp(const void *e1, const void *e2);

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
        return piojo_tree_alloc_cb_intk(evsize,
                                        piojo_alloc_default);
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
        return piojo_tree_alloc_cb_i32k(evsize,
                                        piojo_alloc_default);
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
        return piojo_tree_alloc_cb_i64k(evsize,
                                        piojo_alloc_default);
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
        return piojo_tree_alloc_cb_ptrk(evsize,
                                        piojo_alloc_default);
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
        return piojo_tree_alloc_cb_sizk(evsize,
                                        piojo_alloc_default);
}

/**
 * Allocates a new tree.
 * Uses key size of @b int.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_intk(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_tree_alloc_cb_cmp(evsize,
                                       int_cmp, sizeof(int), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_i32k(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_tree_alloc_cb_cmp(evsize,
                                       i32_cmp, sizeof(int32_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_i64k(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_tree_alloc_cb_cmp(evsize,
                                       i64_cmp, sizeof(int64_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b uintptr_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_ptrk(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_tree_alloc_cb_cmp(evsize,
                                       ptr_cmp, sizeof(uintptr_t), allocator);
}

/**
 * Allocates a new tree.
 * Uses key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_sizk(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_tree_alloc_cb_cmp(evsize,
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
        return piojo_tree_alloc_cb_cmp(evsize,
                                       keycmp, eksize, piojo_alloc_default);
}

/**
 * Allocates a new tree.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] keycmp Entry key comparison function.
 * @param[in] eksize Entry key size.
 * @param[in] allocator Allocator to be used.
 * @return New tree.
 */
piojo_tree_t*
piojo_tree_alloc_cb_cmp(size_t evsize,
                        piojo_cmp_cb keycmp, size_t eksize,
                        piojo_alloc_if allocator)
{
        piojo_tree_t * tree;
        PIOJO_ASSERT(evsize > 0 && eksize > 0);

        tree = (piojo_tree_t *) allocator.alloc_cb(sizeof(piojo_tree_t));
        PIOJO_ASSERT(tree);

        tree->allocator = allocator;
        tree->eksize = eksize;
        tree->evsize = evsize;
        tree->ecount = 0;
        tree->cmp_cb = keycmp;
        tree->nil = NULL;
        tree->nil = alloc_rbnode(tree);
        tree->root = tree->nil;

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
        void *key;
        bool end_p;
        PIOJO_ASSERT(tree);

        newtree = piojo_tree_alloc_cb_cmp(tree->evsize,
                                          tree->cmp_cb, tree->eksize,
                                          tree->allocator);
        newtree->ecount = tree->ecount;

        key = tree->allocator.alloc_cb(tree->eksize);
        PIOJO_ASSERT(key);

        end_p = piojo_tree_first(tree, key);
        while (! end_p){
                insert_node(key, piojo_tree_search(key, tree),
                            INSERT_COPY, newtree);
                end_p = piojo_tree_next(tree, key);
        }
        tree->allocator.free_cb(key);

        return newtree;
}

/**
 * Frees @a tree and all its entries.
 * @param[in] tree Tree being freed.
 */
void
piojo_tree_free(const piojo_tree_t *tree)
{
        piojo_tree_t * t;
        PIOJO_ASSERT(tree);

        t = (piojo_tree_t*) tree;
        piojo_tree_clear(t);
        free_rbnode(t->nil, t);
        t->allocator.free_cb(t);
}

/**
 * Deletes all entries in @a tree.
 * @param[out] tree Tree being cleared.
 */
void
piojo_tree_clear(piojo_tree_t *tree)
{
        PIOJO_ASSERT(tree);

        while (tree->ecount > 0){
                delete_node(tree->root->key, tree);
                --tree->ecount;
        }
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
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(tree->ecount < SIZE_MAX);
        PIOJO_ASSERT(data || tree->evsize == sizeof(bool));

        if (insert_node(key, data, INSERT_NEW, tree) == tree->nil){
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
        rbnode_t *node;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(data || tree->evsize == sizeof(bool));

        node = insert_node(key, data, INSERT_NEW, tree);
        if (node == tree->nil){
                PIOJO_ASSERT(tree->ecount < SIZE_MAX);
                ++tree->ecount;
                return TRUE;
        }

        if (data != NULL){
                memcpy(node->value, data, tree->evsize);
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
        rbnode_t *node;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        node = search_node(key, tree);
        if (node != tree->nil){
                return node->value;
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

        if (delete_node(key, tree)){
                --tree->ecount;
                return TRUE;
        }
        return FALSE;
}

/**
 * Reads the first key in @a tree (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] key
 * @return @b FALSE or @b TRUE if @a tree is empty.
 */
bool
piojo_tree_first(const piojo_tree_t *tree, void *key)
{
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        if (tree->ecount > 0){
                memcpy(key, search_min(tree->root, tree)->key, tree->eksize);
                return FALSE;
        }
        return TRUE;
}

/**
 * Reads the last key in @a tree (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] key
 * @return @b FALSE or @b TRUE if @a tree is empty.
 */
bool
piojo_tree_last(const piojo_tree_t *tree, void *key)
{
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        if (tree->ecount > 0){
                memcpy(key, search_max(tree->root, tree)->key, tree->eksize);
                return FALSE;
        }
        return TRUE;
}

/**
 * Reads the next key (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] key
 * @return @b FALSE or @b TRUE if @a key is the last one.
 */
bool
piojo_tree_next(const piojo_tree_t *tree, void *key)
{
        rbnode_t *rbnode;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        rbnode = search_node(key, tree);
        PIOJO_ASSERT(rbnode && rbnode != tree->nil);

        rbnode = next_node(rbnode, tree);
        if (rbnode != tree->nil){
                memcpy(key, rbnode->key, tree->eksize);
                return FALSE;
        }
        return TRUE;
}

/**
 * Reads the previous key (order given by @a keycmp function).
 * @param[in] tree
 * @param[out] key
 * @return @b FALSE or @b TRUE if @a key is the first one.
 */
bool
piojo_tree_prev(const piojo_tree_t *tree, void *key)
{
        rbnode_t *rbnode;
        PIOJO_ASSERT(tree);
        PIOJO_ASSERT(key);

        rbnode = search_node(key, tree);
        PIOJO_ASSERT(rbnode && rbnode != tree->nil);

        rbnode = prev_node(rbnode, tree);
        if (rbnode != tree->nil){
                memcpy(key, rbnode->key, tree->eksize);
                return FALSE;
        }
        return TRUE;
}

/** @}
 * Private functions.
 */

static rbnode_t*
alloc_rbnode(const piojo_tree_t *tree)
{
        piojo_alloc_if ator = tree->allocator;
        rbnode_t *node;
        size_t nodesiz = sizeof(rbnode_t);

        node = ((rbnode_t*)
                 ator.alloc_cb(nodesiz + tree->eksize + tree->evsize));
        PIOJO_ASSERT(node);

        node->key = (uint8_t*) node + nodesiz;
        node->value = (uint8_t*) node->key + tree->eksize;
        node->left = tree->nil;
        node->right = tree->nil;
        node->parent = tree->nil;
        node->color = COLOR_BLACK;

        return node;
}

static void
free_rbnode(const rbnode_t *node, const piojo_tree_t *tree)
{
        tree->allocator.free_cb(node);
}

static void
init_rbnode(const void *key, const void *data, const rbnode_t *node,
            const piojo_tree_t *tree)
{
        bool null_p = TRUE;
        piojo_alloc_if ator = tree->allocator;

        if (data == NULL){
                data = &null_p;
        }

        memcpy(node->key, key, tree->eksize);
        memcpy(node->value, data, tree->evsize);
}

static void
copy_rbnode(const void *key, const void *data, const rbnode_t *node,
            const piojo_tree_t *tree)
{
        piojo_alloc_if ator = tree->allocator;

        memcpy(node->key, key, tree->eksize);
        memcpy(node->value, data, tree->evsize);
}

static void
rotate_left(rbnode_t *node, piojo_tree_t *tree)
{
        rbnode_t *y;
        PIOJO_ASSERT(node->right != tree->nil);

        y = node->right;
        node->right = y->left;
        if (y->left != tree->nil){
                y->left->parent = node;
        }

        y->parent = node->parent;
        if (node->parent == tree->nil){
                tree->root = y;
        }else if (node->parent->left == node){
                node->parent->left = y;
        }else{
                node->parent->right = y;
        }

        y->left = node;
        node->parent = y;
}

static void
rotate_right(rbnode_t *node, piojo_tree_t *tree)
{
        rbnode_t *x;
        PIOJO_ASSERT(node->left != tree->nil);

        x = node->left;
        node->left = x->right;
        if (x->right != tree->nil){
                x->right->parent = node;
        }

        x->parent = node->parent;
        if (node->parent == tree->nil){
                tree->root = x;
        }else if (node->parent->left == node){
                node->parent->left = x;
        }else{
                node->parent->right = x;
        }

        x->right = node;
        node->parent = x;
}

static rbnode_t*
insert_node(const void *key, const void *data, insert_t op, piojo_tree_t *tree)
{
        int cmpval;
        rbnode_t *newnode, *parent = tree->nil, *cur = tree->root;
        while (cur != tree->nil){
                parent = cur;
                cmpval = tree->cmp_cb(key, cur->key);
                if (cmpval == 0){
                        return cur;
                }else if (cmpval < 0){
                        cur = cur->left;
                }else{
                        cur = cur->right;
                }
        }

        newnode = alloc_rbnode(tree);
        newnode->parent = parent;
        if (parent == tree->nil){
                tree->root = newnode;
        }else{
                cmpval = tree->cmp_cb(key, parent->key);
                if (cmpval < 0){
                        parent->left = newnode;
                }else{
                        parent->right = newnode;
                }
        }
        fix_insert(newnode, tree);

        switch (op){
        case INSERT_NEW:
                init_rbnode(key, data, newnode, tree);
                break;
        case INSERT_COPY:
                copy_rbnode(key, data, newnode, tree);
                break;
        default:
                PIOJO_ASSERT(FALSE);
                break;
        }
        return tree->nil;
}

static void
fix_insert(rbnode_t *node, piojo_tree_t *tree)
{
        rbnode_t *y;
        node->color = COLOR_RED;
        while (node != tree->root && node->parent->color == COLOR_RED){
                if (node->parent == node->parent->parent->left){
                        y = node->parent->parent->right;
                        if (y->color == COLOR_RED){
                                node->parent->color = COLOR_BLACK;
                                y->color = COLOR_BLACK;
                                node->parent->parent->color = COLOR_RED;
                                node = node->parent->parent;
                        }else{
                                if (node == node->parent->right){
                                        node = node->parent;
                                        rotate_left(node, tree);
                                }
                                node->parent->color = COLOR_BLACK;
                                node->parent->parent->color = COLOR_RED;
                                rotate_right(node->parent->parent, tree);
                        }
                }else{
                        y = node->parent->parent->left;
                        if (y->color == COLOR_RED){
                                node->parent->color = COLOR_BLACK;
                                y->color = COLOR_BLACK;
                                node->parent->parent->color = COLOR_RED;
                                node = node->parent->parent;
                        }else{
                                if (node == node->parent->left){
                                        node = node->parent;
                                        rotate_right(node, tree);
                                }
                                node->parent->color = COLOR_BLACK;
                                node->parent->parent->color = COLOR_RED;
                                rotate_left(node->parent->parent, tree);
                        }
                }
        }
        tree->root->color = COLOR_BLACK;
}

static rbnode_t*
search_node(const void *key, const piojo_tree_t *tree)
{
        int cmpval;
        rbnode_t *cur = tree->root;
        while (cur != tree->nil){
                cmpval = tree->cmp_cb(key, cur->key);
                if (cmpval == 0){
                        return cur;
                }else if (cmpval < 0){
                        cur = cur->left;
                }else{
                        cur = cur->right;
                }
        }
        return cur;
}

static rbnode_t*
search_max(rbnode_t *node, const piojo_tree_t *tree)
{
        while (node->right != tree->nil){
                node = node->right;
        }
        return node;
}

static rbnode_t*
search_min(rbnode_t *node, const piojo_tree_t *tree)
{
        while (node->left != tree->nil){
                node = node->left;
        }
        return node;
}

static rbnode_t*
next_node(rbnode_t *node, const piojo_tree_t *tree)
{
        rbnode_t *y;
        if (node->right != tree->nil){
                return search_min(node->right, tree);
        }

        y = node->parent;
        while (y != tree->nil && node == y->right){
                node = y;
                y = y->parent;
        }
        return y;
}

static rbnode_t*
prev_node(rbnode_t *node, const piojo_tree_t *tree)
{
        rbnode_t *y;
        if (node->left != tree->nil){
                return search_max(node->left, tree);
        }

        y = node->parent;
        while (y != tree->nil && node == y->left){
                node = y;
                y = y->parent;
        }
        return y;
}

static bool
delete_node(const void *key, piojo_tree_t *tree)
{
        rbnode_t *x, *y, *node = search_node(key, tree);
        if (node == tree->nil){
                return FALSE;
        }

        if (node->left == tree->nil || node->right == tree->nil){
                y = node;
        }else{
                y = next_node(node, tree);
        }
        if (y->left != tree->nil){
                x = y->left;
        }else{
                x = y->right;
        }

        x->parent = y->parent;
        if (y->parent == tree->nil){
                tree->root = x;
        }else if (y == y->parent->left){
                y->parent->left = x;
        }else{
                y->parent->right = x;
        }

        if (node != y){
                memcpy(node->key, y->key, tree->eksize);
                memcpy(node->value, y->value, tree->evsize);
        }
        if (y->color == COLOR_BLACK){
                fix_delete(x, tree);
        }
        free_rbnode(y, tree);
        return TRUE;
}

static void
fix_delete(rbnode_t *node, piojo_tree_t *tree)
{
        rbnode_t *y;
        while (node != tree->root && node->color == COLOR_BLACK){
                if (node == node->parent->left){
                        y = node->parent->right;
                        if (y->color == COLOR_RED){
                                y->color = COLOR_BLACK;
                                node->parent->color = COLOR_RED;
                                rotate_left(node->parent, tree);
                                y = node->parent->right;
                        }
                        if (y->left->color == COLOR_BLACK &&
                            y->right->color == COLOR_BLACK){
                                y->color = COLOR_RED;
                                node = node->parent;
                        }else{
                                if (y->right->color == COLOR_BLACK){
                                        y->left->color = COLOR_BLACK;
                                        y->color = COLOR_RED;
                                        rotate_right(y, tree);
                                        y = node->parent->right;
                                }
                                y->color = node->parent->color;
                                node->parent->color = COLOR_BLACK;
                                y->right->color = COLOR_BLACK;
                                rotate_left(node->parent, tree);
                                node = tree->root;
                        }
                }else{
                        y = node->parent->left;
                        if (y->color == COLOR_RED){
                                y->color = COLOR_BLACK;
                                node->parent->color = COLOR_RED;
                                rotate_right(node->parent, tree);
                                y = node->parent->left;
                        }
                        if (y->left->color == COLOR_BLACK &&
                            y->right->color == COLOR_BLACK){
                                y->color = COLOR_RED;
                                node = node->parent;
                        }else{
                                if (y->left->color == COLOR_BLACK){
                                        y->right->color = COLOR_BLACK;
                                        y->color = COLOR_RED;
                                        rotate_left(y, tree);
                                        y = node->parent->left;
                                }
                                y->color = node->parent->color;
                                node->parent->color = COLOR_BLACK;
                                y->left->color = COLOR_BLACK;
                                rotate_right(node->parent, tree);
                                node = tree->root;
                        }
                }
        }
        node->color = COLOR_BLACK;
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
