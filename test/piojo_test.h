#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <piojo/piojo_alloc.h>

/* Note that this enforces a read when 'x' is volatile. */
#define PIOJO_UNUSED(x) (void)(x)
#define PIOJO_ASSERT(cond) do{ assert(cond); } while(0)
#define PIOJO_FAIL_IF(cond) do{ assert(! (cond)); } while(0)
#define TEST_STRESS_COUNT 100000

static int alloc_cnt = 0;
static int init_cnt = 0;

static void*
my_alloc(size_t size)
{
        ++alloc_cnt;
        return piojo_alloc_def_alloc(size);
}

static void
my_free(const void *ptr)
{
        PIOJO_ASSERT(alloc_cnt > 0);
        --alloc_cnt;
        piojo_alloc_def_free(ptr);
}

static void
my_init(const void *data, size_t esize, void *newptr)
{
        ++init_cnt;
        piojo_alloc_def_init(data, esize, newptr);
}

static void
my_copy(const void *ptr, size_t esize, void *newptr)
{
        ++init_cnt;
        piojo_alloc_def_copy(ptr, esize, newptr);
}

static void
my_finish(void *ptr)
{
        PIOJO_ASSERT(init_cnt > 0);
        --init_cnt;
        piojo_alloc_def_finish(ptr);
}

#define assert_allocator_init(val) PIOJO_ASSERT(init_cnt == val)
#define assert_allocator_alloc(val) PIOJO_ASSERT(alloc_cnt == val)

static piojo_alloc_t my_allocator = {
        my_alloc,
        my_free,
        my_init,
        my_copy,
        my_finish,
};


static piojo_alloc_kv_t my_kvallocator = {
        my_alloc,
        my_free,
        my_init,
        my_copy,
        my_finish,
        my_init,
        my_copy,
        my_finish,
};
