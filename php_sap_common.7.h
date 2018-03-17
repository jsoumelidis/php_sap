#ifndef PHP_SAP_COMMON_7_H

#define PHP_SAP_COMMON_7_H

#define sap_get_intern(_arg, type) (type*)((char *)Z_OBJ_P(_arg) - Z_OBJ_P(_arg)->handlers->offset)
#define sap_get_function(_arg) sap_get_intern(_arg, sap_function)
#define sap_get_sap_object(_arg) sap_get_intern(_arg, sap_object)

#define PHP_SAP_PARSE_PARAMS_BEGIN() do {
#define PHP_SAP_PARSE_PARAMS zend_parse_parameters_throw
#define PHP_SAP_PARSE_PARAMS_END() } while(0)

#define SAP_THROW_SAPRFC_ERROR_EXCEPTION(_err) {    \
    SAPRFC_ERROR_INFO *__err = (_err);              \
    zval _ex;                                       \
    sap_rfc_error_to_exception(__err, &_ex);        \
    zend_throw_exception_object(&_ex);              \
}

#define sap_throw_exception(_message, _code, _ce) do {                                                                          \
    const char *__message = (_message);                                                                                         \
    int __code = (_code);                                                                                                       \
    zend_class_entry *_ex_ce = (_ce);                                                                                           \
    zval _ex;                                                                                                                   \
    object_init_ex(&_ex, _ex_ce);                                                                                               \
    zend_update_property_stringl(zend_default_exception, &_ex, "message", sizeof("message")-1, __message, strlen(__message));   \
    zend_update_property_long(zend_default_exception, &_ex, "code", sizeof("code") - 1, __code);                                \
    zend_throw_exception_object(&_ex);                                                                                          \
} while(0)

#define MY_ZEND_HASH_FOREACH(ht) do {                       \
    HashTable *__ht = (ht);                                 \
    HashPosition __pos;                                     \
    for (zend_hash_internal_pointer_reset_ex(__ht, &__pos); \
         __pos != HT_INVALID_IDX;                           \
         zend_hash_move_forward_ex(__ht, &__pos)            \
    ) {                                                     \
        Bucket *_p = __ht->arData + __pos;                  \
        zval *_z = &_p->val;

#define MY_ZEND_HASH_FOREACH_VAL(ht, _zval) MY_ZEND_HASH_FOREACH(ht) _zval = _z;
#define MY_ZEND_HASH_FOREACH_KEY_VAL(ht, _h, _key, _keylen, _zval) MY_ZEND_HASH_FOREACH_VAL(ht, _zval) _h = _p->h; _key = _p->key ? _p->key->val : NULL; _keylen = _p->key ? _p->key->len : 0;
#define MY_ZEND_HASH_FOREACH_STR_KEY(ht, _key, _keylen) MY_ZEND_HASH_FOREACH(ht) _key = _p->key ? _p->key->val : NULL; _keylen = _p->key ? _p->key->len : 0;
#define MY_ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _zval) MY_ZEND_HASH_FOREACH_VAL(ht, _zval) _key = _p->key ? _p->key->val : NULL; _keylen = _p->key ? _p->key->len : 0;
#define MY_ZEND_HASH_FOREACH_STR_KEY_PTR(ht, _key, _keylen, _ptr) MY_ZEND_HASH_FOREACH_STR_KEY(ht, _key, _keylen) _ptr = Z_PTR_P(_z);

#define sap_read_object_property(_object, _prop, _scope) zend_read_property(_scope, _object, _prop, strlen(_prop) + 1, 0, NULL)
#define sap_read_object_property_ex(_object, _prop, _proplen, _scope) zend_read_property(_scope, _object, _prop, _proplen, 0, NULL)

#define sap_fetch_connection_rsrc(_object) zend_fetch_resource(Z_RES_P(_object), PHP_SAP_CONNECTION_RES_NAME, le_php_sap_connection)

#define sap_make_resource(_zval, _ptr, _rsrc_id) ZVAL_RES(_zval, zend_register_resource(_ptr, _rsrc_id))

#define sap_get_str_val(_zstr) (char*)ZSTR_VAL(_zstr)

#define my_zval_ptr_dtor(_pzval) zval_ptr_dtor(_pzval)


#define MY_ZEND_HASH_FOREACH_END()  \
    }                               \
} while(0)

#endif /* PHP_SAP_COMMON_7_H */