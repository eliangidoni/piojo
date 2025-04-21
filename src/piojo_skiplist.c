/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 G. Elian Gidoni
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
 * @addtogroup piojolist Piojo Skip List
 * @{
 * Piojo Skip List implementation.
 */

#include <piojo/piojo_skiplist.h>
#include <piojo_defs.h>

struct piojo_skiplist_node_t {
        void *data, *key;
        int level;
        piojo_skiplist_node_t **nexts;
};

struct piojo_skiplist_t {
        piojo_skiplist_node_t *head;
        size_t eksize, esize, ecount;
        int level, max_levels;
        piojo_cmp_cb cmp_cb;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of list in bytes */
const size_t piojo_skiplist_sizeof = sizeof(piojo_skiplist_t);

static const int MAX_LEVELS = 24;

static int
i32_cmp(const void *e1, const void *e2);

static int
i64_cmp(const void *e1, const void *e2);

static int
siz_cmp(const void *e1, const void *e2);

static piojo_skiplist_node_t*
init_node(const void *key, const void *data, const piojo_skiplist_t *list, size_t level);

static void
finish_node(const piojo_skiplist_node_t *node, const piojo_skiplist_t *list);

static void
finish_all(const piojo_skiplist_t *list);

static piojo_skiplist_node_t*
alloc_node(const piojo_skiplist_t *list, size_t level);

static int
node_level(const piojo_skiplist_t *list);

/**
 * Allocates a new list.
 * Uses default allocator and key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_i32k(size_t evsize)
{
        return piojo_skiplist_alloc_cb_i32k(evsize,
                                            piojo_alloc_default);
}

/**
 * Allocates a new list.
 * Uses default allocator and key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_i64k(size_t evsize)
{
        return piojo_skiplist_alloc_cb_i64k(evsize,
                                            piojo_alloc_default);
}

/**
 * Allocates a new list.
 * Uses default allocator and key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_sizk(size_t evsize)
{
        return piojo_skiplist_alloc_cb_sizk(evsize,
                                            piojo_alloc_default);
}

/**
 * Allocates a new list.
 * Uses key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_cb_i32k(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_skiplist_alloc_cb_cmp(evsize,
                                           i32_cmp, sizeof(int32_t), allocator);
}

/**
 * Allocates a new list.
 * Uses key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_cb_i64k(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_skiplist_alloc_cb_cmp(evsize,
                                           i64_cmp, sizeof(int64_t), allocator);
}

/**
 * Allocates a new list.
 * Uses key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_cb_sizk(size_t evsize,
                         piojo_alloc_if allocator)
{
        return piojo_skiplist_alloc_cb_cmp(evsize,
                                           siz_cmp, sizeof(size_t), allocator);
}


/**
 * Allocates a new list.
 * @param[in] evsize Entry size in bytes.
 * @param[in] keycmp Entry key comparison function.
 * @param[in] eksize Entry key size.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize)
{
        return piojo_skiplist_alloc_cb_cmp(evsize, keycmp, eksize, piojo_alloc_default);
}

/**
 * Allocates a new list.
 * @param[in] evsize Entry size in bytes.
 * @param[in] keycmp Entry key comparison function.
 * @param[in] eksize Entry key size.
 * @param[in] allocator Allocator to be used.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_alloc_cb_cmp(size_t evsize, piojo_cmp_cb keycmp, size_t eksize,
        piojo_alloc_if allocator)
{
        piojo_skiplist_t * list;
        PIOJO_ASSERT(evsize > 0 && eksize > 0);

        list = (piojo_skiplist_t *) allocator.alloc_cb(sizeof(piojo_skiplist_t));
        PIOJO_ASSERT(list);

        list->allocator = allocator;
        list->esize = evsize;
        list->eksize = eksize;
        list->cmp_cb = keycmp;
        list->ecount = 0;
        list->level = 0;
        list->max_levels = MAX_LEVELS;

        list->head = alloc_node(list, list->max_levels);
        PIOJO_ASSERT(list->head);

        return list;
}

/**
 * Copies @a list and all its entries.
 * @param[in] list List being copied.
 * @return New list.
 */
piojo_skiplist_t*
piojo_skiplist_copy(const piojo_skiplist_t *list)
{
        piojo_skiplist_t *newlist;
        piojo_skiplist_node_t *current;
        PIOJO_ASSERT(list);

        newlist = piojo_skiplist_alloc_cb_cmp(list->esize, list->cmp_cb,
                list->eksize, list->allocator);
        PIOJO_ASSERT(newlist);

        current = list->head->nexts[0];
        while (current != NULL){
                piojo_skiplist_insert(current->key, current->data, newlist);
                current = current->nexts[0];
        }
        return newlist;
}

/**
 * Frees @a list and all its entries.
 * @param[in] list List being freed.
 */
void
piojo_skiplist_free(const piojo_skiplist_t *list)
{
        piojo_alloc_if allocator;
        PIOJO_ASSERT(list);

        allocator = list->allocator;

        finish_all(list);
        allocator.free_cb(list);
}

/**
 * Deletes all entries in @a list.
 * @param[out] list List being cleared.
 */
void
piojo_skiplist_clear(piojo_skiplist_t *list)
{
        PIOJO_ASSERT(list);
        finish_all(list);
        list->ecount = 0;
        list->head = alloc_node(list, list->max_levels);
        PIOJO_ASSERT(list->head);
}

/**
 * Returns number of entries.
 * @param[in] list
 * @return Number of entries in @a list.
 */
size_t
piojo_skiplist_size(const piojo_skiplist_t *list)
{
        PIOJO_ASSERT(list);
        return list->ecount;
}

/**
 * Inserts a new entry.
 * If @a data is @b NULL, the value is replaced with @b TRUE (useful for sets).
 * @param[in] key Entry key.
 * @param[in] data Entry value.
 * @param[out] list List being modified.
 * @return @b TRUE if inserted, @b FALSE if @a key is duplicate.
 */
bool
piojo_skiplist_insert(const void *key, const void *data, piojo_skiplist_t *list)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(list->ecount < SIZE_MAX);
        PIOJO_ASSERT(data || list->esize == sizeof(bool));

        piojo_skiplist_node_t **update = list->allocator.alloc_cb(sizeof(piojo_skiplist_node_t*) * (list->max_levels + 1));
        PIOJO_ASSERT(update);
        memset(update, 0, sizeof(piojo_skiplist_node_t*) * (list->max_levels + 1));

        piojo_skiplist_node_t *current = list->head;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL && list->cmp_cb(key, current->nexts[i]->key) > 0) {
                        current = current->nexts[i];
                }
                update[i] = current;
        }

        current = current->nexts[0];
        if (current != NULL && list->cmp_cb(key, current->key) == 0) {
                list->allocator.free_cb(update);
                return FALSE;
        }

        int new_level = node_level(list);
        if (new_level > list->level) {
                for (int i = list->level + 1; i <= new_level; i++) {
                        update[i] = list->head;
                }
                list->level = new_level;
        }

        piojo_skiplist_node_t *newnode = init_node(key, data, list, new_level);
        for (int i = 0; i <= new_level; i++) {
                newnode->nexts[i] = update[i]->nexts[i];
                update[i]->nexts[i] = newnode;
        }
        list->allocator.free_cb(update);
        ++list->ecount;
        return TRUE;
}

