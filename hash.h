#ifndef PHP_SAP_HASH_H

#define PHP_SAP_HASH_H

#include "php.h"

#define ZEND_HASH_FOREACH(ht) do {                                                  \
    HashTable *__ht = (ht);                                                         \
    HashPosition __pos;                                                             \
    void **__data;                                                                  \
    for (zend_hash_internal_pointer_reset_ex(__ht, &__pos);                         \
        zend_hash_get_current_data_ex(__ht, (void**)&__data, &__pos) == SUCCESS;    \
        zend_hash_move_forward_ex(__ht, &__pos))                                    \
    {

#define ZEND_HASH_FOREACH_VAL(ht, _zval) ZEND_HASH_FOREACH(ht) _zval = *__data;
#define ZEND_HASH_FOREACH_KEY_VAL(ht, _h, _key, _keylen, _zval) ZEND_HASH_FOREACH_VAL(ht, _zval) _h = __pos->h; _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define ZEND_HASH_FOREACH_STR_KEY(ht, _key, _keylen) ZEND_HASH_FOREACH(ht) _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _zval) ZEND_HASH_FOREACH_VAL(ht, _zval) _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define ZEND_HASH_FOREACH_STR_KEY_PTR(ht, _key, _keylen, _ptr) ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _ptr);
#define ZEND_HASH_FOREACH_END() \
    }                           \
} while(0)

/* hash functions */
static void * my_zend_hash_find_ptr(HashTable *ht, char *key, int keylen)
{
    void **ptr_ptr;

    if (zend_hash_find(ht, key, keylen + 1, (void**)&ptr_ptr) == SUCCESS) {
        return *ptr_ptr;
    }

    return NULL;
}

static zval * my_zend_hash_find_zval(HashTable *ht, char *key, int keylen)
{
    zval **ptr_ptr;

    if (zend_hash_find(ht, key, keylen + 1, (void**)&ptr_ptr) == SUCCESS) {
        return *ptr_ptr;
    }

    return NULL;
}

static void * my_zend_hash_update_ptr(HashTable *ht, char *key, int keylen, void *ptr, size_t size)
{
    void **dest;

    if (SUCCESS == zend_hash_update(ht, key, keylen + 1, &ptr, size, (void**)&dest)) {
        return *dest;
    }

    return NULL;
}

static zval * my_zend_hash_update_zval(HashTable *ht, char *key, int keylen, zval *z)
{
    zval **dest;

    if (SUCCESS == zend_hash_update(ht, key, keylen + 1, &z, sizeof(zval*), (void**)&dest)) {
        return *dest;
    }

    return NULL;
}

static void * my_zend_hash_add_ptr(HashTable *ht, char *key, int keylen, void *ptr, size_t size)
{
    void **dest;

    if (SUCCESS == zend_hash_add(ht, key, keylen + 1, &ptr, size, (void**)&dest)) {
        return *dest;
    }

    return NULL;
}

static zval * my_zend_hash_add_zval(HashTable *ht, char *key, int keylen, zval *z)
{
    zval **dest;

    if (SUCCESS == zend_hash_add(ht, key, keylen + 1, &z, sizeof(zval*), (void**)&dest)) {
        return *dest;
    }

    return NULL;
}

static zval * my_zend_hash_add_new_zval(HashTable *ht, char *key, int keylen)
{
    zval *znewentry = NULL;

    MAKE_STD_ZVAL(znewentry);

    if (znewentry && !(znewentry = my_zend_hash_add_zval(ht, key, keylen, znewentry))) {
        zval_ptr_dtor(&znewentry);
    }

    return znewentry;
}

static void * my_zend_hash_next_index_insert_ptr(HashTable *ht, void *ptr, size_t size)
{
    void **dest;

    if (SUCCESS == zend_hash_next_index_insert(ht, &ptr, size, (void**)&dest)) {
        return *dest;
    }

    return NULL;
}

static zval * my_zend_hash_next_index_insert_zval(HashTable *ht, zval *z)
{
    zval **dest;

    if (SUCCESS == zend_hash_next_index_insert(ht, &z, sizeof(zval*), (void**)&dest)) {
        return *dest;
    }

    return NULL;
}

static zval * my_zend_hash_next_index_insert_new_zval(HashTable *ht)
{
    zval *znewentry = NULL;

    MAKE_STD_ZVAL(znewentry);

    if (znewentry && !(znewentry = my_zend_hash_next_index_insert_zval(ht, znewentry))) {
        zval_ptr_dtor(&znewentry);
    }

    return znewentry;
}

#endif /* PHP_SAP_HASH_H */
