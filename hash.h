#ifndef PHP_SAP_HASH_H

#define PHP_SAP_HASH_H

#include "php.h"

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
