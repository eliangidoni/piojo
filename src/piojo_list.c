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
 * @addtogroup piojolist Piojo List
 * @{
 * Piojo List implementation.
 */

#include <piojo/piojo_list.h>
#include <piojo_defs.h>

struct piojo_list_node_t {
        void *data;
        piojo_list_node_t *next, *prev;
};

struct piojo_list_t {
        piojo_list_node_t *head, *tail;
        size_t esize, ecount;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of list in bytes */
const size_t piojo_list_sizeof = sizeof(piojo_list_t);

static piojo_list_node_t*
init_node(const void *data, const piojo_list_t *list);

static piojo_list_node_t*
copy_node(const piojo_list_node_t *node, const piojo_list_t *newlist);

static void
finish_node(const piojo_list_node_t *node, const piojo_list_t *list);

static void
finish_all(const piojo_list_t *list);

static piojo_list_node_t*
alloc_node(const piojo_list_t *list);

/**
 * Allocates a new list.
 * Uses default allocator and entry size of @b int.
 * @return New list.
 */
piojo_list_t*
piojo_list_alloc(void)
{
        return piojo_list_alloc_s(sizeof(int));
}

/**
 * Allocates a new list.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @return New list.
 */
piojo_list_t*
piojo_list_alloc_s(size_t esize)
{
        return piojo_list_alloc_cb(esize, piojo_alloc_default);
}

/**
 * Allocates a new list.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New list.
 */
piojo_list_t*
piojo_list_alloc_cb(size_t esize, piojo_alloc_if allocator)
{
        piojo_list_t * list;
        list = (piojo_list_t *) allocator.alloc_cb(sizeof(piojo_list_t));
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(esize > 0);

        list->allocator = allocator;
        list->esize = esize;
        list->ecount = 0;
        list->head = alloc_node(list);
        list->tail = alloc_node(list);
        list->head->next = list->tail;
        list->tail->prev = list->head;

        return list;
}

/**
 * Copies @a list and all its entries.
 * @param[in] list List being copied.
 * @return New list.
 */
piojo_list_t*
piojo_list_copy(const piojo_list_t *list)
{
        piojo_list_node_t *next, *node, *newnode;
        piojo_list_t *newlist;
        PIOJO_ASSERT(list);

        newlist = piojo_list_alloc_cb(list->esize, list->allocator);
        PIOJO_ASSERT(newlist);

        node = list->head->next;
        while (node != list->tail){
                next = node->next;
                newnode = copy_node(node, newlist);
                newnode->prev = newlist->tail->prev;
                newnode->prev->next = newnode;
                newnode->next = newlist->tail;
                newlist->tail->prev = newnode;
                node = next;
        }
        newlist->ecount = list->ecount;

        return newlist;
}

/**
 * Frees @a list and all its entries.
 * @param[in] list List being freed.
 */
void
piojo_list_free(const piojo_list_t *list)
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
piojo_list_clear(piojo_list_t *list)
{
        PIOJO_ASSERT(list);
        finish_all(list);
        list->ecount = 0;
        list->head = alloc_node(list);
        list->tail = alloc_node(list);
        list->head->next = list->tail;
        list->tail->prev = list->head;
}

/**
 * Returns number of entries.
 * @param[in] list
 * @return Number of entries in @a list.
 */
size_t
piojo_list_size(const piojo_list_t *list)
{
        PIOJO_ASSERT(list);
        return list->ecount;
}

/**
 * Inserts a new entry.
 * @param[in] data Entry value.
 * @param[in] next Node following the entry being inserted.
 * @param[out] list List being modified.
 * @return Node of inserted entry.
 */
piojo_list_node_t*
piojo_list_insert(const void *data, piojo_list_node_t *next,
                  piojo_list_t *list)
{
        piojo_list_node_t * node;
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(next);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(list->ecount < SIZE_MAX);

        node = init_node(data, list);
        node->next = next;
        node->prev = next->prev;
        next->prev = node;
        node->prev->next = node;
        ++list->ecount;

        return node;
}

/**
 * Replaces an entry.
 * @param[in] data Entry value.
 * @param[in] node Node being replaced.
 * @param[out] list List being modified.
 * @return @a node.
 */
piojo_list_node_t*
piojo_list_set(const void *data, piojo_list_node_t *node,
               piojo_list_t *list)
{
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(node);
        PIOJO_ASSERT(data);

        memcpy(node->data, data, list->esize);

        return node;
}

/**
 * Inserts a new entry at the beginning of @a list.
 * @param[in] data Entry value.
 * @param[out] list List being modified.
 * @return Node of inserted entry.
 */
piojo_list_node_t*
piojo_list_prepend(const void *data, piojo_list_t *list)
{
        piojo_list_node_t * node;
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(list->ecount < SIZE_MAX);

        node = init_node(data, list);
        node->next = list->head->next;
        node->next->prev = node;
        node->prev = list->head;
        list->head->next = node;

        ++list->ecount;
        return node;
}

/**
 * Inserts a new entry at the end of @a list.
 * @param[in] data Entry value.
 * @param[out] list List being modified.
 * @return Node of inserted entry.
 */
piojo_list_node_t*
piojo_list_append(const void *data, piojo_list_t *list)
{
        piojo_list_node_t *node;
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(data);
        PIOJO_ASSERT(list->ecount < SIZE_MAX);

        node = init_node(data, list);
        node->prev = list->tail->prev;
        node->prev->next = node;
        node->next = list->tail;
        list->tail->prev = node;
        ++list->ecount;

        return node;
}

/**
 * Deletes a node.
 * @param[in] node Node of the entry being deleted.
 * @param[out] list Non-empty list.
 * @return Next node, @b NULL if @a list is now empty.
 */
piojo_list_node_t*
piojo_list_delete(const piojo_list_node_t *node, piojo_list_t *list)
{
        piojo_list_node_t *next;
        PIOJO_ASSERT(list);
        PIOJO_ASSERT(node);
        PIOJO_ASSERT(list->ecount > 0);

        next = node->next;
        next->prev = node->prev;
        next->prev->next = next;
        --list->ecount;
        finish_node(node, list);
        if (list->ecount == 0){
                return NULL;
        }
        return next;
}

/**
 * Reads the first node in @a list.
 * @param[in] list
 * @return First node, @b NULL if @a list is empty.
 */
piojo_list_node_t*
piojo_list_first(const piojo_list_t *list)
{
        PIOJO_ASSERT(list);
        if (list->ecount == 0){
                return NULL;
        }
        return list->head->next;
}

/**
 * Reads the last node in @a list.
 * @param[in] list
 * @return Last node, @b NULL if @a list is empty.
 */
piojo_list_node_t*
piojo_list_last(const piojo_list_t *list)
{
        PIOJO_ASSERT(list);
        if (list->ecount == 0){
                return NULL;
        }
        return list->tail->prev;
}

/**
 * Reads the next node.
 * @param[in] node
 * @return Next node, @b NULL if @a node is the last one.
 */
piojo_list_node_t*
piojo_list_next(const piojo_list_node_t *node)
{
        PIOJO_ASSERT(node);
        if (node->next->next == NULL){
                return NULL;
        }
        return node->next;
}

/**
 * Reads the previous node.
 * @param[in] node
 * @return Previous node, @b NULL if @a node is the first one.
 */
piojo_list_node_t*
piojo_list_prev(const piojo_list_node_t *node)
{
        PIOJO_ASSERT(node);
        if (node->prev->prev == NULL){
                return NULL;
        }
        return node->prev;
}

/**
 * Reads entry from @a node.
 * @param[in] node
 * @return Entry value.
 */
void*
piojo_list_entry(const piojo_list_node_t *node)
{
        PIOJO_ASSERT(node);
        return node->data;
}

/** @}
 * Private functions.
 */

static piojo_list_node_t*
init_node(const void *data, const piojo_list_t *list)
{
        piojo_list_node_t *node = alloc_node(list);
        memcpy(node->data, data, list->esize);
        return node;
}

static piojo_list_node_t*
copy_node(const piojo_list_node_t *node, const piojo_list_t *newlist)
{
        piojo_list_node_t *newnode = alloc_node(newlist);
        memcpy(newnode->data, node->data, newlist->esize);
        return newnode;
}

static void
finish_node(const piojo_list_node_t *node, const piojo_list_t *list)
{
        list->allocator.free_cb(node->data);
        list->allocator.free_cb(node);
}

static void
finish_all(const piojo_list_t *list)
{
        piojo_list_node_t *next, *node;
        node = list->head;
        while (node != NULL){
                next = node->next;
                finish_node(node, list);
                node = next;
        }
}

static piojo_list_node_t*
alloc_node(const piojo_list_t *list)
{
        piojo_list_node_t *node;
        void *ndata;
        piojo_alloc_if ator = list->allocator;

        ndata = ator.alloc_cb(list->esize);
        PIOJO_ASSERT(ndata);

        node = (piojo_list_node_t*) ator.alloc_cb(sizeof(piojo_list_node_t));
        PIOJO_ASSERT(node);

        node->prev = NULL;
        node->next = NULL;
        node->data = ndata;

        return node;
}
