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
 * @addtogroup piojohash Piojo Hash Table
 * @{
 * Piojo Hash Table implementation.
 */

#include <piojo/piojo_hash.h>
#include <piojo_defs.h>

typedef struct entry_t entry_t;
struct entry_t {
        void *key, *value;
        entry_t *next;
};

typedef struct {
        size_t bidx;
        entry_t *prev;
        const piojo_hash_t *table;
} iter_t;

struct piojo_hash_t {
        entry_t **buckets;
        size_t eksize, evsize, ecount, bucketcnt;
        piojo_eq_cb eq_cb;
        piojo_alloc_if allocator;
};
/** @hideinitializer Size of hash table in bytes */
const size_t piojo_hash_sizeof = sizeof(piojo_hash_t);

static const size_t DEFAULT_BUCKET_COUNT = 32;
static const double LOAD_RATIO_MAX = 0.8f;

static uint32_t
calc_hash(const unsigned char *buf, size_t len);

static piojo_hash_t*
alloc_hash(size_t evsize, piojo_eq_cb keyeq, size_t eksize,
           piojo_alloc_if allocator, size_t bucketcnt);

static entry_t*
insert_entry(entry_t *newkv, insert_t op, piojo_hash_t *hash);

static iter_t
search_entry(const void *key, const piojo_hash_t *hash);

static void
delete_entry(iter_t iter, piojo_hash_t *hash);

static entry_t*
init_entry(const void *key, const void *data, const piojo_hash_t *hash);

static entry_t*
copy_entry(const void *key, const void *data, const piojo_hash_t *hash);

static void
finish_entry(const piojo_hash_t *hash, entry_t *kv);

static void
finish_all(const piojo_hash_t *hash);

static iter_t*
next_valid_entry(iter_t *iter);

static void
expand_table(piojo_hash_t *hash);

static bool
i32_eq(const void *e1, const void *e2);

static bool
i64_eq(const void *e1, const void *e2);

static bool
siz_eq(const void *e1, const void *e2);

/**
 * Allocates a new hash table.
 * Uses default allocator and key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_i32k(size_t evsize)
{
        return piojo_hash_alloc_cb_i32k(evsize, piojo_alloc_default);
}

/**
 * Allocates a new hash table.
 * Uses default allocator and key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_i64k(size_t evsize)
{
        return piojo_hash_alloc_cb_i64k(evsize, piojo_alloc_default);
}

/**
 * Allocates a new hash table.
 * Uses default allocator and key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_sizk(size_t evsize)
{
        return piojo_hash_alloc_cb_sizk(evsize, piojo_alloc_default);
}

/**
 * Allocates a new hash table.
 * Uses key size of @b int32_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_cb_i32k(size_t evsize, piojo_alloc_if allocator)
{
        return piojo_hash_alloc_cb_eq(evsize, i32_eq, sizeof(int32_t),
                                      allocator);
}

/**
 * Allocates a new hash table.
 * Uses key size of @b int64_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_cb_i64k(size_t evsize, piojo_alloc_if allocator)
{
        return piojo_hash_alloc_cb_eq(evsize, i64_eq, sizeof(int64_t),
                                      allocator);
}

/**
 * Allocates a new hash table.
 * Uses key size of @b size_t.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_cb_sizk(size_t evsize, piojo_alloc_if allocator)
{
        return piojo_hash_alloc_cb_eq(evsize, siz_eq, sizeof(size_t),
                                      allocator);
}

/**
 * Allocates a new hash table.
 * Uses default allocator.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] keyeq Entry key equality function.
 * @param[in] eksize Entry key size in bytes.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_eq(size_t evsize, piojo_eq_cb keyeq, size_t eksize)
{
        return piojo_hash_alloc_cb_eq(evsize, keyeq, eksize,
                                      piojo_alloc_default);
}

/**
 * Allocates a new hash table.
 * @param[in] evsize Entry value size in bytes.
 * @param[in] keyeq Entry key equality function.
 * @param[in] eksize Entry key size in bytes.
 * @param[in] allocator Allocator to be used.
 * @return New hash table.
 */
piojo_hash_t*
piojo_hash_alloc_cb_eq(size_t evsize, piojo_eq_cb keyeq, size_t eksize,
                       piojo_alloc_if allocator)
{
        return alloc_hash(evsize, keyeq, eksize, allocator,
                          DEFAULT_BUCKET_COUNT);
}

/**
 * Copies @a hash and all its entries.
 * @param[in] hash Hash table being copied.
 * @return New hash.
 */
piojo_hash_t*
piojo_hash_copy(const piojo_hash_t *hash)
{
        piojo_hash_t *newhash;
        size_t bidx;
        entry_t *kv;
        PIOJO_ASSERT(hash);

        newhash = alloc_hash(hash->evsize, hash->eq_cb, hash->eksize,
                             hash->allocator, hash->bucketcnt);

        for (bidx = 0; bidx < hash->bucketcnt; ++bidx){
                kv = hash->buckets[bidx];
                while (kv != NULL){
                        insert_entry(kv, INSERT_COPY, newhash);
                        kv = kv->next;
                }
        }

        newhash->ecount = hash->ecount;

        return newhash;
}

/**
 * Frees @a hash and all its entries.
 * @param[in] hash Hash table being freed.
 */
void
piojo_hash_free(const piojo_hash_t *hash)
{
        PIOJO_ASSERT(hash);

        finish_all(hash);

        hash->allocator.free_cb(hash->buckets);
        hash->allocator.free_cb(hash);
}

/**
 * Deletes all entries in @a hash.
 * @param[out] hash Hash table being cleared.
 */
void
piojo_hash_clear(piojo_hash_t *hash)
{
        PIOJO_ASSERT(hash);

        finish_all(hash);

        memset(hash->buckets, 0, hash->bucketcnt * sizeof(entry_t*));
        hash->ecount = 0;
}

/**
 * Returns number of entries.
 * @param[in] hash Hash table
 * @return Number of entries in @a hash.
 */
size_t
piojo_hash_size(const piojo_hash_t *hash)
{
        PIOJO_ASSERT(hash);
        return hash->ecount;
}

/**
 * Inserts a new entry.
 * If @a data is @b NULL, the value is replaced with @b TRUE (useful for sets).
 * @param[in] key Entry key.
 * @param[in] data Entry value.
 * @param[out] hash Hash table being modified.
 * @return @b TRUE if inserted, @b FALSE if @a key is duplicate.
 */
bool
piojo_hash_insert(const void *key, const void *data, piojo_hash_t *hash)
{
        entry_t kv;
        double lratio;
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(data || hash->evsize == sizeof(bool));

        lratio = (double) hash->ecount / hash->bucketcnt;
        if (lratio > LOAD_RATIO_MAX){
                expand_table(hash);
        }

        kv.key = (void*) key;
        kv.value = (void*) data;
        if (insert_entry(&kv, INSERT_NEW, hash) == NULL){
                ++hash->ecount;
                return TRUE;
        }
        return FALSE;
}

/**
 * Replaces or inserts an entry.
 * If @a data is @b NULL, the value is replaced with @b TRUE (useful for sets).
 * @param[in] key Entry key.
 * @param[in] data Entry value.
 * @param[out] hash Hash table being modified.
 * @return @b TRUE if @a key is new, @b FALSE otherwise.
 */
bool
piojo_hash_set(const void *key, const void *data, piojo_hash_t *hash)
{
        entry_t kv, *oldkv;
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(key);
        PIOJO_ASSERT(data || hash->evsize == sizeof(bool));

        kv.key = (void*) key;
        kv.value = (void*) data;
        oldkv = insert_entry(&kv, INSERT_NEW, hash);
        if (oldkv == NULL){
                ++hash->ecount;
                return TRUE;
        }

        if (data != NULL){
                memcpy(oldkv->value, data, hash->evsize);
        }
        return FALSE;
}

/**
 * Searches an entry by key.
 * @param[in] key Entry key.
 * @param[in] hash Hash table.
 * @return Entry value or @b NULL if key doesn't exist.
 */
void*
piojo_hash_search(const void *key, const piojo_hash_t *hash)
{
        iter_t iter;
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(key);

        iter = search_entry(key, hash);
        if (iter.table != NULL){
                if (iter.prev == NULL){
                        return hash->buckets[iter.bidx]->value;
                }
                return iter.prev->next->value;
        }
        return NULL;
}

/**
 * Deletes an entry by key.
 * @param[in] key Entry key.
 * @param[out] hash Hash table.
 * @return @b TRUE if deleted, @b FALSE if @a key doesn't exist.
 */
bool
piojo_hash_delete(const void *key, piojo_hash_t *hash)
{
        iter_t iter;
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(key);

        iter = search_entry(key, hash);
        if (iter.table != NULL){
                delete_entry(iter, hash);
                --hash->ecount;
                return TRUE;
        }
        return FALSE;
}

/**
 * Reads the first entry in @a hash.
 * @param[in] hash Hash table.
 * @param[out] data Entry value, can be @b NULL.
 * @return @b first key or @b NULL if @a hash is empty.
 */
const void*
piojo_hash_first(const piojo_hash_t *hash, void **data)
{
        size_t bidx = 0;
        iter_t *iter;
        PIOJO_ASSERT(hash);

        if (hash->ecount > 0){
                while (hash->buckets[bidx] == NULL){
                        ++bidx;
                }
                if (data != NULL){
                        *data = hash->buckets[bidx]->value;
                }
                return hash->buckets[bidx]->key;
        }
        return NULL;

}
/**
 * Reads the next entry.
 * @param[in] key
 * @param[in] hash Hash table.
 * @param[out] data Entry value, can be @b NULL.
 * @return @b next key or @b NULL if @a key is the last one.
 */
const void*
piojo_hash_next(const void *key, const piojo_hash_t *hash, void **data)
{
        iter_t iter;
        entry_t *kv;
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(key);

        iter = search_entry(key, hash);
        PIOJO_ASSERT(iter.table != NULL);

        if (iter.prev == NULL){
                iter.prev = hash->buckets[iter.bidx];
        }else{
                iter.prev = iter.prev->next;
        }
        if (next_valid_entry(&iter) != NULL){
                if (iter.prev == NULL){
                        kv = hash->buckets[iter.bidx];
                }else{
                        kv = iter.prev->next;
                }
                if (data != NULL){
                        *data = kv->value;
                }
                return kv->key;
        }
        return NULL;
}

/** @}
 * Private functions.
 */

/* 32 bit Fowler/Noll/Vo FNV-1a hash on a buffer. */
static uint32_t
calc_hash(const unsigned char *buf, size_t len)
{
        uint32_t hval = (uint32_t)0x811c9dc5;
        unsigned char *bp = (unsigned char *)buf;
        unsigned char *be = bp + len;
        while (bp < be) {
                /* xor the bottom with the current octet */
                hval ^= (uint32_t)*bp++;
                /* multiply by the 32 bit FNV magic prime mod 2^32 */
                hval *= (uint32_t)0x01000193;
        }
        return hval;
}

static iter_t*
next_valid_entry(iter_t *iter)
{
        if (iter->prev->next == NULL){
                iter->prev = NULL;
                ++iter->bidx;
                while (iter->bidx < iter->table->bucketcnt &&
                       iter->table->buckets[iter->bidx] == NULL){
                        ++iter->bidx;
                }
                if (iter->bidx >= iter->table->bucketcnt){
                        return NULL;
                }
        }
        return iter;
}

static entry_t*
init_entry(const void *key, const void *data, const piojo_hash_t *hash)
{
        bool null_p = TRUE;
        entry_t *kv;
        piojo_alloc_if ator = hash->allocator;
        size_t kvsize, ksize = hash->eksize;

        if (data == NULL){
                data = &null_p;
        }

        PIOJO_ASSERT(piojo_safe_addsiz_p(sizeof(entry_t), ksize));
        kvsize = sizeof(entry_t) + ksize;
        PIOJO_ASSERT(piojo_safe_addsiz_p(kvsize, hash->evsize));
        kvsize += hash->evsize;

        kv = (entry_t*) ator.alloc_cb(kvsize);
        PIOJO_ASSERT(kv);

        kv->key = (uint8_t*)kv + sizeof(entry_t);
        memcpy(kv->key, key, ksize);

        kv->value = (uint8_t*)kv->key + ksize;
        memcpy(kv->value, data, hash->evsize);

        kv->next = NULL;

        return kv;
}

static entry_t*
copy_entry(const void *key, const void *data, const piojo_hash_t *hash)
{
        entry_t *kv;
        piojo_alloc_if ator = hash->allocator;
        size_t ksize = hash->eksize;

        kv = ((entry_t*) ator.alloc_cb(sizeof(entry_t) +
                                       ksize + hash->evsize));
        PIOJO_ASSERT(kv);

        kv->key = (uint8_t*)kv + sizeof(entry_t);
        memcpy(kv->key, key, ksize);

        kv->value = (uint8_t*)kv->key + ksize;
        memcpy(kv->value, data, hash->evsize);

        kv->next = NULL;

        return kv;
}

static void
finish_entry(const piojo_hash_t *hash, entry_t *kv)
{
        piojo_alloc_if ator = hash->allocator;

        ator.free_cb(kv);
}

static void
finish_all(const piojo_hash_t *hash)
{
        size_t bidx;
        entry_t *kv, *next;
        for (bidx = 0; bidx < hash->bucketcnt; ++bidx){
                kv = hash->buckets[bidx];
                while (kv != NULL){
                        next = kv->next;
                        finish_entry(hash, kv);
                        kv = next;
                }
        }
}

static void
expand_table(piojo_hash_t *hash)
{
        size_t bidx;
        entry_t *kv, *nextkv;
        entry_t **olddata = hash->buckets;
        size_t newcnt, newsiz, oldcnt = hash->bucketcnt;
        piojo_alloc_if ator = hash->allocator;
        PIOJO_ASSERT(hash->bucketcnt < SIZE_MAX);

        newcnt = hash->bucketcnt / ADT_GROWTH_DENOMINATOR;
        PIOJO_ASSERT(piojo_safe_addsiz_p(hash->bucketcnt, newcnt));
        hash->bucketcnt += newcnt;

        PIOJO_ASSERT(piojo_safe_mulsiz_p(hash->bucketcnt,
                                         sizeof(entry_t*)));
        newsiz = hash->bucketcnt * sizeof(entry_t*);

        hash->buckets = (entry_t**) ator.alloc_cb(newsiz);
        PIOJO_ASSERT(hash->buckets);
        memset(hash->buckets, 0, newsiz);

        for (bidx = 0; bidx < oldcnt; ++bidx){
                kv = olddata[bidx];
                while (kv != NULL){
                        nextkv = kv->next;
                        kv->next = NULL;
                        insert_entry(kv, INSERT_PTR, hash);
                        kv = nextkv;
                }
        }

        ator.free_cb(olddata);
}

#ifdef PIOJO_DEBUG
void
print_hash(piojo_hash_t *hash){
        size_t i;
        entry_t *kv;
        for (i = 0; i < hash->bucketcnt; ++i){
                kv = hash->buckets[i];
                printf ("Bucket %lu: ", i);
                while (kv != NULL){
                        printf("%d=%d, ", *(int*)kv->key, *(int*)kv->value);
                        kv = kv->next;
                }
                printf("\n");
        }
        printf("\n");
}
#endif /* PIOJO_DEBUG */


static entry_t*
insert_entry(entry_t *newkv, insert_t op, piojo_hash_t *hash)
{
        size_t bidx;
        entry_t *kv;

        bidx = calc_hash((const unsigned char*)newkv->key,
                         hash->eksize) % hash->bucketcnt;

        kv = hash->buckets[bidx];
        if (kv != NULL){
                if (hash->eq_cb(newkv->key, kv->key)){
                        return kv;
                }
                while (kv->next != NULL){
                        kv = kv->next;
                        if (hash->eq_cb(newkv->key, kv->key)){
                                return kv;
                        }
                }
        }

        switch (op){
        case INSERT_NEW:
                newkv = init_entry(newkv->key, newkv->value, hash);
                break;
        case INSERT_COPY:
                newkv = copy_entry(newkv->key, newkv->value, hash);
                break;
        case INSERT_PTR:
                /* Nothing to do. */
                break;
        default:
                PIOJO_ASSERT(FALSE);
                break;
        }

        if (kv != NULL){
                kv->next = newkv;
        }else{
                hash->buckets[bidx] = newkv;
        }

        return NULL;
}

static iter_t
search_entry(const void *key, const piojo_hash_t *hash)
{
        size_t bidx;
        iter_t iter;
        entry_t *kv, *prevkv=NULL;
        PIOJO_ASSERT(hash);
        PIOJO_ASSERT(key);

        iter.table = NULL;

        bidx = calc_hash((const unsigned char*)key,
                         hash->eksize) % hash->bucketcnt;
        kv = hash->buckets[bidx];
        while (kv != NULL){
                if (hash->eq_cb(key, kv->key)){
                        iter.table = hash;
                        iter.bidx = bidx;
                        iter.prev = prevkv;
                        return iter;
                }
                prevkv = kv;
                kv = kv->next;
        }

        return iter;
}

static void
delete_entry(iter_t iter, piojo_hash_t *hash)
{
        entry_t *kv;
        if (iter.prev != NULL){
                kv = iter.prev->next;
                iter.prev->next = kv->next;
        }else{
                kv = hash->buckets[iter.bidx];
                hash->buckets[iter.bidx] = kv->next;
        }

        finish_entry(hash, kv);
}

static piojo_hash_t*
alloc_hash(size_t evsize, piojo_eq_cb keyeq, size_t eksize,
           piojo_alloc_if allocator, size_t bucketcnt)
{

        piojo_hash_t * hash;
        size_t size;
        PIOJO_ASSERT(eksize > 0 && evsize > 0);
        PIOJO_ASSERT(bucketcnt > 0);
        PIOJO_ASSERT(piojo_safe_mulsiz_p(bucketcnt,
                                         sizeof(entry_t*)));
        size = bucketcnt * sizeof(entry_t*);

        hash = (piojo_hash_t *) allocator.alloc_cb(sizeof(piojo_hash_t));
        PIOJO_ASSERT(hash);

        hash->allocator = allocator;
        hash->eksize = eksize;
        hash->evsize = evsize;
        hash->ecount = 0;
        hash->eq_cb = keyeq;
        hash->bucketcnt = bucketcnt;
        hash->buckets = (entry_t**) allocator.alloc_cb(size);
        PIOJO_ASSERT(hash->buckets);
        memset(hash->buckets, 0, size);

        return hash;
}

/*
 * Private equality functions.
 */

static bool
i32_eq(const void *e1, const void *e2)
{
        int32_t v1 = *(int32_t*) e1;
        int32_t v2 = *(int32_t*) e2;
        if (v1 == v2){
                return TRUE;
        }
        return FALSE;
}

static bool
i64_eq(const void *e1, const void *e2)
{
        int64_t v1 = *(int64_t*) e1;
        int64_t v2 = *(int64_t*) e2;
        if (v1 == v2){
                return TRUE;
        }
        return FALSE;
}

static bool
siz_eq(const void *e1, const void *e2)
{
        size_t v1 = *(size_t*) e1;
        size_t v2 = *(size_t*) e2;
        if (v1 == v2){
                return TRUE;
        }
        return FALSE;
}
