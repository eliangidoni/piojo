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
 * Piojo List API.
 */

#ifndef PIOJO_LIST_H_
#define PIOJO_LIST_H_

#include <piojo/piojo_common.h>
#include <piojo/piojo_alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct piojo_list_node;
typedef struct piojo_list_node piojo_list_node_t;

struct piojo_list;
typedef struct piojo_list piojo_list_t;
extern const size_t piojo_list_sizeof;

piojo_list_t*
piojo_list_alloc(void);

piojo_list_t*
piojo_list_alloc_s(size_t esize);

piojo_list_t*
piojo_list_alloc_cb(size_t esize, piojo_alloc_t allocator);

piojo_list_t*
piojo_list_copy(const piojo_list_t *list);

void
piojo_list_free(const piojo_list_t *list);

void
piojo_list_clear(piojo_list_t *list);

size_t
piojo_list_size(const piojo_list_t *list);

piojo_list_node_t*
piojo_list_insert(const void *data, piojo_list_node_t *next,
                  piojo_list_t *list);

piojo_list_node_t*
piojo_list_set(const void *data, piojo_list_node_t *node,
               piojo_list_t *list);

piojo_list_node_t*
piojo_list_prepend(const void *data, piojo_list_t *list);

piojo_list_node_t*
piojo_list_append(const void *data, piojo_list_t *list);

piojo_list_node_t*
piojo_list_delete(const piojo_list_node_t *node, piojo_list_t *list);

piojo_list_node_t*
piojo_list_first(const piojo_list_t *list);

piojo_list_node_t*
piojo_list_last(const piojo_list_t *list);

piojo_list_node_t*
piojo_list_next(const piojo_list_node_t *node);

piojo_list_node_t*
piojo_list_prev(const piojo_list_node_t *node);

void*
piojo_list_entry(const piojo_list_node_t *node);

#ifdef __cplusplus
}
#endif
#endif