/**
 * Replaces or inserts an entry.
 * If @a data is @b NULL, the value is replaced with @b TRUE (useful for sets).
 * @param[in] key Entry key.
 * @param[in] data Entry value.
 * @param[out] list List being modified.
 * @return @b TRUE if @a key is new, @b FALSE otherwise.
 */
bool
piojo_skiplist_set(const void *key, const void *data, piojo_skiplist_t *list)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(data || list->esize == sizeof(bool));

        piojo_skiplist_node_t **update = list->allocator.alloc_cb(sizeof(piojo_skiplist_node_t*) * (list->max_levels + 1));
        PIOJO_ASSERT(update);
        memset(update, 0, sizeof(piojo_skiplist_node_t*) * (list->max_levels + 1));

        piojo_skiplist_node_t *current = list->head;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL && list->cmp_cb(key, current->nexts[i]->key) > 0) {
                        current = current->nexts[i];
                }
                update[i] = current;
        }

        current = current->nexts[0];
        if (current != NULL && list->cmp_cb(key, current->key) == 0) {
                if (data != NULL){
                        memcpy(current->data, data, list->esize);
                }
                list->allocator.free_cb(update);
                return FALSE;
        }

        int new_level = node_level(list);
        if (new_level > list->level) {
                for (int i = list->level + 1; i <= new_level; i++) {
                        update[i] = list->head;
                }
                list->level = new_level;
        }

        piojo_skiplist_node_t *newnode = init_node(key, data, list, new_level);
        for (int i = 0; i <= new_level; i++) {
                newnode->nexts[i] = update[i]->nexts[i];
                update[i]->nexts[i] = newnode;
        }
        list->allocator.free_cb(update);
        ++list->ecount;
        return TRUE;
}

