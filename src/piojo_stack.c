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
 * @addtogroup piojostack Piojo Stack
 * @{
 * Piojo Stack implementation.
 */

#include <piojo/piojo_stack.h>
#include <piojo/piojo_array.h>
#include <piojo_defs.h>

struct piojo_stack {
        piojo_array_t *data;
        piojo_alloc_t allocator;
};
/** @hideinitializer Size of stack in bytes */
const size_t piojo_stack_sizeof = sizeof(piojo_stack_t);

/**
 * Allocates a new stack.
 * Uses default allocator and entry size of @b int.
 * @return New stack.
 */
piojo_stack_t*
piojo_stack_alloc(void)
{
        return piojo_stack_alloc_s(sizeof(int));
}

/**
 * Allocates a new stack.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @return New stack.
 */
piojo_stack_t*
piojo_stack_alloc_s(size_t esize)
{
        return piojo_stack_alloc_n(esize, DEFAULT_ADT_ECOUNT);
}

/**
 * Allocates a new stack.
 * Uses default allocator.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @return New stack.
 */
piojo_stack_t*
piojo_stack_alloc_n(size_t esize, size_t ecount)
{
        return piojo_stack_alloc_cb_n(esize, ecount, piojo_alloc_default);
}

/**
 * Allocates a new stack.
 * @param[in] esize Entry size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New stack.
 */
piojo_stack_t*
piojo_stack_alloc_cb(size_t esize, piojo_alloc_t allocator)
{
        return piojo_stack_alloc_cb_n(esize, DEFAULT_ADT_ECOUNT, allocator);
}

/**
 * Allocates a new stack.
 * @param[in] esize Entry size in bytes.
 * @param[in] ecount Number of entries to reserve space for.
 * @param[in] allocator Allocator to be used.
 * @return New stack.
 */
piojo_stack_t*
piojo_stack_alloc_cb_n(size_t esize, size_t ecount, piojo_alloc_t allocator)
{
        piojo_stack_t * st;
        st = (piojo_stack_t *) allocator.alloc_cb(sizeof(piojo_stack_t));
        PIOJO_ASSERT(st);
        PIOJO_ASSERT(esize > 0);
        PIOJO_ASSERT(ecount > 0);

        st->allocator = allocator;
        st->data = piojo_array_alloc_cb_n(esize, ecount, st->allocator);
        PIOJO_ASSERT(st->data);

        return st;
}

/**
 * Copies @a stack and all its entries.
 * @param[in] stack Stack being copied.
 * @return New stack.
 */
piojo_stack_t*
piojo_stack_copy(const piojo_stack_t *stack)
{
        piojo_alloc_t allocator;
        piojo_stack_t * newstack;
        PIOJO_ASSERT(stack);

        allocator = stack->allocator;
        newstack = (piojo_stack_t *) allocator.alloc_cb(sizeof(piojo_stack_t));
        PIOJO_ASSERT(newstack);

        newstack->allocator = allocator;
        newstack->data = piojo_array_copy(stack->data);
        PIOJO_ASSERT(newstack->data);

        return newstack;
}

/**
 * Frees @a stack and all its entries.
 * @param[in] stack Stack being freed.
 */
void
piojo_stack_free(const piojo_stack_t *stack)
{
        piojo_alloc_t allocator;
        PIOJO_ASSERT(stack);

        allocator = stack->allocator;
        piojo_array_free(stack->data);
        allocator.free_cb(stack);
}

/**
 * Deletes all entries in @a stack.
 * @param[out] stack Stack being cleared.
 */
void
piojo_stack_clear(piojo_stack_t *stack)
{
        PIOJO_ASSERT(stack);
        piojo_array_clear(stack->data);
}

/**
 * Returns number of entries.
 * @param[in] stack
 * @return Number of entries in @a stack.
 */
size_t
piojo_stack_size(const piojo_stack_t *stack)
{
        PIOJO_ASSERT(stack);
        return piojo_array_size(stack->data);
}

/**
 * Inserts a new entry after the last entry.
 * @param[in] data Entry value.
 * @param[out] stack Stack being modified.
 */
void
piojo_stack_push(const void *data, piojo_stack_t *stack)
{
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(data);
        piojo_array_append(data, stack->data);
}

/**
 * Deletes the last entry.
 * @param[out] stack Non-empty stack.
 */
void
piojo_stack_pop(piojo_stack_t *stack)
{
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) > 0);
        piojo_array_delete(piojo_array_size(stack->data) - 1, stack->data);
}

/**
 * Reads the last entry.
 * @param[in] stack Non-empty stack.
 * @return Entry value.
 */
void*
piojo_stack_peek(const piojo_stack_t *stack)
{
        PIOJO_ASSERT(stack);
        PIOJO_ASSERT(piojo_stack_size(stack) > 0);
        return piojo_array_at(piojo_array_size(stack->data) - 1, stack->data);
}

/** @} */
