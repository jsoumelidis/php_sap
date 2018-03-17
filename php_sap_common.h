#ifndef PHP_SAP_COMMON_H
#define PHP_SAP_COMMON_H

#define sap_get_intern(_arg, type) (type*)zend_object_store_get_object(_arg TSRMLS_CC)
#define sap_get_function(_arg) sap_get_intern(_arg, sap_function)
#define sap_get_sap_object(_arg) sap_get_intern(_arg, sap_object)

#define PHP_SAP_PARSE_PARAMS_BEGIN() do { zend_error_handling _eh; zend_replace_error_handling(EH_THROW, zend_invalid_args_exception, &_eh TSRMLS_CC);
#define PHP_SAP_PARSE_PARAMS zend_parse_parameters
#define PHP_SAP_PARSE_PARAMS_END() zend_restore_error_handling(&_eh TSRMLS_CC); } while(0)

#define SAP_THROW_SAPRFC_ERROR_EXCEPTION(_err) do {     \
    SAPRFC_ERROR_INFO *__err = (_err);                  \
    zval *_ex;                                          \
    MAKE_STD_ZVAL(_ex);                                 \
    sap_rfc_error_to_exception(__err, _ex TSRMLS_CC);   \
    zend_throw_exception_object(_ex TSRMLS_CC);         \
} while(0)

#define sap_throw_exception(_message, _code, _ce) do {                                                                                  \
    const char *__message = (_message);                                                                                                 \
    int __code = (_code);                                                                                                               \
    zend_class_entry *__ex_ce = (_ce);                                                                                                  \
    zval *_ex;                                                                                                                          \
    MAKE_STD_ZVAL(_ex);                                                                                                                 \
    object_init_ex(_ex, __ex_ce);                                                                                                       \
    zend_update_property_stringl(zend_default_exception,_ex, "message", sizeof("message")-1, __message, strlen(__message) TSRMLS_CC);   \
    zend_update_property_long(zend_default_exception, _ex, "code", sizeof("code") - 1, __code TSRMLS_CC);                               \
    zend_throw_exception_object(_ex TSRMLS_CC);                                                                                         \
} while(0)

#define MY_ZEND_HASH_FOREACH(ht) do {                                               \
    HashTable *__ht = (ht);                                                         \
    HashPosition __pos;                                                             \
    void **__data;                                                                  \
    for (zend_hash_internal_pointer_reset_ex(__ht, &__pos);                         \
        zend_hash_get_current_data_ex(__ht, (void**)&__data, &__pos) == SUCCESS;    \
        zend_hash_move_forward_ex(__ht, &__pos))                                    \
    {

#define MY_ZEND_HASH_FOREACH_VAL(ht, _zval) MY_ZEND_HASH_FOREACH(ht) _zval = *__data;
#define MY_ZEND_HASH_FOREACH_KEY_VAL(ht, _h, _key, _keylen, _zval) MY_ZEND_HASH_FOREACH_VAL(ht, _zval) _h = __pos->h; _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define MY_ZEND_HASH_FOREACH_STR_KEY(ht, _key, _keylen) MY_ZEND_HASH_FOREACH(ht) _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define MY_ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _zval) MY_ZEND_HASH_FOREACH_VAL(ht, _zval) _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define MY_ZEND_HASH_FOREACH_STR_KEY_PTR(ht, _key, _keylen, _ptr) MY_ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _ptr);

#define sap_read_object_property(_object, _prop, _scope) zend_read_property(_scope, _object, _prop, strlen(_prop) + 1, 0 TSRMLS_CC)
#define sap_read_object_property_ex(_object, _prop, _proplen, _scope) zend_read_property(_scope, _object, _prop, _proplen, 0 TSRMLS_CC)

#define sap_fetch_connection_rsrc(_object) zend_fetch_resource(&_object TSRMLS_CC, -1, PHP_SAP_CONNECTION_RES_NAME, NULL, 1, le_php_sap_connection)

#if PHP_VERSION_ID < 50400
#   define sap_make_resource(_zval, _ptr, _rsrc_id) zend_register_resource(_zval, _ptr, _rsrc_id)
#else
#   define sap_make_resource(_zval, _ptr, _rsrc_id) zend_register_resource(_zval, _ptr, _rsrc_id TSRMLS_CC)
#endif

#define sap_get_str_val(_str) (char*)(_str)

#define my_zval_ptr_dtor(_pzval) zval_ptr_dtor(&(_pzval))

#define MY_ZEND_HASH_FOREACH_END()  \
    }                               \
} while(0)

#endif /* PHP_SAP_COMMON_H */