/**
 * Searches an entry by key.
 * @param[in] key Entry key.
 * @param[in] list
 * @return Entry value or @b NULL if key doesn't exist.
 */
void*
piojo_skiplist_search(const void *key, const piojo_skiplist_t *list)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(key);

        piojo_skiplist_node_t *current = list->head;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL && list->cmp_cb(key, current->nexts[i]->key) > 0) {
                        current = current->nexts[i];
                }
        }
        current = current->nexts[0];
        if (current != NULL && list->cmp_cb(key, current->key) == 0) {
                return current->data;
        }
        return NULL;
}

/**
 * Deletes an entry by key.
 * @param[in] key Entry key.
 * @param[out] list
 * @return TRUE if deleted, FALSE if @a key doesn't exist.
 */
bool
piojo_skiplist_delete(const void *key, piojo_skiplist_t *list)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(list->ecount > 0);

        piojo_skiplist_node_t **update = list->allocator.alloc_cb(sizeof(piojo_skiplist_node_t*) * (list->max_levels + 1));
        PIOJO_ASSERT(update);
        memset(update, 0, sizeof(piojo_skiplist_node_t*) * (list->max_levels + 1));
        piojo_skiplist_node_t *current = list->head;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL && list->cmp_cb(key, current->nexts[i]->key) > 0) {
                        current = current->nexts[i];
                }
                update[i] = current;
        }

        current = current->nexts[0];
        if (current != NULL && list->cmp_cb(key, current->key) == 0) {
                for (int i = 0; i <= list->level; i++) {
                        if (update[i]->nexts[i] != current) {
                                break;
                        }
                        update[i]->nexts[i] = current->nexts[i];
                }
                while (list->level > 0 && list->head->nexts[list->level] == NULL) {
                        list->level--;
                }
                --list->ecount;
                finish_node(current, list);
                list->allocator.free_cb(update);
                return TRUE;
        }
        list->allocator.free_cb(update);
        return FALSE;
}

/**
 * Reads the first key in @a list (order given by @a keycmp function).
 * @param[in] list
 * @param[out] data Entry value, can be @b NULL.
 * @return first key or @b NULL if @a list is empty.
 */
const void*
piojo_skiplist_first(const piojo_skiplist_t *list, void **data)
{
        PIOJO_ASSERT(list);
        // skip sentinel
        if (list->head->nexts[0] != NULL){
                if (data != NULL){
                        *data = list->head->nexts[0]->data;
                }
                return list->head->nexts[0]->key;
        }
        return NULL;
}

/**
 * Reads the last key in @a list (order given by @a keycmp function).
 * @param[in] list
 * @param[out] data Entry value, can be @b NULL.
 * @return last key or @b NULL if @a list is empty.
 */
const void*
piojo_skiplist_last(const piojo_skiplist_t *list, void **data)
{
        PIOJO_ASSERT(list);

        piojo_skiplist_node_t *current = list->head;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL) {
                        current = current->nexts[i];
                }
        }
        if (current != NULL && current != list->head) {
                if (data != NULL){
                        *data = current->data;
                }
                return current->key;
        }
        return NULL;
}

/**
 * Reads the next key (order given by @a keycmp function).
 * @param[in] key
 * @param[in] list
 * @param[out] data Entry value, can be @b NULL.
 * @return next key or @b NULL if @a key is the last one.
 */
const void*
piojo_skiplist_next(const void *key, const piojo_skiplist_t *list, void **data)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(key);

        piojo_skiplist_node_t *current = list->head;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL && list->cmp_cb(key, current->nexts[i]->key) >= 0) {
                        current = current->nexts[i];
                }
        }
        current = current->nexts[0];
        if (current != NULL) {
                if (data != NULL){
                        *data = current->data;
                }
                return current->key;
        }
        return NULL;
}

/**
 * Reads the previous key (order given by @a keycmp function).
 * @param[in] key
 * @param[in] list
 * @param[out] data Entry value, can be @b NULL.
 * @return previous key or @b NULL if @a key is the first one.
 */
const void*
piojo_skiplist_prev(const void *key, const piojo_skiplist_t *list, void **data)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(key);
        piojo_skiplist_node_t *current = list->head, *prev = NULL;
        for (int i = list->level; i >= 0; i--) {
                while (current->nexts[i] != NULL && list->cmp_cb(key, current->nexts[i]->key) > 0) {
                        current = current->nexts[i];
                }
                prev = current;
        }

        if (prev != NULL && prev != list->head) {
                if (data != NULL) {
                        *data = prev->data;
                }
                return prev->key;
        }
        return NULL;
}

/** @}
 * Private functions.
 */

static piojo_skiplist_node_t*
init_node(const void *key, const void *data, const piojo_skiplist_t *list, size_t level)
{
        bool null_p = TRUE;
        if (data == NULL){
                data = &null_p;
        }
        piojo_skiplist_node_t *node = alloc_node(list, level);
        memcpy(node->data, data, list->esize);
        memcpy(node->key, key, list->eksize);
        return node;
}

static void
finish_node(const piojo_skiplist_node_t *node, const piojo_skiplist_t *list)
{
        list->allocator.free_cb(node->data);
        list->allocator.free_cb(node->key);
        list->allocator.free_cb(node->nexts);
        list->allocator.free_cb(node);
}

static void
finish_all(const piojo_skiplist_t *list)
{
        piojo_skiplist_node_t *next, *node;
        node = list->head;
        while (node != NULL){
                next = node->nexts[0];
                finish_node(node, list);
                node = next;
        }
}

static piojo_skiplist_node_t*
alloc_node(const piojo_skiplist_t *list, size_t level)
{
        piojo_skiplist_node_t *node, **nexts;
        void *ndata, *nkey;
        piojo_alloc_if ator = list->allocator;

        ndata = ator.alloc_cb(list->esize);
        PIOJO_ASSERT(ndata);

        nkey = ator.alloc_cb(list->eksize);
        PIOJO_ASSERT(nkey);

        nexts = (piojo_skiplist_node_t**) ator.alloc_cb(sizeof(piojo_skiplist_node_t*) * (level + 1));
        PIOJO_ASSERT(nexts);
        memset(nexts, 0, sizeof(piojo_skiplist_node_t*) * (level + 1));

        node = (piojo_skiplist_node_t*) ator.alloc_cb(sizeof(piojo_skiplist_node_t));
        PIOJO_ASSERT(node);

        node->nexts = nexts;
        node->level = level;
        node->data = ndata;
        node->key = nkey;

        return node;
}


static int
node_level(const piojo_skiplist_t *list)
{
        int new_level = 0;
        while (new_level < list->max_levels && (rand() % 2) == 0) {
                new_level++;
        }
        return new_level;
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
