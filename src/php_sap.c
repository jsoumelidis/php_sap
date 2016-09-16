#include "php_sap.h"

#ifdef PHP_SAP_WITH_PTHREADS
#define PTW32_INCLUDE_WINDOWS_H
#ifdef TIME_H
#define HAVE_STRUCT_TIMESPEC
#endif //TIME_H

#include "pthread.h"

#endif //PHP_SAP_WITH_PTHREADS

#include "php_ini.h"
#include "ext\standard\info.h"

#include "zend_objects.h"
#include "zend_exceptions.h"

#ifdef HAVE_SPL
	#include "ext\spl\spl_exceptions.h"

	#define zend_invalid_args_exception spl_ce_InvalidArgumentException
#else
	#define zend_invalid_args_exception zend_default_exception
#endif

#ifdef HAVE_DATE
	#include "ext\date\php_date.h"
#endif

#define SAP_ME_ARGS(classname, method) arginfo_##classname##_##method
#define SAP_FE_ARGS(func) arginfo_func_##func

#define SAP_ERROR_SET_RFCFUNCTION(_err, _func, _funclen) do {	\
	SAPRFC_ERROR_INFO *__err = (_err);							\
	const char *__func = (_func);								\
	unsigned int __funclen = (_funclen);						\
	__err->l_nwsdkfunction = __funclen;							\
	memcpy(__err->nwsdkfunction, __func, __funclen);			\
	memset(__err->nwsdkfunction + __funclen, 0, 1);				\
} while (0);

#define SAP_ERROR_SET_FUNCTION_AND_RETURN(_err, _func, _retval) SAP_ERROR_SET_RFCFUNCTION(_err, _func, strlen(_func)); return _retval

#define SAPRFC_PARAMETER_PTR_DTOR sap_rfc_parameter_ptr_dtor

#define XtSizeOf(type, member) sizeof(((type *)0)->member)

#if PHP_VERSION_ID < 70000

#define sap_get_intern(_arg, type) (type*)zend_object_store_get_object(_arg TSRMLS_CC)
#define sap_get_function(_arg) sap_get_intern(_arg, sap_function)
#define sap_get_sap_object(_arg) sap_get_intern(_arg, sap_object)

#define PHP_SAP_PARSE_PARAMS_BEGIN() do { zend_error_handling _eh; zend_replace_error_handling(EH_THROW, zend_invalid_args_exception, &_eh TSRMLS_CC);
#define PHP_SAP_PARSE_PARAMS zend_parse_parameters
#define PHP_SAP_PARSE_PARAMS_END() zend_restore_error_handling(&_eh TSRMLS_CC); } while(0)

#define SAP_THROW_SAPRFC_ERROR_EXCEPTION(_err) {		\
	SAPRFC_ERROR_INFO *__err = (_err);					\
	zval *_ex;											\
	MAKE_STD_ZVAL(_ex);									\
	sap_rfc_error_to_exception(__err, _ex TSRMLS_CC);	\
	zend_throw_exception_object(_ex TSRMLS_CC);			\
}

#define sap_throw_exception(_message, _code, _ce) do {																					\
	const char *__message = (_message);																									\
	int __code = (_code);																												\
	zend_class_entry *__ex_ce = (_ce);																									\
	zval *_ex;																															\
	MAKE_STD_ZVAL(_ex);																													\
	object_init_ex(_ex, __ex_ce);																										\
	zend_update_property_stringl(zend_default_exception, _ex, "message", sizeof("message")-1, __message, strlen(__message) TSRMLS_CC);	\
	zend_update_property_long(zend_default_exception, _ex, "code", sizeof("code") - 1, __code TSRMLS_CC);								\
	zend_throw_exception_object(_ex TSRMLS_CC);																							\
} while(0)

#define MY_ZEND_HASH_FOREACH(ht) do {												\
	HashTable *__ht = (ht);															\
	HashPosition __pos;																\
	void **__data;																	\
	for (zend_hash_internal_pointer_reset_ex(__ht, &__pos);							\
		 zend_hash_get_current_data_ex(__ht, (void**)&__data, &__pos) == SUCCESS;	\
		 zend_hash_move_forward_ex(__ht, &__pos))									\
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
#define sap_make_resource(_zval, _ptr, _rsrc_id) zend_register_resource(_zval, _ptr, _rsrc_id)
#else
#define sap_make_resource(_zval, _ptr, _rsrc_id) zend_register_resource(_zval, _ptr, _rsrc_id TSRMLS_CC)
#endif

#define sap_get_str_val(_str) (char*)(_str)

#define my_zval_ptr_dtor(_pzval) zval_ptr_dtor(&(_pzval))

#else

#define sap_get_intern(_arg, type) (type*)((char *)Z_OBJ_P(_arg) - Z_OBJ_P(_arg)->handlers->offset)
#define sap_get_function(_arg) sap_get_intern(_arg, sap_function)
#define sap_get_sap_object(_arg) sap_get_intern(_arg, sap_object)

#define PHP_SAP_PARSE_PARAMS_BEGIN() do {
#define PHP_SAP_PARSE_PARAMS zend_parse_parameters_throw
#define PHP_SAP_PARSE_PARAMS_END() } while(0)

#define SAP_THROW_SAPRFC_ERROR_EXCEPTION(_err) {	\
	SAPRFC_ERROR_INFO *__err = (_err);				\
	zval _ex;										\
	sap_rfc_error_to_exception(__err, &_ex);		\
	zend_throw_exception_object(&_ex);				\
}

#define sap_throw_exception(_message, _code, _ce) do {																			\
	const char *__message = (_message);																							\
	int __code = (_code);																										\
	zend_class_entry *_ex_ce = (_ce);																							\
	zval _ex;																													\
	object_init_ex(&_ex, _ex_ce);																								\
	zend_update_property_stringl(zend_default_exception, &_ex, "message", sizeof("message")-1, __message, strlen(__message));	\
	zend_update_property_long(zend_default_exception, &_ex, "code", sizeof("code") - 1, __code);								\
	zend_throw_exception_object(&_ex);																							\
} while(0)

#define MY_ZEND_HASH_FOREACH(ht) do {						\
	HashTable *__ht = (ht);									\
	HashPosition __pos;										\
	zval *_z;												\
	for (zend_hash_internal_pointer_reset_ex(__ht, &__pos);	\
		 _z = zend_hash_get_current_data_ex(__ht, &__pos);	\
		 zend_hash_move_forward_ex(__ht, &__pos))			\
	{														\
		uint32_t _idx = __pos;								\
		Bucket *_p = __ht->arData + _idx;

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

#endif

#define MY_ZEND_HASH_FOREACH_END()	\
	}								\
} while(0)

typedef enum _TRIM_TYPE {
	TRIM_LEFT = 0x01,
	TRIM_RIGHT = 0x02,
	TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
} TRIM_TYPE;

typedef struct _SAPRFC_ERROR_INFO {
	RFC_ERROR_INFO err;
	char nwsdkfunction[30];
	unsigned int l_nwsdkfunction;
} SAPRFC_ERROR_INFO;

typedef enum _SAPRFC_PARAM_STATE {
	SAPRFC_PARAM_INACTIVE = 0,
	SAPRFC_PARAM_ACTIVE = 1,
	SAPRFC_PARAM_DEFAULT = 2
} SAPRFC_PARAM_STATE;

typedef struct _SAPRFC_PARAMETER_DESC {
	RFC_PARAMETER_DESC			param;
	SAPRFC_PARAM_STATE			state;
} SAPRFC_PARAMETER_DESC;

typedef struct _sap_function {
#if PHP_VERSION_ID < 70000
	zend_object			std;
#endif
	php_sap_connection	*connection;
	php_sap_function	*function_descr;
#if PHP_VERSION_ID >= 70000
	zend_object			std;
#endif
} sap_function;

typedef struct _sap_object {
#if PHP_VERSION_ID < 70000
	zend_object			std;
#endif
	php_sap_connection	*connection;
	zend_class_entry	*func_ce;
#if PHP_VERSION_ID >= 70000
	zend_object			std;
#endif
} sap_object;

ZEND_BEGIN_MODULE_GLOBALS(sap)
	int		rtrim_export_strings;
ZEND_END_MODULE_GLOBALS(sap)

ZEND_DECLARE_MODULE_GLOBALS(sap)

#ifdef ZTS
#define PHP_SAP_GLOBALS(v) TSRMG(sap_globals_id, zend_sap_globals*, v)
#else
#define PHP_SAP_GLOBALS(v) (sap_globals.v)
#endif

static ZEND_MODULE_GLOBALS_CTOR_D(sap)
{
	sap_globals->rtrim_export_strings = 1;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("sap.rtrim_export_strings", "On", PHP_INI_ALL, OnUpdateBool, rtrim_export_strings, zend_sap_globals, sap_globals)
PHP_INI_END()

SAPRFC_ERROR_INFO sap_last_error;

#if PHP_SAP_WITH_PTHREADS
pthread_mutex_t rfc_utf8_to_sapuc_mutex;
pthread_mutex_t rfc_sapuc_to_utf8_mutex;
#endif

/**		php_sap module procedural functions -- begin	**/
PHP_FUNCTION(sap_connect);

ZEND_BEGIN_ARG_INFO(SAP_FE_ARGS(sap_connect), 0)
	ZEND_ARG_INFO(0, logonParameters)
ZEND_END_ARG_INFO()

PHP_FUNCTION(sap_last_error);

PHP_FUNCTION(sap_invoke_function);

ZEND_BEGIN_ARG_INFO(SAP_FE_ARGS(sap_invoke_function), 0)
	ZEND_ARG_INFO(0, module)
	ZEND_ARG_INFO(0, connection)
	ZEND_ARG_INFO(0, imports)
ZEND_END_ARG_INFO()

zend_function_entry php_sap_module_function_entry[] = {
	PHP_FE(sap_connect,			SAP_FE_ARGS(sap_connect))
	PHP_FE(sap_last_error,		NULL)
	PHP_FE(sap_invoke_function,	SAP_FE_ARGS(sap_invoke_function))
	PHP_FE_END
};

/**		php_sap module procedural functions -- end		**/

zend_module_entry sap_module_entry = {
	STANDARD_MODULE_HEADER,
	"sap",
	php_sap_module_function_entry,
	PHP_MINIT(sap),
	PHP_MSHUTDOWN(sap),
	NULL, /* PHP_RINIT(php_sap), */
	NULL, /* PHP_RSHUTDOWN(php_sap), */
	PHP_MINFO(sap),
	PHP_SAP_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHP_SAP
ZEND_GET_MODULE(sap)
#endif

int le_php_sap_connection;

zend_class_entry * sap_ce_SapException;
zend_class_entry * sap_ce_Sap;
zend_class_entry * sap_ce_SapFunction;
zend_class_entry * sap_ce_SapRfcReadTable;

zend_class_entry * zend_default_exception;

zend_object_handlers sap_object_handlers;
zend_object_handlers sap_function_handlers;

/** Exporting functions - begin **/
SAP_PHP_API zend_class_entry * php_sap_get_sap_ce(void)
{
	return sap_ce_Sap;
}

SAP_PHP_API zend_object_handlers * php_sap_get_sap_handlers(void)
{
	return &sap_object_handlers;
}

SAP_PHP_API php_sap_connection * php_sap_get_connection(zval *zsap TSRMLS_DC)
{
	sap_object *intern = sap_get_sap_object(zsap);

	return intern->connection;
}

SAP_PHP_API zend_class_entry * php_sap_get_function_ce(void)
{
	return sap_ce_SapFunction;
}

SAP_PHP_API zend_object_handlers * php_sap_get_function_handlers(void)
{
	return &sap_function_handlers;
}

SAP_PHP_API php_sap_function * php_sap_get_function_descr(zval *zfunction TSRMLS_DC)
{
	sap_function *intern = sap_get_function(zfunction);

	return intern->function_descr;
}

SAP_PHP_API zend_class_entry * php_sap_get_exception_ce(void)
{
	return sap_ce_SapException;
}
/** Exporting functions - end **/

/**	Class methods definition -- begin	**/

/* {{{ */

/* SapException */
PHP_METHOD(SapException, getMessageKey);
PHP_METHOD(SapException, getMessageType);
PHP_METHOD(SapException, getMessageId);
PHP_METHOD(SapException, getMessageNumber);
PHP_METHOD(SapException, getMessageVar1);
PHP_METHOD(SapException, getMessageVar2);
PHP_METHOD(SapException, getMessageVar3);
PHP_METHOD(SapException, getMessageVar4);
PHP_METHOD(SapException, getNwSdkFunction);

const zend_function_entry sap_exception_fe[] = {
	PHP_ME(SapException,	getMessageKey,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageType,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageId,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageNumber,	NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageVar1,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageVar2,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageVar3,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getMessageVar4,		NULL,	ZEND_ACC_PUBLIC)
	PHP_ME(SapException,	getNwSdkFunction,	NULL,	ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* Sap */
PHP_METHOD(Sap, __construct);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(Sap, __construct), 0)
	ZEND_ARG_INFO(0, logonParameters)
ZEND_END_ARG_INFO()

PHP_METHOD(Sap, setFunctionClass);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(Sap, setFunctionClass), 0)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

PHP_METHOD(Sap, call);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(Sap, call), 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, imports)
ZEND_END_ARG_INFO()

PHP_METHOD(Sap, fetchFunction);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(Sap, fetchFunction), 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, moduleClass)
ZEND_END_ARG_INFO()

const zend_function_entry sap_fe_Sap[] = {
	PHP_ME(Sap,				__construct,			SAP_ME_ARGS(Sap, __construct),		ZEND_ACC_PUBLIC)
	PHP_ME(Sap,				setFunctionClass,		SAP_ME_ARGS(Sap, setFunctionClass),	ZEND_ACC_PUBLIC)
	PHP_ME(Sap,				call,					SAP_ME_ARGS(Sap, call),				ZEND_ACC_PUBLIC)
	PHP_ME(Sap,				fetchFunction,			SAP_ME_ARGS(Sap, fetchFunction),	ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* SapFunction */
PHP_METHOD(SapFunction, getName);

PHP_METHOD(SapFunction, setActive);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(SapFunction, setActive), 0)
	ZEND_ARG_INFO(0, param)
	ZEND_ARG_INFO(0, isActive)
ZEND_END_ARG_INFO()

PHP_METHOD(SapFunction, __invoke);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(SapFunction, __invoke), 0)
ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

PHP_METHOD(SapFunction, getParameters);

PHP_METHOD(SapFunction, getTypeName);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(SapFunction, getTypeName), 0)
	ZEND_ARG_INFO(0, param)
ZEND_END_ARG_INFO()

const zend_function_entry sap_fe_SapFunction[] = {
	PHP_ME(SapFunction,		__invoke,			SAP_ME_ARGS(SapFunction, __invoke),		ZEND_ACC_PUBLIC)
	PHP_ME(SapFunction,		getName,			NULL,									ZEND_ACC_PUBLIC)
	PHP_ME(SapFunction,		getParameters,		NULL,									ZEND_ACC_PUBLIC)
	PHP_ME(SapFunction,		setActive,			SAP_ME_ARGS(SapFunction, setActive),	ZEND_ACC_PUBLIC)
	PHP_ME(SapFunction,		getTypeName,		SAP_ME_ARGS(SapFunction, getTypeName),	ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* SapRfcReadTable */
PHP_METHOD(SapRfcReadTable, getName);

PHP_METHOD(SapRfcReadTable, select);

ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(SapRfcReadTable, select), 0)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, table)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, rowcount)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

const zend_function_entry sap_fe_SapRfcReadTable[] = {
	PHP_ME(SapRfcReadTable,	getName,		NULL,										ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(SapRfcReadTable,	select,			SAP_ME_ARGS(SapRfcReadTable, select),		ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/**	Class methods definition -- end		**/

/* }}} */

static int sap_import(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err TSRMLS_DC);

static int sap_export(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *rv, SAPRFC_ERROR_INFO *err TSRMLS_DC);

/* hash functions */
static void * my_zend_hash_find_ptr(HashTable *ht, char *key, int keylen)
{
#if PHP_VERSION_ID < 70000
	void **ptr_ptr;

	if (zend_hash_find(ht, key, keylen + 1, (void**)&ptr_ptr) == SUCCESS) {
		return *ptr_ptr;
	}

	return NULL;
#else
	return zend_hash_str_find_ptr(ht, key, keylen);
#endif
}

zval * my_zend_hash_find_zval(HashTable *ht, char *key, int keylen)
{
#if PHP_VERSION_ID < 70000
	zval **ptr_ptr;

	if (zend_hash_find(ht, key, keylen + 1, (void**)&ptr_ptr) == SUCCESS) {
		return *ptr_ptr;
	}

	return NULL;
#else
	return zend_hash_str_find(ht, key, keylen);
#endif
}

static void * my_zend_hash_update_ptr(HashTable *ht, char *key, int keylen, void *ptr, size_t size)
{
#if PHP_VERSION_ID < 70000
	void **dest;

	if (SUCCESS == zend_hash_update(ht, key, keylen + 1, &ptr, size, (void**)&dest)) {
		return *dest;
	}

	return NULL;
#else
	return zend_hash_str_update_ptr(ht, key, keylen, ptr);
#endif
}

static zval * my_zend_hash_update_zval(HashTable *ht, char *key, int keylen, zval *z)
{
#if PHP_VERSION_ID < 70000
	zval **dest;

	if (SUCCESS == zend_hash_update(ht, key, keylen + 1, &z, sizeof(zval*), (void**)&dest)) {
		return *dest;
	}

	return NULL;
#else
	return zend_hash_str_update(ht, key, keylen, z);
#endif
}

static void * my_zend_hash_add_ptr(HashTable *ht, char *key, int keylen, void *ptr, size_t size)
{
#if PHP_VERSION_ID < 70000
	void **dest;

	if (SUCCESS == zend_hash_add(ht, key, keylen + 1, &ptr, size, (void**)&dest)) {
		return *dest;
	}

	return NULL;
#else
	return zend_hash_str_add_ptr(ht, key, keylen, ptr);
#endif
}

static zval * my_zend_hash_add_zval(HashTable *ht, char *key, int keylen, zval *z)
{
#if PHP_VERSION_ID < 70000
	zval **dest;

	if (SUCCESS == zend_hash_add(ht, key, keylen + 1, &z, sizeof(zval*), (void**)&dest)) {
		return *dest;
	}

	return NULL;
#else
	return zend_hash_str_add(ht, key, keylen, z);
#endif
}

static zval * my_zend_hash_add_new_zval(HashTable *ht, char *key, int keylen)
{
#if PHP_VERSION_ID < 70000
	zval *znewentry = NULL;

	MAKE_STD_ZVAL(znewentry);

	if (znewentry && !(znewentry = my_zend_hash_add_zval(ht, key, keylen, znewentry))) {
		zval_ptr_dtor(&znewentry);
	}

	return znewentry;
#else
	return zend_hash_str_add(ht, key, keylen, &EG(uninitialized_zval));
#endif
}

static void * my_zend_hash_next_index_insert_ptr(HashTable *ht, void *ptr, size_t size)
{
#if PHP_VERSION_ID < 70000
	void **dest;

	if (SUCCESS == zend_hash_next_index_insert(ht, &ptr, size, (void**)&dest)) {
		return *dest;
	}

	return NULL;
#else
	return zend_hash_next_index_insert_ptr(ht, ptr);
#endif
}

static zval * my_zend_hash_next_index_insert_zval(HashTable *ht, zval *z)
{
#if PHP_VERSION_ID < 70000
	zval **dest;

	if (SUCCESS == zend_hash_next_index_insert(ht, &z, sizeof(zval*), (void**)&dest)) {
		return *dest;
	}

	return NULL;
#else
	return zend_hash_next_index_insert(ht, z);
#endif
}

static zval * my_zend_hash_next_index_insert_new_zval(HashTable *ht)
{
#if PHP_VERSION_ID < 70000
	zval *znewentry = NULL;

	MAKE_STD_ZVAL(znewentry);

	if (znewentry && !(znewentry = my_zend_hash_next_index_insert_zval(ht, znewentry))) {
		zval_ptr_dtor(&znewentry);
	}

	return znewentry;
#else
	return zend_hash_next_index_insert(ht, &EG(uninitialized_zval));
#endif
}

/* Common functions */

static unsigned int strlenU8(char *str, unsigned int str_len)
{
	unsigned int len = 0, pos = 0;

	for (pos = 0; pos < str_len && str[pos]; pos++)
	{
		if ((str[pos] & 0xc0) != 0x80) len++;
	}

	return len;
}

void utf8_trim(char *str, int len, char **out, int *outlen, TRIM_TYPE trimtype)
{
	char *s = str, *e = str + len;

	/* left trim */
	if (trimtype & TRIM_LEFT) {
		while (isspace(*s)) {
			len--; s++;
		}
	}

	/* right trim */
	if (trimtype & TRIM_RIGHT) {
		while (--e >= s)
		{
			int u8_char_len;

			/* Calculate utf8 character length backwards */
			for (u8_char_len = 1; (*e & 0xc0) == 0x80 && e > s; u8_char_len++, e--);

			if (!isspace(*e)) break;

			len = len - u8_char_len;
		}
	}

	*out = estrndup(s, len);
	*outlen = len;
}

static int utf8_substr(char *str, int len, unsigned int start, unsigned int length, char **substr, int *substr_len)
{
	int pos = 0;
	unsigned int u8pos = 0;
	unsigned int sub_len = 0, sub_lenu8 = 0;
	int substr_start_pos = -1;
	
	for (pos = 0; pos < len && str[pos]; pos++)
	{
		if ((str[pos] & 0xc0) != 0x80)
		{
			if (sub_lenu8 == length) break;

			if (u8pos == start) substr_start_pos = pos;

			if (u8pos >= start) sub_lenu8++;

			u8pos++;
		}

		if (u8pos > start) sub_len++;
	}

	if (substr_start_pos < 0) { /* substring not in range of str */
		return FAILURE;
	}

	*substr = estrndup(str + substr_start_pos, sub_len);
	*substr_len = sub_len;

	return SUCCESS;
}

static int utf8_to_sapuc_l(char *str, int len, SAP_UC **uc, unsigned int *uc_len, SAPRFC_ERROR_INFO *err)
{
	SAP_UC *retval = NULL;
	unsigned int sapuc_num_chars = 0;
	RFC_RC res;
	
try_again:
	
#if PHP_SAP_WITH_PTHREADS
	/* Avoid concurrent access to the RfcSAPUCToUTF8 function */
	pthread_mutex_lock(&rfc_utf8_to_sapuc_mutex);
#endif

	res = RfcUTF8ToSAPUC((RFC_BYTE*)str, len, retval, &sapuc_num_chars, uc_len, (RFC_ERROR_INFO*)err);

#if PHP_SAP_WITH_PTHREADS
	pthread_mutex_unlock(&rfc_utf8_to_sapuc_mutex);
#endif

	switch (res)
	{
		/**
		* In case of RFC_BUFFER_TOO_SMALL the NW library fills 'sapuc_num_chars' with the required number
		* of SAP_UC characters (including the last null terminating SAP_UC char)
		*/
		case RFC_BUFFER_TOO_SMALL:
		{
			size_t uc_size = (sapuc_num_chars) * sizeof(SAP_UC);
			
			if (NULL != retval) {
				retval = erealloc(retval, uc_size);
			} else {
				retval = emalloc(uc_size);
			}

			memset(retval, 0, uc_size);

			goto try_again;
		}
		case RFC_OK:
			if (NULL == retval) {
				/**
				 * retval will be NULL if str == "", so we have to allocate an empty SAP_UC string
				 */
				retval = mallocU(0);
			}

			*uc = retval;

			return SUCCESS;
		default:
			if (NULL != retval) {
				efree(retval);
			}

			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcUTF8ToSAPUC", FAILURE);
	}
}

static int utf8_to_sapuc(char *str, SAP_UC **uc, unsigned int *uc_len, SAPRFC_ERROR_INFO *err)
{
	int strl = strlen(str);

	return utf8_to_sapuc_l(str, strl, uc, uc_len, err);
}

static int sapuc_to_utf8_l(SAP_UC *strU16, unsigned int strU16len, char **strU8, int *strU8len, SAPRFC_ERROR_INFO *err)
{
	char *utf8 = NULL;
	unsigned int utf8len = 0;
	RFC_RC res;
	
try_again:

#if PHP_SAP_WITH_PTHREADS
	/* Avoid concurrent access to the RfcSAPUCToUTF8 function */
	pthread_mutex_lock(&rfc_sapuc_to_utf8_mutex);
#endif

	res = RfcSAPUCToUTF8(strU16, strU16len, (RFC_BYTE*)utf8, &utf8len, strU8len, (RFC_ERROR_INFO*)err);

#if PHP_SAP_WITH_PTHREADS
	pthread_mutex_unlock(&rfc_sapuc_to_utf8_mutex);
#endif

	switch (res)
	{
		case RFC_BUFFER_TOO_SMALL: {
			/**
			 * In case of RFC_BUFFER_TOO_SMALL, nw sdk fills 'utf8len' with the required number
			 * of bytes for convertion (including the trailing null byte)
			 */
			if (NULL != utf8) {
				utf8 = erealloc(utf8, utf8len);
			} else {
				utf8 = emalloc(utf8len);
			}
			
			memset(utf8, 0, utf8len);

			goto try_again;
		}
		case RFC_OK: {

			if (NULL == utf8) {
				/**
				* utf8 will be NULL if str == "", so we have to allocate an empty string
				*/
				utf8 = estrndup("", 0);
			}

			*strU8 = utf8;

			return SUCCESS;
		}
		default: {
			if (NULL != utf8) {
				efree(utf8);
			}
			
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSAPUCToUTF8", FAILURE);
		}
	}
}

static int sapuc_to_utf8(SAP_UC *strU16, char **strU8, int *strU8len, SAPRFC_ERROR_INFO *err)
{
	return sapuc_to_utf8_l(strU16, strlenU(strU16), strU8, strU8len, err);
}

#if PHP_VERSION_ID >= 70000
static int sap_call_object_method(zval *object, zend_class_entry *scope_ce, const char *func, zend_function *fn_proxy, zval *args, zval *rv TSRMLS_DC)
#else
static int sap_call_object_method(zval *object, zend_class_entry *scope_ce, const char *func, zend_function *fn_proxy, zval *args, zval **rv_ptr_ptr TSRMLS_DC)
#endif
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	int retval;
	zval function_name;
	
	/* Setup function call info */
	fci.symbol_table = NULL;
	fci.function_table = &scope_ce->function_table;
	fci.no_separation = 1;
	fci.param_count = 0;
	fci.params = NULL;
	fci.size = sizeof(zend_fcall_info);

#if PHP_VERSION_ID >= 70000
	fci.retval = rv;

	ZVAL_STRING(&function_name, func);
	fci.function_name = function_name;

	if (!fn_proxy && !(fn_proxy = zend_hash_find_ptr(fci.function_table, Z_STR(fci.function_name)))) {
#else
	fci.retval_ptr_ptr = rv_ptr_ptr;

	ZVAL_STRING(&function_name, func, 1);
	fci.function_name = &function_name;

	/*
	 * For some reason, pointers are stored differently in zend_class_entry->function_table so the last argument is not void*** but void**
	 * Got that from zend std object handlers -> get_closure
	 */
	if (!fn_proxy && SUCCESS != zend_hash_find(fci.function_table, Z_STRVAL(function_name), Z_STRLEN(function_name) + 1, (void**)&fn_proxy)) {
#endif
		zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method %s::%s", sap_get_str_val(scope_ce->name), func);
	}

	/* Set function call arguments */
	zend_fcall_info_args(&fci, args TSRMLS_CC);

	/* Setup function call cache */
#if PHP_VERSION_ID < 70000
	fcc.object_ptr = object;
#else
	fcc.object = fci.object = Z_OBJ_P(object);
#endif
	fcc.initialized = 1;
	fcc.function_handler = fn_proxy;
	fcc.calling_scope = scope_ce;
	fcc.called_scope = scope_ce;

	/* Do call */
	retval = zend_call_function(&fci, &fcc TSRMLS_CC);

	zval_dtor(&function_name);

	return retval;
}

int format_datetime_object(zval *object, zval *return_value, const char *format TSRMLS_DC)
{
	zval __args;
	zval *zformat_argument;
	int call_result;
#if PHP_VERSION_ID < 70000
	zval *return_value_ptr = NULL;
#endif
	
	array_init(&__args);

	zformat_argument = my_zend_hash_next_index_insert_new_zval(Z_ARRVAL(__args));

	if (NULL == zformat_argument) {
		zval_dtor(&__args);
		return FAILURE;
	}

#if PHP_VERSION_ID < 70000
	ZVAL_STRING(zformat_argument, format, 1);

	call_result = sap_call_object_method(object, Z_OBJCE_P(object), "format", NULL, &__args, &return_value_ptr TSRMLS_CC);

	if (call_result == SUCCESS) {
		ZVAL_ZVAL(return_value, return_value_ptr, 1, 1);
	}
#else
	ZVAL_STRING(zformat_argument, format);

	call_result = sap_call_object_method(object, Z_OBJCE_P(object), "format", NULL, &__args, return_value);
#endif

	zval_dtor(&__args);

	return call_result;
}

static zval php_sap_create_exception(zend_class_entry *ce, int code, char *format, ...)
{
	va_list args;
	char *message;
	zval ex;

	va_start(args, format);
	vspprintf(&message, 0, format, args);
	va_end(args);

	if (NULL == ce) {
		ce = zend_default_exception;
	}

	object_init_ex(&ex, ce);

	zend_update_property_string(zend_default_exception, &ex, "message", sizeof("message") - 1, message);
	zend_update_property_long(zend_default_exception, &ex, "code", sizeof("code") - 1, code);

	return ex;
}

static void php_sap_error(SAPRFC_ERROR_INFO *err, const char *key, int code, char *format, ...)
{
	va_list args;
	char *message;
	unsigned int messageUlen = 512, keyUlen = 128;
	RFC_ERROR_INFO e;

	/* Clear error */
	memset(err, 0, sizeof(SAPRFC_ERROR_INFO));

	va_start(args, format);
	vspprintf(&message, 0, format, args);
	va_end(args);
	
	RfcUTF8ToSAPUC(message, strlen(message), err->err.message, &messageUlen, &messageUlen, &e);

	RfcUTF8ToSAPUC(key, strlen(key), err->err.key, &keyUlen, &keyUlen, &e);

	err->err.code = code;
}

static void sap_rfc_error_to_exception(SAPRFC_ERROR_INFO *err, zval *exception TSRMLS_DC)
{
	char *key, *msgType, *msgId, *msgNumber, *msgv1, *msgv2, *msgv3, *msgv4, *message;
	int keyLen, msgTypeLen, msgIdLen, msgNumberLen, msgv1Len, msgv2Len, msgv3Len, msgv4Len, messageLen;
	SAPRFC_ERROR_INFO e;

	object_init_ex(exception, sap_ce_SapException);

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgType, &msgType, &msgTypeLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGTY", sizeof("MSGTY") - 1, msgType, msgTypeLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgClass, &msgId, &msgIdLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGID", sizeof("MSGID") - 1, msgId, msgIdLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgNumber, &msgNumber, &msgNumberLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGNO", sizeof("MSGNO") - 1, msgNumber, msgNumberLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.message, &message, &messageLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "message", sizeof("message") - 1, message, messageLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV1, &msgv1, &msgv1Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGV1", sizeof("MSGV1") - 1, msgv1, msgv1Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV2, &msgv2, &msgv2Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGV2", sizeof("MSGV2") - 1, msgv2, msgv2Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV3, &msgv3, &msgv3Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGV3", sizeof("MSGV3") - 1, msgv3, msgv3Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV4, &msgv4, &msgv4Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "MSGV4", sizeof("MSGV4") - 1, msgv4, msgv4Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.key, &key, &keyLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "KEY", sizeof("KEY") - 1, key, keyLen TSRMLS_CC);
	}

	if (err->nwsdkfunction > 0) {
		zend_update_property_stringl(sap_ce_SapException, exception, "nwsdkfunction", sizeof("nwsdkfunction") - 1, err->nwsdkfunction, err->l_nwsdkfunction TSRMLS_CC);
	}

	zend_update_property_long(sap_ce_SapException, exception, "code", sizeof("code") - 1, err->err.code TSRMLS_CC);
}

#if PHP_VERSION_ID < 70000
static void sap_rfc_parameter_ptr_dtor(SAPRFC_PARAMETER_DESC **param)
{
	SAPRFC_PARAMETER_DESC *p = *param;
#else
static void sap_rfc_parameter_ptr_dtor(zval *zparam)
{
	SAPRFC_PARAMETER_DESC *p = Z_PTR_P(zparam);
#endif
	efree(p);
}

static HashTable * sap_function_description_to_array(RFC_FUNCTION_DESC_HANDLE fdh, SAPRFC_ERROR_INFO *err)
{
	unsigned int paramCount, i;
	HashTable *retval;
	SAPRFC_PARAMETER_DESC *sp;

	if (RFC_OK != RfcGetParameterCount(fdh, &paramCount, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetParameterCount", NULL);
	}

	/**
	 * Allocate parameters' hashtable
	 */
	ALLOC_HASHTABLE(retval);
	zend_hash_init(retval, paramCount, NULL, SAPRFC_PARAMETER_PTR_DTOR, 0);

	for (i = 0; i < paramCount; i++)
	{
		char *paramName; int paramNameLen;

		/* Allocate memory for current parameter */
		sp = emalloc(sizeof(SAPRFC_PARAMETER_DESC));
		memset(sp, 0, sizeof(SAPRFC_PARAMETER_DESC));

		if (RFC_OK != RfcGetParameterDescByIndex(fdh, i, &sp->param, (RFC_ERROR_INFO*)err)) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetParameterDescByIndex", NULL);
		}

		/* Set default parameter's state */
		sp->state = SAPRFC_PARAM_DEFAULT;

		/* The key of the hashtable's new entry will be the utf8-converted parameter's name */
		if (SUCCESS != sapuc_to_utf8(sp->param.name, &paramName, &paramNameLen, err)) {
			return NULL;
		}

		/* Append parameter to parameters' hashtable */
		my_zend_hash_add_ptr(retval, paramName, paramNameLen, sp, sizeof(SAPRFC_PARAMETER_DESC*));

		efree(paramName);
	}

	return retval;
}

static php_sap_connection * sap_create_connection_resource(HashTable *lparams)
{
	php_sap_connection *retval;

	retval = emalloc(sizeof(php_sap_connection));
	memset(retval, 0, sizeof(php_sap_connection));

	ALLOC_HASHTABLE(retval->lparams);
	zend_hash_init(retval->lparams, 0, NULL, ZVAL_PTR_DTOR, 0);

	if (NULL != lparams)
	{
		char *lparam;
		int lparamlen;
		zval *zlpvalue;

		MY_ZEND_HASH_FOREACH_STR_KEY_VAL(lparams, lparam, lparamlen, zlpvalue)
		{
			zval *pcopy;
			zval *z;
#if PHP_VERSION_ID >= 70000
			zval zcopy;

			pcopy = &zcopy;
#endif

			/* We are only interested on parameters with a string key */
			if (NULL == lparam) {
				continue;
			}

			/* param value must be string because it has to be converted later to a UTF-16 string */
			switch (Z_TYPE_P(zlpvalue))
			{
				case IS_STRING: break;
				case IS_LONG:
				case IS_DOUBLE:
				case IS_NULL:
				{
#if PHP_VERSION_ID < 70000
					MAKE_STD_ZVAL(pcopy);
#endif
					ZVAL_ZVAL(pcopy, zlpvalue, 1, 0);
					convert_to_string(pcopy);
					zlpvalue = pcopy;
				}
				default: continue; /* Ignore parameters with unsupported values */

			}

			if (z = my_zend_hash_add_zval(retval->lparams, lparam, lparamlen, zlpvalue)) {
				Z_ADDREF_P(z);
			}

			if ( zlpvalue == pcopy ) {
				my_zval_ptr_dtor(pcopy);
			}
		}
		MY_ZEND_HASH_FOREACH_END();
	}

	return retval;
}

static void php_sap_connection_ptr_dtor(php_sap_connection *connection)
{
	if (--connection->refCount == 0)
	{
		RFC_ERROR_INFO e;

		if (NULL != connection->handle) {
			RfcCloseConnection(connection->handle, &e);
		}

		zend_hash_destroy(connection->lparams);
		FREE_HASHTABLE(connection->lparams);

		memset(connection, 0, sizeof(php_sap_connection));

		efree(connection);
	}
}

static ZEND_RSRC_DTOR_FUNC(php_sap_connection_rsrc_dtor)
{
#if PHP_VERSION_ID < 70000
	php_sap_connection *connection = rsrc->ptr;
#else
	php_sap_connection *connection = res->ptr;
#endif

	php_sap_connection_ptr_dtor(connection);
}

static php_sap_function * sap_create_function_from_descr_handle(RFC_FUNCTION_DESC_HANDLE fdh, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	php_sap_function *retval;

	retval = emalloc(sizeof(php_sap_function));
	memset(retval, 0, sizeof(php_sap_function));
	
	retval->fdh = fdh;
	retval->params = sap_function_description_to_array(fdh, err);

	if (NULL == retval->params) {
		efree(retval);
		return NULL;
	}

	return retval;
}

static void php_sap_function_ptr_dtor(php_sap_function *func)
{
	zend_hash_destroy(func->params);
	FREE_HASHTABLE(func->params);
	
	efree(func);
}

static ZEND_RSRC_DTOR_FUNC(php_sap_function_rsrc_dtor)
{
#if PHP_VERSION_ID < 70000
	php_sap_function *func = rsrc->ptr;
#else
	php_sap_function *func = res->ptr;
#endif

	php_sap_function_ptr_dtor(func);
}

#if PHP_VERSION_ID < 70000
static void sap_object_free_object_storage(void *object TSRMLS_DC)
{
	sap_object *intern = (sap_object*)object;
#else
static void sap_object_free_object_storage(zend_object *object)
{
	sap_object *intern = (sap_object*)((char*)(object) - object->handlers->offset);
#endif
	
	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->connection) {
		php_sap_connection_ptr_dtor(intern->connection);
		intern->connection = NULL;
	}

#if PHP_VERSION_ID < 70000
	efree(intern); /* custom object memory is freed by php on versions >= 7.0.0 */
#endif
}

#if PHP_VERSION_ID < 70000
static zend_object_value sap_object_create_object_ex(zend_class_entry *ce, sap_object **sap TSRMLS_DC)
{
	zend_object_value retval;
	size_t sz = sizeof(sap_object);
#else
static zend_object * sap_object_create_object_ex(zend_class_entry *ce, sap_object **sap TSRMLS_DC)
{
	zend_object *retval;
	size_t sz = sizeof(sap_object) + zend_object_properties_size(ce);
#endif
	sap_object *intern;

	intern = *sap = emalloc(sz);
	memset(intern, 0, sz);

	intern->func_ce = sap_ce_SapFunction;

	zend_object_std_init(&intern->std, ce TSRMLS_CC);
#if PHP_VERSION_ID >= 50400
	object_properties_init(&intern->std, ce);
#else
	{
		zval tmp;
		zend_hash_copy(intern->std.properties, &ce->default_properties, (copy_ctor_func_t)zval_property_ctor, (void *)&tmp, sizeof(zval *));
	}
#endif

#if PHP_VERSION_ID >= 70000
	intern->std.handlers = &sap_object_handlers;

	retval = &intern->std;
#else
	retval.handle = zend_objects_store_put(intern, NULL, sap_object_free_object_storage, NULL TSRMLS_CC);
	retval.handlers = &sap_object_handlers;
#endif

	return retval;
}

#if PHP_VERSION_ID < 70000
static zend_object_value sap_object_create_object(zend_class_entry *ce TSRMLS_DC)
#else
static zend_object * sap_object_create_object(zend_class_entry *ce TSRMLS_DC)
#endif
{
	sap_object *tmp;

	return sap_object_create_object_ex(ce, &tmp TSRMLS_CC);
}

#if PHP_VERSION_ID >= 70000
static zend_object * sap_object_clone_object(zval *object)
{
	zend_object *retval;
#else
static zend_object_value sap_object_clone_object(zval *object TSRMLS_DC)
{
	zend_object_value retval;
	zval tmp;
#endif
	sap_object *clone_intern, *intern = sap_get_sap_object(object);

	retval = sap_object_create_object_ex(Z_OBJCE_P(object), &clone_intern TSRMLS_CC);

	if (intern->connection) {
		clone_intern->connection = intern->connection;
		clone_intern->connection->refCount++;
	}

	/* Clone properties */
#if PHP_VERSION_ID < 70000
	zend_objects_clone_members(&clone_intern->std, retval, &intern->std, Z_OBJVAL_P(object).handle TSRMLS_CC);
#else
	zend_objects_clone_members(&clone_intern->std, &intern->std);
#endif

	return retval;
}

#if PHP_VERSION_ID < 70000
static void sap_function_free_object_storage(void *object TSRMLS_DC)
{
	sap_function *intern = (sap_function*)object;
#else
static void sap_function_free_object_storage(zend_object *object)
{
	sap_function *intern = (sap_function*)((char*)(object) - object->handlers->offset);
#endif

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->function_descr) {
		php_sap_function_ptr_dtor(intern->function_descr);
	}

	if (intern->connection) {
		php_sap_connection_ptr_dtor(intern->connection);
	}

#if PHP_VERSION_ID < 70000
	efree(intern); /* custom object memory is freed by php on versions >= 7.0.0 */
#endif
}

#if PHP_VERSION_ID < 70000
static zend_object_value sap_function_create_object_ex(zend_class_entry *ce, sap_function **func TSRMLS_DC)
{
	zend_object_value retval;
	size_t sz = sizeof(sap_function);
#else
static zend_object * sap_function_create_object_ex(zend_class_entry *ce, sap_function **func)
{
	zend_object *retval;
	size_t sz = sizeof(sap_function) + zend_object_properties_size(ce);
#endif
	sap_function *intern;
	
	intern = *func = emalloc(sz);
	memset(intern, 0, sz);

	zend_object_std_init(&intern->std, ce TSRMLS_CC);
#if PHP_VERSION_ID >= 50400
	object_properties_init(&intern->std, ce);
#else
	{
		zval tmp;
		zend_hash_copy(intern->std.properties, &ce->default_properties, (copy_ctor_func_t)zval_property_ctor, (void *)&tmp, sizeof(zval *));
	}
#endif

#if PHP_VERSION_ID >= 70000
	intern->std.handlers = &sap_function_handlers;

	retval = &intern->std;
#else
	retval.handle = zend_objects_store_put(intern, NULL, sap_function_free_object_storage, NULL TSRMLS_CC);
	retval.handlers = &sap_function_handlers;
#endif

	return retval;
}

#if PHP_VERSION_ID < 70000
static zend_object_value sap_function_create_object(zend_class_entry *ce TSRMLS_DC)
#else
static zend_object * sap_function_create_object(zend_class_entry *ce TSRMLS_DC)
#endif
{
	sap_function *tmp;

	return sap_function_create_object_ex(ce, &tmp TSRMLS_CC);
}

#if PHP_VERSION_ID >= 70000
static zend_object * sap_function_clone_object(zval *object)
{
	zend_object *retval;
#else
static zend_object_value sap_function_clone_object(zval *object TSRMLS_DC)
{
	zend_object_value retval;
#endif
	sap_function *clone_intern, *intern = sap_get_function(object);
	SAPRFC_ERROR_INFO error;

	retval = sap_function_create_object_ex(Z_OBJCE_P(object), &clone_intern TSRMLS_CC);

	/* Clone Function Parameters */
	if (intern->function_descr)
	{
		clone_intern->function_descr = sap_create_function_from_descr_handle(intern->function_descr->fdh, &error TSRMLS_CC);

		if (NULL == clone_intern->function_descr) {
			zend_error(E_ERROR, "Could not clone function");
		}
	}

	if (intern->connection) {
		clone_intern->connection = intern->connection;
		clone_intern->connection->refCount++;
	}

	/* Clone properties */
#if PHP_VERSION_ID < 70000
	zend_objects_clone_members(&clone_intern->std, retval, &intern->std, Z_OBJVAL_P(object).handle TSRMLS_CC);
#else
	zend_objects_clone_members(&clone_intern->std, &intern->std);
#endif

	return retval;
}

static int sap_function_cast_object(zval *readobj, zval *retval, int type TSRMLS_DC)
{
	zend_object_handlers *std_object_handlers;
#if PHP_VERSION_ID >= 70000
	zval zname, *zname_ptr = &zname;
#else
	zval *zname_ptr;
#endif

	switch (type)
	{
		case IS_STRING:
		{
#if PHP_VERSION_ID >= 70000
			if (SUCCESS == sap_call_object_method(readobj, Z_OBJCE_P(readobj), "getname", NULL, NULL, zname_ptr))
			{
				if (readobj == retval) {
					zval_ptr_dtor(readobj);
				}

				
#else
			if (SUCCESS == sap_call_object_method(readobj, Z_OBJCE_P(readobj), "getname", NULL, NULL, &zname_ptr TSRMLS_CC))
			{
				if (readobj == retval) {
					zval_ptr_dtor(&readobj);
				}
#endif
				ZVAL_COPY_VALUE(retval, zname_ptr);

				return SUCCESS;
			}
			else if (EG(exception)) {
				return FAILURE;
			}

			break;
		}

		default: break;
	}

	std_object_handlers = zend_get_std_object_handlers();

	if (std_object_handlers->cast_object) {
		return std_object_handlers->cast_object(readobj, retval, type TSRMLS_CC);
	}
}

static int sap_connection_open(php_sap_connection *connection, SAPRFC_ERROR_INFO *err)
{
	RFC_CONNECTION_PARAMETER *params, *p;
	unsigned int paramCount, i;
	unsigned int l;

	paramCount = zend_hash_num_elements(connection->lparams);

	if (0 == paramCount) {
		php_sap_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Invalid logon parameters");
		return FAILURE;
	}

	params = p = emalloc(paramCount * sizeof(RFC_CONNECTION_PARAMETER));
	memset(params, 0, paramCount * sizeof(RFC_CONNECTION_PARAMETER));

	do {
		char *lparam;
		unsigned int lparamlen;
		zval *lpvalue;

		MY_ZEND_HASH_FOREACH_STR_KEY_VAL(connection->lparams, lparam, lparamlen, lpvalue)
		{
			if (NULL == lparam || Z_TYPE_P(lpvalue) != IS_STRING) {
				continue;
			}

			if (SUCCESS != utf8_to_sapuc_l(Z_STRVAL_P(lpvalue), Z_STRLEN_P(lpvalue), (SAP_UC**)&p->value, &l, err)) {
				return FAILURE;
			}

			if (SUCCESS != utf8_to_sapuc_l(lparam, lparamlen, (SAP_UC**)&p->name, &l, err)) {
				return FAILURE;
			}

			p++;
		}
		MY_ZEND_HASH_FOREACH_END();
	} while (0);

	connection->handle = RfcOpenConnection(params, paramCount, (RFC_ERROR_INFO*)err);

	for (i = 0, p = params; i < paramCount; i++, p++)
	{
		efree((void*)p->name);
		efree((void*)p->value);
	}

	efree(params);

	if (NULL == connection->handle) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcOpenConnection", FAILURE);
	}

	return SUCCESS;
}

static int sap_import_scalar(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	switch (type)
	{
		case RFCTYPE_INT:
		case RFCTYPE_INT1:
		case RFCTYPE_INT2:
		case RFCTYPE_INT8:
		{
			zval copy;

			if (Z_TYPE_P(zvalue) != IS_LONG)
			{
				ZVAL_ZVAL(&copy, zvalue, 1, 0);
				convert_to_long(&copy);
				zvalue = &copy;
			}

			if (Z_TYPE_P(zvalue) != IS_LONG)
			{
				SAP_UC *scalarTypeU16 = (SAP_UC*)RfcGetTypeAsString(type);
				char *scalarTypeU8;
				int scalarTypeU8Len;

				if (SUCCESS != sapuc_to_utf8(scalarTypeU16, &scalarTypeU8, &scalarTypeU8Len, err)) {
					return FAILURE;
				}
				
				php_sap_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for type %s must be integer (%s given)", scalarTypeU8, zend_get_type_by_const(Z_TYPE_P(zvalue)));

				efree(scalarTypeU8);

				return FAILURE;
			}

			if (RFC_OK != RfcSetInt(dh, name, Z_LVAL_P(zvalue), (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetInt", FAILURE);
			}

			if (UNEXPECTED(zvalue == &copy)) {
				zval_dtor(&copy); /* Do we really need this? */
			}

			break;
		}
		case RFCTYPE_BCD:
		case RFCTYPE_DECF16:
		case RFCTYPE_DECF34:
		case RFCTYPE_FLOAT:
		{
			zval copy;

			if (Z_TYPE_P(zvalue) != IS_DOUBLE)
			{
				ZVAL_ZVAL(&copy, zvalue, 1, 0);
				convert_to_double(&copy);
				zvalue = &copy;
			}

			if (Z_TYPE_P(zvalue) != IS_DOUBLE)
			{
				SAP_UC *scalarTypeU16 = (SAP_UC*)RfcGetTypeAsString(type);
				char *scalarTypeU8;
				int scalarTypeU8Len;

				if (SUCCESS != sapuc_to_utf8(scalarTypeU16, &scalarTypeU8, &scalarTypeU8Len, err)) {
					return FAILURE;
				}
				
				php_sap_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for type %s must be double (%s given)", scalarTypeU8, zend_get_type_by_const(Z_TYPE_P(zvalue)));

				efree(scalarTypeU8);

				return FAILURE;
			}

			if (RFC_OK != RfcSetFloat(dh, name, Z_DVAL_P(zvalue), (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetInt", FAILURE);
			}

			if (UNEXPECTED(zvalue == &copy)) {
				zval_dtor(&copy); /* Do we really need this? */
			}

			break;
		}
		case RFCTYPE_BYTE:
		case RFCTYPE_XSTRING:
		{
			/* This data type requires php string */
			if (Z_TYPE_P(zvalue) != IS_STRING)
			{
				SAP_UC *scalarTypeU16 = (SAP_UC*)RfcGetTypeAsString(type);
				char *scalarTypeU8;
				int scalarTypeU8Len;

				if (SUCCESS != sapuc_to_utf8(scalarTypeU16, &scalarTypeU8, &scalarTypeU8Len, err)) {
					return FAILURE;
				}

				php_sap_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for type %s must be a string (%s given)", scalarTypeU8, zend_get_type_by_const(Z_TYPE_P(zvalue)));

				return FAILURE;
			}
			
			if (RFC_OK != RfcSetBytes(dh, name, (SAP_RAW*)Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetBytes", FAILURE);
			}

			break;
		}
#if HAVE_DATE
		case RFCTYPE_TIME:
		case RFCTYPE_DATE:
		{
			if (Z_TYPE_P(zvalue) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zvalue), php_date_get_date_ce() TSRMLS_CC))
			{
				zval rv;
				char *format = type == RFCTYPE_TIME ? "His" : "Ymd";
				RFC_CHAR *valU;
				unsigned int valUlen;

				if (format_datetime_object(zvalue, &rv, format TSRMLS_CC) != SUCCESS)
				{
					php_sap_error(err, "SAPRFC_CONVERSION_FAILURE", -1, "Could not format object of class %s to '%s'", sap_get_str_val(Z_OBJCE_P(zvalue)->name), format);
					return FAILURE;
				}

				if (SUCCESS != utf8_to_sapuc_l(Z_STRVAL(rv), Z_STRLEN(rv), &valU, &valUlen, err)) {
					return FAILURE;
				}

				zval_dtor(&rv);

				if (RFC_OK != RfcSetChars(dh, name, valU, valUlen, (RFC_ERROR_INFO*)err)) {
					SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetChars", FAILURE);
				}

				break;
			}
			else if (Z_TYPE_P(zvalue) == IS_STRING && Z_STRLEN_P(zvalue) == 0) {
				return SUCCESS;
			}
			/* else: go to case default */
		}
#endif
		default:
		{
			zval copy;
			unsigned int strU8len;
			SAP_UC *strU;
			unsigned int strU16len;

			if (Z_TYPE_P(zvalue) != IS_STRING)
			{
				ZVAL_ZVAL(&copy, zvalue, 1, 0);
				convert_to_string(&copy);
				zvalue = &copy;
			}

			strU8len = strlenU8(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue));

			if (strU8len > length)
			{
				char *sub;
				int sublen;

				if (SUCCESS == utf8_substr(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), 0, length, &sub, &sublen))
				{
					zval_dtor(&copy);

#if PHP_VERSION_ID < 70000
					ZVAL_STRINGL(&copy, sub, sublen, 0);
#else
					ZVAL_STRINGL(&copy, sub, sublen);
					efree(sub);
#endif	
					zvalue = &copy;
				}
			}

			if (SUCCESS != utf8_to_sapuc_l(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), &strU, &strU16len, err)) {
				return FAILURE;
			}

			if (UNEXPECTED(zvalue == &copy)) {
				zval_dtor(&copy);
			}

			if (RFC_OK != RfcSetChars(dh, name, strU, strU16len, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetChars", FAILURE);
			}
			
			break;
		}
	
	}
	return SUCCESS;
}

static int sap_import_structure(RFC_STRUCTURE_HANDLE sh, RFC_TYPE_DESC_HANDLE tdh, HashTable *fields, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	unsigned int fieldCount, i;

	if (RFC_OK != RfcGetFieldCount(tdh, &fieldCount, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFieldCount", FAILURE);
	}

	for (i = 0; i < fieldCount; i++)
	{
		char *fname;
		int fnamelen;
		RFC_FIELD_DESC field;
		zval *zfvalue;

		if (RFC_OK != RfcGetFieldDescByIndex(tdh, i, &field, (RFC_ERROR_INFO*)err)) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFieldDescByIndex", FAILURE);
		}

		if (SUCCESS != sapuc_to_utf8(field.name, &fname, &fnamelen, err)) {
			return FAILURE;
		}

		if (zfvalue = my_zend_hash_find_zval(fields, fname, fnamelen)) {
			if (SUCCESS != sap_import(sh, field.name, field.type, field.typeDescHandle, field.nucLength, zfvalue, err TSRMLS_CC)) {
				return FAILURE;
			}
		}

		efree(fname);
	}

	return SUCCESS;
}

static int sap_import_table(RFC_TABLE_HANDLE th, RFC_TYPE_DESC_HANDLE tdh, HashTable *rows, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	zval *zrow;

	MY_ZEND_HASH_FOREACH_VAL(rows, zrow)
	{
		RFC_STRUCTURE_HANDLE sh;

#if PHP_VERSION_ID >= 70000
		ZVAL_DEREF(zrow);
#endif

		if (Z_TYPE_P(zrow) != IS_ARRAY) {
			continue;
		}

		if (NULL == (sh = RfcAppendNewRow(th, (RFC_ERROR_INFO*)err))) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcAppendNewRow", FAILURE);
		}

		if (SUCCESS != sap_import_structure(sh, tdh, Z_ARRVAL_P(zrow), err TSRMLS_CC)) {
			return FAILURE;
		}
	}
	MY_ZEND_HASH_FOREACH_END();

	return SUCCESS;
}

static int sap_import(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{

#if PHP_VERSION_ID >= 70000
	ZVAL_DEREF(zvalue);
#endif

	switch (type)
	{
		case RFCTYPE_TABLE:
		{
			RFC_TABLE_HANDLE th;

			if (UNEXPECTED(Z_TYPE_P(zvalue) != IS_ARRAY)) {
				php_sap_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for a RFCTYPE_TABLE must be an array (%s given)", zend_get_type_by_const(Z_TYPE_P(zvalue)));
				return FAILURE;
			}

			if (UNEXPECTED(RFC_OK != RfcGetTable(dh, name, &th, (RFC_ERROR_INFO*)err))) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetTable", FAILURE);
			}

			return sap_import_table(th, tdh, Z_ARRVAL_P(zvalue), err TSRMLS_CC);
		}
		case RFCTYPE_STRUCTURE:
		{
			RFC_STRUCTURE_HANDLE sh;

			if (Z_TYPE_P(zvalue) != IS_ARRAY) {
				php_sap_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for a RFCTYPE_STRUCTURE must be an array (%s given)", zend_get_type_by_const(Z_TYPE_P(zvalue)));
				return FAILURE;
			}

			if (RFC_OK != RfcGetStructure(dh, name, &sh, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetStructure", FAILURE);
			}

			return sap_import_structure(sh, tdh, Z_ARRVAL_P(zvalue), err TSRMLS_CC);
		}
		default: {
			if (Z_TYPE_P(zvalue) == IS_NULL) {
				return SUCCESS; /* Leave default value */
			}

			return sap_import_scalar(dh, name, type, length, zvalue, err TSRMLS_CC);
		}
	}
}

static int sap_export_scalar(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, unsigned int nucLength, zval *rv, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	switch (type)
	{
		case RFCTYPE_INT:
		case RFCTYPE_INT1:
		case RFCTYPE_INT2:
		case RFCTYPE_INT8:
		{
			RFC_INT ival;

			if (RFC_OK != RfcGetInt(dh, name, &ival, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetInt", FAILURE);
			}

			ZVAL_LONG(rv, ival);

			break;
		}
		case RFCTYPE_BCD:
		case RFCTYPE_DECF16:
		case RFCTYPE_DECF34:
		case RFCTYPE_FLOAT:
		{
			RFC_FLOAT fval;

			if (RFC_OK != RfcGetFloat(dh, name, &fval, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFloat", FAILURE);
			}

			ZVAL_DOUBLE(rv, fval);

			break;
		}
		case RFCTYPE_BYTE:
		{
			SAP_RAW *buffer = emalloc(nucLength);

			memset(buffer, 0, nucLength);

			if (RFC_OK != RfcGetBytes(dh, name, buffer, nucLength, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetXString", FAILURE);
			}

#if PHP_VERSION_ID < 70000
			ZVAL_STRINGL(rv, (char*)buffer, nucLength, 0);
#else
			ZVAL_STRINGL(rv, (char*)buffer, nucLength);
			efree(buffer);
#endif
			

			break;
		}
		case RFCTYPE_XSTRING:
		{
			SAP_RAW *rawBuffer = NULL;
			unsigned int rawBufferLen = nucLength * sizeof(SAP_RAW);

			/*
			* RFCTYPE_XSTRING is a variable length type and we don't know what buffer size is required
			* The NW library claims the length to be 0 so we will use RfcGetXString to find out
			* the exact buffer length we need
			*/

		try_again_xstring:

			/* Free previous allocated buffer, if any */
			if (rawBuffer) {
				efree(rawBuffer);
			}

			rawBuffer = emalloc(rawBufferLen);
			memset(rawBuffer, 0, rawBufferLen);

			switch (RfcGetXString(dh, name, rawBuffer, rawBufferLen, &rawBufferLen, (RFC_ERROR_INFO*)err))
			{
				case RFC_BUFFER_TOO_SMALL:
					/*
					* if RFC_BUFFER_TOO_SMALL then the rawBufferLen variable contains the required number of bytes
					* to properly export this RFCTYPE_XSTRING value
					*/
					goto try_again_xstring;

				case RFC_OK: break;

				default: {
					efree(rawBuffer);
					SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetXString", FAILURE);
				}
			}

#if PHP_VERSION_ID < 70000
			ZVAL_STRINGL(rv, (char*)rawBuffer, rawBufferLen, 0);
#else
			ZVAL_STRINGL(rv, (char*)rawBuffer, rawBufferLen);
			efree(rawBuffer);
#endif
			break;
		}
		case RFCTYPE_STRING:
		{
			SAP_UC *bufferU16 = NULL;
			unsigned int bufferU16len = nucLength; /* SAP_UC chars */
			char *str;
			int len; /* bytes */

			/*
			 * RFCTYPE_STRING is a variable length type and we don't know what buffer size is required
			 * The NW library claims the length to be 0 so we will use RfcGetString to find out 
			 * the exact buffer length we need 
			 */

		try_again_string:

			/* Free previous allocated buffer, if any */
			if (bufferU16) {
				efree(bufferU16);
			}

			/* The result will be null-terminated by the nw library so we have to allocate one more SAP_UC char */
			bufferU16len++;

			bufferU16 = emalloc(bufferU16len * sizeof(SAP_UC));
			memset(bufferU16, 0, bufferU16len * sizeof(SAP_UC));

			switch (RfcGetString(dh, name, bufferU16, bufferU16len, &bufferU16len, (RFC_ERROR_INFO*)err))
			{
				case RFC_BUFFER_TOO_SMALL:
					/*
					* if RFC_BUFFER_TOO_SMALL then the numChars variable contains the required number of SAP_UC characters
					* to properly export this RFCTYPE_STRING value
					*/
					goto try_again_string;
				case RFC_OK: break;
				default: {
					efree(bufferU16);
					SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetString", FAILURE);
				}
			}
			
			if (SUCCESS != sapuc_to_utf8_l(bufferU16, bufferU16len, &str, &len, err)) {
				return FAILURE;
			}

			efree(bufferU16);

			if (len > 0) {
#if PHP_VERSION_ID < 70000
				ZVAL_STRINGL(rv, str, len, 0);
#else
				ZVAL_STRINGL(rv, str, len);
				efree(str);
#endif
			}
			else {
				efree(str);
				ZVAL_NULL(rv);
			}

			

			break;
		}
		default:
		{
			RFC_CHAR *uChars = emalloc(nucLength * sizeof(RFC_CHAR));
			char *str;
			int len;

			memset(uChars, 0, nucLength * sizeof(RFC_CHAR));
			

			if (RFC_OK != RfcGetChars(dh, name, uChars, nucLength, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetChars", FAILURE);
			}

			if (SUCCESS != sapuc_to_utf8_l(uChars, nucLength, &str, &len, err)) {
				return FAILURE;
			}

			efree(uChars);

			if (PHP_SAP_GLOBALS(rtrim_export_strings))
			{
				char *trimmed_str = NULL;
				int trimmed_str_len;

				utf8_trim(str, len, &trimmed_str, &trimmed_str_len, TRIM_RIGHT);

				if (NULL != trimmed_str)
				{
					char *oldstr = str;

					str = trimmed_str;
					len = trimmed_str_len;
					
					efree(oldstr);
				}
			}

			if (len > 0) {
#if PHP_VERSION_ID < 70000
				ZVAL_STRINGL(rv, str, len, 0);
#else
				ZVAL_STRINGL(rv, str, len);
				efree(str);
#endif
			}
			else {
				efree(str);
				ZVAL_NULL(rv);
			}

			break;
		}
	}

	return SUCCESS;
}

static int sap_export_structure(RFC_STRUCTURE_HANDLE sh, RFC_TYPE_DESC_HANDLE tdh, zval *rv, SAPRFC_ERROR_INFO *err TSRMLS_DC) {
	int isArrayAccess = FALSE;
	unsigned int fieldCount, i;

	if (RFC_OK != RfcGetFieldCount(tdh, &fieldCount, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFieldCount", FAILURE);
	}

	array_init_size(rv, fieldCount);

	for (i = 0; i < fieldCount; i++)
	{
		RFC_FIELD_DESC field;
		char *fname;
		int fnamelen;
		zval *pfval;

		if (RFC_OK != RfcGetFieldDescByIndex(tdh, i, &field, (RFC_ERROR_INFO*)err)) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFieldDescByIndex", FAILURE);
		}

		if (SUCCESS != sapuc_to_utf8(field.name, &fname, &fnamelen, err)) {
			return FAILURE;
		}

#if PHP_VERSION_ID < 70000
		MAKE_STD_ZVAL(pfval);
#else
		pfval = &EG(uninitialized_zval);
#endif
		pfval = my_zend_hash_add_zval(Z_ARRVAL_P(rv), fname, fnamelen, pfval);

		efree(fname);

		if (NULL == pfval) { /* Out of memory, full table etc... */
			break;
		}

		if (SUCCESS != sap_export(sh, field.name, field.type, field.typeDescHandle, field.nucLength, pfval, err TSRMLS_CC)) {
			return FAILURE;
		}
	}

	return SUCCESS;
}

static int sap_export_table(RFC_TABLE_HANDLE th, RFC_TYPE_DESC_HANDLE tdh, zval *rv, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	int retval = SUCCESS;
	unsigned int rowCount;
	RFC_ERROR_INFO e;
	RFC_RC rc;

	if (RFC_OK != RfcGetRowCount(th, &rowCount, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetRowCount", FAILURE);
	}

	array_init_size(rv, rowCount);

	for (rc = RfcMoveToFirstRow(th, &e); rc == RFC_OK; rc = RfcMoveToNextRow(th, &e))
	{
		RFC_STRUCTURE_HANDLE sh;
		zval *pzrow = NULL;

		if (NULL == (sh = RfcGetCurrentRow(th, (RFC_ERROR_INFO*)err))) {
			retval = FAILURE;
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetCurrentRow", FAILURE);
		}

		if (!(pzrow = my_zend_hash_next_index_insert_new_zval(Z_ARRVAL_P(rv)))) { /* Out of memory, full hashtable etc... */ 
			break;
		}

		if (SUCCESS != sap_export_structure(sh, tdh, pzrow, err TSRMLS_CC)) {
			return FAILURE;
		}
	}

	return retval;
}

static int sap_export(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *rv, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	switch (type)
	{
		case RFCTYPE_TABLE:
		{
			RFC_TABLE_HANDLE th;

			if (RFC_OK != RfcGetTable(dh, name, &th, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetTable", FAILURE);
			}

			return sap_export_table(th, tdh, rv, err TSRMLS_CC);
		}
		case RFCTYPE_STRUCTURE:
		{
			RFC_STRUCTURE_HANDLE sh;

			if (RFC_OK != RfcGetStructure(dh, name, &sh, (RFC_ERROR_INFO*)err)) {
				SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetTable", FAILURE);
			}

			return sap_export_structure(sh, tdh, rv, err TSRMLS_CC);
		}
		default: {
			return sap_export_scalar(dh, name, type, length, rv, err TSRMLS_CC);
		}
	}
}

static php_sap_function * sap_fetch_function(char *name, int nameLen, php_sap_connection *connection, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	SAP_UC *nameU;
	unsigned int nameUlen;
	RFC_FUNCTION_DESC_HANDLE fdh;
	int connectionIsValid = 0;
	
	if (NULL == connection->handle || RFC_OK != RfcIsConnectionHandleValid(connection->handle, &connectionIsValid, (RFC_ERROR_INFO*)err) || !connectionIsValid) {
		php_sap_error(err, "SAPRFC_INVALID_CONNECTION", -1, "There is no active connection to a SAP System");
		return NULL;
	}
	
	if (SUCCESS != utf8_to_sapuc_l(name, nameLen, &nameU, &nameUlen, err)) {
		return NULL;
	}

	fdh = RfcGetFunctionDesc(connection->handle, nameU, (RFC_ERROR_INFO*)err);
	efree(nameU);

	if (NULL == fdh) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFunctionDesc", NULL);
	}
	
	return sap_create_function_from_descr_handle(fdh, err TSRMLS_CC);
}

static int sap_function_invoke(php_sap_function *function, php_sap_connection *connection, HashTable *imports, HashTable *exports, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	RFC_FUNCTION_HANDLE fh;
	char *pname;
	int pnamelen;
	SAPRFC_PARAMETER_DESC *sp;
	
	if (NULL == (fh = RfcCreateFunction(function->fdh, (RFC_ERROR_INFO*)err))) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcCreateFunction", FAILURE);
	}

	MY_ZEND_HASH_FOREACH_STR_KEY_PTR(function->params, pname, pnamelen, sp)
	{
		zval *zpvalue;

		if (sp->state != SAPRFC_PARAM_DEFAULT && RFC_OK != RfcSetParameterActive(fh, sp->param.name, sp->state, (RFC_ERROR_INFO*)err)) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetParameterActive", FAILURE);
		}

		if (NULL == imports || !(sp->param.direction & RFC_IMPORT) || sp->state == SAPRFC_PARAM_INACTIVE) {
			continue;
		}

		zpvalue = my_zend_hash_find_zval(imports, pname, pnamelen);

		if (NULL == zpvalue) {
			continue;
		}
		
		if (SUCCESS != sap_import(fh, sp->param.name, sp->param.type, sp->param.typeDescHandle, sp->param.nucLength, zpvalue, err TSRMLS_CC)) {
			return FAILURE;
		}
	}
	MY_ZEND_HASH_FOREACH_END();
	
	if (RFC_OK != RfcInvoke(connection->handle, fh, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcInvoke", FAILURE);
	}
	
	MY_ZEND_HASH_FOREACH_STR_KEY_PTR(function->params, pname, pnamelen, sp)
	{
		zval *ev = NULL;

		if (!(sp->param.direction & RFC_EXPORT) || sp->state == SAPRFC_PARAM_INACTIVE) {
			continue;
		}

		ev = my_zend_hash_add_new_zval(exports, pname, pnamelen);

		if (NULL == ev) { /* out of memory, full hashtable etc */
			break;
		}

		if (SUCCESS != sap_export(fh, sp->param.name, sp->param.type, sp->param.typeDescHandle, sp->param.nucLength, ev, err TSRMLS_CC)) {
			return FAILURE;
		}
	}
	MY_ZEND_HASH_FOREACH_END();
	
	RfcDestroyFunction(fh, (RFC_ERROR_INFO*)err);

	return SUCCESS;
}

PHP_FUNCTION(sap_connect)
{
	int res;
	zval *logonParameters = NULL;
	php_sap_connection *crsrc;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "a", &logonParameters);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		RETURN_FALSE;
	}

	crsrc = sap_create_connection_resource(Z_ARRVAL_P(logonParameters));
	crsrc->refCount++;

	if (SUCCESS != sap_connection_open(crsrc, &sap_last_error)) {
		php_sap_connection_ptr_dtor(crsrc);
		RETURN_FALSE;
	}

	sap_make_resource(return_value, crsrc, le_php_sap_connection);
}

PHP_FUNCTION(sap_last_error)
{
	char *key, *msgType, *msgClass, *msgNumber, *msgv1, *msgv2, *msgv3, *msgv4, *message;
	int keyLen, msgTypeLen, msgClassLen, msgNumberLen, msgv1Len, msgv2Len, msgv3Len, msgv4Len, messageLen;
	SAPRFC_ERROR_INFO e;

	array_init(return_value);

	add_assoc_long(return_value, "CODE", sap_last_error.err.code);

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.key, &key, &keyLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "KEY", key, keyLen);
		efree(key);
#else
		add_assoc_stringl(return_value, "KEY", key, keyLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgType, &msgType, &msgTypeLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGTY", msgType, msgTypeLen);
		efree(msgType);
#else
		add_assoc_stringl(return_value, "MSGTY", msgType, msgTypeLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgClass, &msgClass, &msgClassLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGID", msgClass, msgClassLen);
		efree(msgClass);
#else
		add_assoc_stringl(return_value, "MSGID", msgClass, msgClassLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgNumber, &msgNumber, &msgNumberLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGNO", msgNumber, msgNumberLen);
		efree(msgNumber);
#else
		add_assoc_stringl(return_value, "MSGNO", msgNumber, msgNumberLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV1, &msgv1, &msgv1Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGV1", msgv1, msgv1Len);
		efree(msgv1);
#else
		add_assoc_stringl(return_value, "MSGV1", msgv1, msgv1Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV2, &msgv2, &msgv2Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGV2", msgv2, msgv2Len);
		efree(msgv2);
#else
		add_assoc_stringl(return_value, "MSGV2", msgv2, msgv2Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV3, &msgv3, &msgv3Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGV3", msgv3, msgv3Len);
		efree(msgv3);
#else
		add_assoc_stringl(return_value, "MSGV3", msgv3, msgv3Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV4, &msgv4, &msgv4Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGV4", msgv4, msgv4Len);
		efree(msgv4);
#else
		add_assoc_stringl(return_value, "MSGV4", msgv4, msgv4Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.message, &message, &messageLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "MSGTEXT", message, messageLen);
		efree(message);
#else
		add_assoc_stringl(return_value, "MSGTEXT", message, messageLen, 0);
#endif
	}

	if (sap_last_error.l_nwsdkfunction > 0) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "nwsdkfunction", sap_last_error.nwsdkfunction, sap_last_error.l_nwsdkfunction);
#else
		add_assoc_stringl(return_value, "nwsdkfunction", sap_last_error.nwsdkfunction, sap_last_error.l_nwsdkfunction, 1);
#endif
	}
}

PHP_FUNCTION(sap_invoke_function)
{
	int res;
	char *name;
	int namelen;
	zval *zcrsrc, *zimports = NULL;
	HashTable *imports = NULL;
	php_sap_function *frsrc;
	php_sap_connection *crsrc;
	int invoke_result;
	
	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "sr|a", &name, &namelen, &zcrsrc, &zimports);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		RETURN_FALSE;
	}

	if (NULL == (crsrc = sap_fetch_connection_rsrc(zcrsrc))) {
		RETURN_FALSE;
	}

	if (NULL == (frsrc = sap_fetch_function(name, namelen, crsrc, &sap_last_error TSRMLS_CC))) {
		RETURN_FALSE;
	}

	if (NULL != zimports) {
		imports = Z_ARRVAL_P(zimports);
	}

	array_init(return_value);

	invoke_result = sap_function_invoke(frsrc, crsrc, imports, Z_ARRVAL_P(return_value), &sap_last_error TSRMLS_CC);

	php_sap_function_ptr_dtor(frsrc);

	if (SUCCESS != invoke_result) {
		zval_dtor(return_value);
		RETURN_FALSE;
	}
}

PHP_METHOD(SapException, getMessageType)
{
	zval *zMessageType = sap_read_object_property_ex(getThis(), "MSGTY", sizeof("MSGTY") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageType, 1, 0);
}

PHP_METHOD(SapException, getMessageId)
{
	zval *zMessageId = sap_read_object_property_ex(getThis(), "MSGID", sizeof("MSGID") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageId, 1, 0);
}

PHP_METHOD(SapException, getMessageNumber)
{
	zval *zMessageNumber = sap_read_object_property_ex(getThis(), "MSGNO", sizeof("MSGNO") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageNumber, 1, 0);
}

PHP_METHOD(SapException, getMessageVar1)
{
	zval *zMessageVar1 = sap_read_object_property_ex(getThis(), "MSGV1", sizeof("MSGV1") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageVar1, 1, 0);
}

PHP_METHOD(SapException, getMessageVar2)
{
	zval *zMessageVar2 = sap_read_object_property_ex(getThis(), "MSGV2", sizeof("MSGV2") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageVar2, 1, 0);
}

PHP_METHOD(SapException, getMessageVar3)
{
	zval *zMessageVar3 = sap_read_object_property_ex(getThis(), "MSGV3", sizeof("MSGV3") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageVar3, 1, 0);
}

PHP_METHOD(SapException, getMessageVar4)
{
	zval *zMessageVar4 = sap_read_object_property_ex(getThis(), "MSGV4", sizeof("MSGV4") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageVar4, 1, 0);
}

PHP_METHOD(SapException, getMessageKey)
{
	zval *zMessageKey = sap_read_object_property_ex(getThis(), "KEY", sizeof("KEY") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zMessageKey, 1, 0);
}

PHP_METHOD(SapException, getNwSdkFunction)
{
	zval *zNwSdkFunction = sap_read_object_property_ex(getThis(), "nwsdkfunction", sizeof("nwsdkfunction") - 1, Z_OBJCE_P(getThis()));

	RETURN_ZVAL(zNwSdkFunction, 1, 0);
}

PHP_METHOD(Sap, __construct)
{
	sap_object *intern;
	int res;
	zval *zlogonParameters = NULL;
	SAPRFC_ERROR_INFO error;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zlogonParameters);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		return;
	}

	intern = sap_get_sap_object(getThis());

	/* if constructor is called again, destroy previous connection */
	if (intern->connection) {
		php_sap_connection_ptr_dtor(intern->connection);
	}

	intern->connection = sap_create_connection_resource(Z_ARRVAL_P(zlogonParameters));
	intern->connection->refCount++;

	if (SUCCESS != sap_connection_open(intern->connection, &error)) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}
}

PHP_METHOD(Sap, setFunctionClass)
{
	int res;
	zend_class_entry *fce;
	sap_object *intern;

	fce = php_sap_get_function_ce();

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "C", &fce);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		return;
	}

	intern = sap_get_sap_object(getThis());
	intern->func_ce = fce;
}

PHP_METHOD(Sap, call)
{
	int res;
	char *name;
	int namelen;
	zval *zimports = NULL;
	sap_object *intern;
	HashTable *imports = NULL;
	php_sap_function *func;
	int cresult;
	SAPRFC_ERROR_INFO error;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &name, &namelen, &zimports);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		RETURN_FALSE;
	}

	intern = sap_get_sap_object(getThis());

	if (NULL == intern->connection) {
		sap_throw_exception("There is no connection to a SAP system", -1, sap_ce_SapException);
		RETURN_FALSE;
	}

	/* Fetch function's information */
	func = sap_fetch_function(name, namelen, intern->connection, &error TSRMLS_CC);

	/* Could not fetch function information */
	if (NULL == func) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}

	if (NULL != zimports) {
		imports = Z_ARRVAL_P(zimports);
	}

	/* Exports will be stored in return_value's hashtable */
	array_init(return_value);

	/* Invoke the function module */
	cresult = sap_function_invoke(func, intern->connection, imports, Z_ARRVAL_P(return_value), &error TSRMLS_CC);

	/* Free function's resources */
	php_sap_function_ptr_dtor(func);

	/* Call failed */
	if (SUCCESS != cresult) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}
}

PHP_METHOD(Sap, fetchFunction)
{
	int res;
	zval *zfunction;
	char *function_name;
	int function_len;
	zend_class_entry *fce = sap_ce_SapFunction;
	zval *zargs = NULL;
	sap_object *intern;
	php_sap_function *function_descr_rsrc;
	sap_function *func;
	SAPRFC_ERROR_INFO error;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "z|Ca", &zfunction, &fce, &zargs);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		return;
	}

	switch (Z_TYPE_P(zfunction))
	{
		case IS_STRING:
			function_name = estrndup(Z_STRVAL_P(zfunction), Z_STRLEN_P(zfunction));
			function_len = Z_STRLEN_P(zfunction);
			break;
		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(zfunction), sap_ce_SapFunction TSRMLS_CC))
			{
				/**
				* If the first argument is a SapFunction object, get function's name
				* by calling SapFunction::getName method
				*/
				zval *retval_ptr;
#if PHP_VERSION_ID >= 70000
				zval rv;

				retval_ptr = &rv;

				/* all functions/methods for php objects are stored in lower case */
				if (SUCCESS != sap_call_object_method(zfunction, Z_OBJCE_P(zfunction), "getname", NULL, NULL, retval_ptr)) {
#else
				if (SUCCESS != sap_call_object_method(zfunction, Z_OBJCE_P(zfunction), "getname", NULL, NULL, &retval_ptr TSRMLS_CC)) {
#endif
					RETURN_FALSE;
				}

				if (Z_TYPE_P(retval_ptr) != IS_STRING) {
					zend_error_noreturn(E_ERROR, "Method %s::getName() should return a string (%s returned)", sap_get_str_val(fce->name), zend_get_type_by_const(Z_TYPE_P(retval_ptr)));
					return;
				}

				function_name = estrndup(Z_STRVAL_P(retval_ptr), Z_STRLEN_P(retval_ptr));
				function_len = Z_STRLEN_P(retval_ptr);

				my_zval_ptr_dtor(retval_ptr);

				break;
			}
		default:
			sap_throw_exception("Argument 1 of Sap::fetchFunction() must be a string or a SapFunction object", -1, zend_invalid_args_exception);
			RETURN_FALSE;
	}

	intern = sap_get_sap_object(getThis());

	/* Get function description from SAP backend */
	function_descr_rsrc = sap_fetch_function(function_name, function_len, intern->connection, &error TSRMLS_CC);

	efree(function_name);

	if (NULL == function_descr_rsrc) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}

	if (Z_TYPE_P(zfunction) == IS_OBJECT)
	{
		/**
		 * If the first argument is a SapFunction object, function's description will be stored
		 * in that object, instead of creating a new one
		 */

		RETVAL_ZVAL(zfunction, 1, 0);
	}
	else {
		/**
		 * Create a new SapFunction object. If a class was provided (2nd argument)
		 * we will create an instance of that class using the 3rd parameter (array) as
		 * constructor arguments. If not, we will use the default class of this Sap object
		 */
		if (ZEND_NUM_ARGS() < 2) {
			fce = intern->func_ce;
		}

		/* Create the SapFunction object and set it as return value */
		object_init_ex(return_value, fce);

		/* Call object's constructor, if exists */
		if (fce->constructor)
		{
			int retval;
			zval *rv_ptr;
#if PHP_VERSION_ID >= 70000
			zval rv;

			rv_ptr = &rv;

			retval = sap_call_object_method(return_value, fce, ZSTR_VAL(fce->constructor->common.function_name), fce->constructor, zargs, rv_ptr TSRMLS_CC);
#else
			retval = sap_call_object_method(return_value, fce, fce->constructor->common.function_name, fce->constructor, zargs, &rv_ptr TSRMLS_CC);
#endif
			/* we are not interested in constructor's return value */
			my_zval_ptr_dtor(rv_ptr);

			if (SUCCESS != retval) {
				php_error(E_ERROR, "Could not call '%s' object's constructor", sap_get_str_val(fce->name));
				RETURN_FALSE;
			}
		}
	}
	
	/* Fetch the sap_function object from the return value */
	func = sap_get_function(return_value);

	/* If a function resource was previously set for the returning SapFunction object, destroy it */
	if (UNEXPECTED(NULL != func->function_descr)) {
		php_sap_function_ptr_dtor(func->function_descr);
	}

	/* Assign the function's description resource */
	func->function_descr = function_descr_rsrc;

	/* If a connection resource was previously set for the returning SapFunction object, destroy it */
	if (UNEXPECTED(NULL != func->connection)) {
		php_sap_connection_ptr_dtor(func->connection);
	}

	/* Assign the connection resource */
	func->connection = intern->connection;
	func->connection->refCount++;
}

PHP_METHOD(SapFunction, getName)
{
	sap_function *intern;
	RFC_ABAP_NAME funcNameU;
	char *name;
	int namelen;
	SAPRFC_ERROR_INFO error;

	intern = sap_get_function(getThis());

	if (NULL == intern->function_descr) {
#if PHP_VERSION_ID >= 70000
		RETURN_STRING("");
#else
		RETURN_STRING("", 0);
#endif
	}

	if (RFC_OK != RfcGetFunctionName(intern->function_descr->fdh, funcNameU, (RFC_ERROR_INFO*)&error)) {
		SAP_ERROR_SET_RFCFUNCTION(&sap_last_error, "RfcGetFunctionName", sizeof("RfcGetFunctionName") - 1);
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}

	if (SUCCESS != sapuc_to_utf8(funcNameU, &name, &namelen, &error)) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}

#if PHP_VERSION_ID >= 70000
	ZVAL_STRINGL(return_value, name, namelen);
	efree(name);
#else
	ZVAL_STRINGL(return_value, name, namelen, 0);
#endif
}

PHP_METHOD(SapFunction, setActive)
{
	int res;
	char *pname;
	int pnamelen;
	int isActive = SAPRFC_PARAM_DEFAULT;
	sap_function *intern;
	SAPRFC_PARAMETER_DESC *sp;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &pname, &pnamelen, &isActive);
	PHP_SAP_PARSE_PARAMS_END();

	RETVAL_FALSE;

	if (FAILURE == res) {
		return;
	}
	
	intern = sap_get_function(getThis());

	if (!intern->function_descr) {
		return;
	}

	if (sp = my_zend_hash_find_ptr(intern->function_descr->params, pname, pnamelen)) {
		sp->state = isActive;
		RETURN_TRUE;
	}
}

PHP_METHOD(SapFunction, __invoke)
{
	int res;
	zval *args = NULL;
	HashTable *imports = NULL;
	sap_function *intern;
	SAPRFC_ERROR_INFO error;
	
	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &args);
	PHP_SAP_PARSE_PARAMS_END();

	RETVAL_FALSE;

	if (res == FAILURE) {
		return;
	}

	if (NULL != args) {
		imports = Z_ARRVAL_P(args);
	}

	intern = sap_get_function(getThis());

	if (NULL == intern->function_descr || NULL == intern->connection) {
		return;
	}

	array_init(return_value);

	if (SUCCESS != sap_function_invoke(intern->function_descr, intern->connection, imports, Z_ARRVAL_P(return_value), &error TSRMLS_CC))
	{
		zval_dtor(return_value);

		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
		RETURN_FALSE;
	}
}

PHP_METHOD(SapFunction, getParameters)
{
	sap_function *intern;
	char *pname;
	int pnamelen;

	intern = sap_get_function(getThis());

	if (!intern->function_descr) {
		RETURN_FALSE;
	}

	array_init_size(return_value, zend_hash_num_elements(intern->function_descr->params));

	MY_ZEND_HASH_FOREACH_STR_KEY(intern->function_descr->params, pname, pnamelen)
	{
#if PHP_VERSION_ID >= 70000
		add_next_index_stringl(return_value, pname, pnamelen);
#else
		add_next_index_stringl(return_value, pname, pnamelen, 1);
#endif
	}
	MY_ZEND_HASH_FOREACH_END();
}

PHP_METHOD(SapFunction, getTypeName)
{
	int res;
	char *pname;
	int pnamelen;
	sap_function *intern;
	SAPRFC_PARAMETER_DESC *sp;
	SAPRFC_ERROR_INFO error;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "s", &pname, &pnamelen);
	PHP_SAP_PARSE_PARAMS_END();

	RETVAL_FALSE;

	if (FAILURE == res) {
		return;
	}

	intern = sap_get_function(getThis());

	if (!intern->function_descr || !(sp = my_zend_hash_find_ptr(intern->function_descr->params, pname, pnamelen))) {
		return;
	}

	if (sp->param.type == RFCTYPE_TABLE || sp->param.type == RFCTYPE_STRUCTURE)
	{
		RFC_ABAP_NAME typeNameU;
		char *typeName;
		int typeNameLen;

		memset(&error, 0, sizeof(SAPRFC_ERROR_INFO));

		if (RFC_OK != RfcGetTypeName(sp->param.typeDescHandle, typeNameU, (RFC_ERROR_INFO*)&error)) {
			SAP_ERROR_SET_RFCFUNCTION(&error, "RfcGetTypeName", sizeof("RfcGetTypeName") - 1);
			SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
			RETURN_FALSE;
		}

		if (SUCCESS != sapuc_to_utf8(typeNameU, &typeName, &typeNameLen, &error)) {
			SAP_THROW_SAPRFC_ERROR_EXCEPTION(&error);
			RETURN_FALSE;
		}

#if PHP_VERSION_ID >= 70000
		ZVAL_STRINGL(return_value, typeName, typeNameLen);
		efree(typeName);
#else
		ZVAL_STRINGL(return_value, typeName, typeNameLen, 0);
#endif
	}
}

PHP_METHOD(SapRfcReadTable, getName)
{
#if PHP_VERSION_ID >= 70000
	RETURN_STRING("RFC_READ_TABLE");
#else
	RETURN_STRING("RFC_READ_TABLE", 1);
#endif
}

PHP_METHOD(SapRfcReadTable, select)
{
	zval *zfields;
	char *table_name;
	int table_name_len;
	zval *zwhere = NULL;
	int rowCount = 0;
	int offset = 0;
	int res;
	zval __invoke_args;
	zval *pzimports;
	
	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "zs|all", &zfields, &table_name, &table_name_len, &zwhere, &rowCount, &offset);
	PHP_SAP_PARSE_PARAMS_END();

	if (res == FAILURE) {
		return;
	}

	/* The array of arguments that will be used to call the __invoke method of this SapRfcReadTable object */
	array_init(&__invoke_args);

	/* The first (and only) argument of __invoke_args array, will be the RFC_READ_TABLE import parameters */
	pzimports = my_zend_hash_next_index_insert_new_zval(Z_ARRVAL(__invoke_args));

	if (NULL == pzimports) {
		/* Appending a zval to __invoke_args array failed */
		zval_dtor(&__invoke_args);
		RETURN_FALSE;
	}

	/* Init the import parameters array zval */
	array_init(pzimports);

	/* Create the QUERY_TABLE parameter */
	{
		zval *zparam_QUERY_TABLE;

		zparam_QUERY_TABLE = my_zend_hash_add_new_zval(Z_ARRVAL_P(pzimports), "QUERY_TABLE", sizeof("QUERY_TABLE") - 1);

		if (NULL != zparam_QUERY_TABLE) {
#if PHP_VERSION_ID >= 70000
			ZVAL_STRINGL(zparam_QUERY_TABLE, table_name, table_name_len);
#else
			ZVAL_STRINGL(zparam_QUERY_TABLE, table_name, table_name_len, 1);
#endif
		}
	}

	/* Create the DELIMITER parameter and set it to "" */
	{
		zval *zparam_DELIMITER;

		zparam_DELIMITER = my_zend_hash_add_new_zval(Z_ARRVAL_P(pzimports), "DELIMITER", sizeof("DELIMITER") - 1);

		if (NULL != zparam_DELIMITER) {
#if PHP_VERSION_ID >= 70000
			ZVAL_STRING(zparam_DELIMITER, "");
#else
			ZVAL_STRING(zparam_DELIMITER, "", 1);
#endif
		}
	}

	/* Create the ROWCOUNT parameter */
	if (rowCount > 0)
	{
		zval *zparam_ROWCOUNT;

		zparam_ROWCOUNT = my_zend_hash_add_new_zval(Z_ARRVAL_P(pzimports), "ROWCOUNT", sizeof("ROWCOUNT") - 1);
		
		if (NULL != zparam_ROWCOUNT) {
			ZVAL_LONG(zparam_ROWCOUNT, rowCount);
		}
	}

	/* Create the ROWSKIPS parameter */
	if (offset > 0)
	{
		zval *zparam_ROWSKIPS;

		zparam_ROWSKIPS = my_zend_hash_add_new_zval(Z_ARRVAL_P(pzimports), "ROWSKIPS", sizeof("ROWSKIPS") - 1);

		if (NULL != zparam_ROWSKIPS) {
			ZVAL_LONG(zparam_ROWSKIPS, offset);
		}
	}

	/* Create the FIELDS parameter */
	{
		zval *zparam_FIELDS;

		zparam_FIELDS = my_zend_hash_add_new_zval(Z_ARRVAL_P(pzimports), "FIELDS", sizeof("FIELDS") - 1);

		if (NULL != zparam_FIELDS)
		{
			zval zfieldsAsArray;

			/* FIELDS parameter is a table */
			array_init(zparam_FIELDS);

			if (Z_TYPE_P(zfields) == IS_STRING)
			{
				zval *zfieldname = zfields;

				if (Z_STRLEN_P(zfieldname) == 0) {
					zend_throw_exception(zend_invalid_args_exception, "SapRfcReadTable::select() : Empty string as query field is not allowed", -1);
					RETURN_FALSE;
				}

				/**
				 * If user provided a string as query fields then we will convert to array as follows:
				 * 1) if the string equals to sql star (*), we will convert to an empty array,
				 *    as an empty FIELDS parameter will select all table's fields
				 * 2) otherwise we will convert to an array with only one element (the provided field name)
				 */

				zfields = &zfieldsAsArray;

				array_init(zfields);

				if (0 != memcmp(Z_STRVAL_P(zfieldname), "*", sizeof("*") - 1))
				{
					my_zend_hash_next_index_insert_zval(Z_ARRVAL_P(zfields), zfieldname);

					Z_ADDREF_P(zfieldname);
				}
			}
			else if (Z_TYPE_P(zfields) != IS_ARRAY)
			{
				const char *exMessageFormat = "Argument 1 of SapRfcReadTable::select() must be a string or array. %s given";
				const char *typeGiven = zend_get_type_by_const(Z_TYPE_P(zfields));

				zend_throw_exception_ex(zend_invalid_args_exception, -1, exMessageFormat, typeGiven);
				RETURN_FALSE;
			}

			/* Append requested fields to the FIELDS table parameter */
			do
			{
				zval *zfieldname;
				ulong h;
				char *alias;
				int aliaslen;

				MY_ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(zfields), h, alias, aliaslen, zfieldname)
				{
					zval *zfieldrow;
					zval *zalias = NULL;
#if PHP_VERSION_ID >= 70000
					zval zaliasval;

					zalias = &zaliasval;
#endif

					/* Field names must be strings */
					if (Z_TYPE_P(zfieldname) != IS_STRING) {
						php_error(E_WARNING, "Ignoring incorrect field %s", alias ? "alias" : "name");
						continue;
					}

					/**
					 * If a string key is set for the current element then this will be used as field name
					 * and the zfieldname will contain the alias name that will be used later on export
					 */
					if (alias)
					{
#if PHP_VERSION_ID >= 70000
						ZVAL_STRINGL(zalias, alias, aliaslen);
#else
						MAKE_STD_ZVAL(zalias);
						ZVAL_STRINGL(zalias, alias, aliaslen, 1);
#endif
						/* set reference count to zero for now, it will be increased later */
						Z_DELREF_P(zalias);

						/* replace field's name */
						zfieldname = zalias;
					}

					/* Append a new row to FIELDS table */
					zfieldrow = my_zend_hash_next_index_insert_new_zval(Z_ARRVAL_P(zparam_FIELDS));

					if (zfieldrow)
					{
						/* Each FIELDS table row, is an array itself */
						array_init_size(zfieldrow, 1);

						if (my_zend_hash_add_zval(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1, zfieldname)) {
							Z_ADDREF_P(zfieldname);
						}
					}
				}
				MY_ZEND_HASH_FOREACH_END();
			} while (0);
		}
	}

	/* Create the 'where' clause (OPTIONS parameter) */
	if (NULL != zwhere && zend_hash_num_elements(Z_ARRVAL_P(zwhere)) > 0)
	{
		char *field;
		int field_len;
		ulong h;
		zval *val;
		zval *zparam_OPTIONS = NULL;

		zparam_OPTIONS = my_zend_hash_add_new_zval(Z_ARRVAL_P(pzimports), "OPTIONS", sizeof("OPTIONS") - 1);

		if (NULL != zparam_OPTIONS)
		{
			/* Parameter OPTIONS is a table */
			array_init(zparam_OPTIONS);

			MY_ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(zwhere), h, field, field_len, val)
			{
				zval *pzrealval = NULL;
				zval *zoptionsrow = NULL;
				zval *ztextfield = NULL;
#if PHP_VERSION_ID >= 70000
				zval zrealval;

				pzrealval = &zrealval;
#endif

				/* append new row to table 'OPTIONS' */
				zoptionsrow = my_zend_hash_next_index_insert_new_zval(Z_ARRVAL_P(zparam_OPTIONS));

				if (NULL == zoptionsrow) {
					break;
				}

				array_init_size(zoptionsrow, 1);

				/* append field 'TEXT' to the new row */
				ztextfield = my_zend_hash_add_new_zval(Z_ARRVAL_P(zoptionsrow), "TEXT", sizeof("TEXT") - 1);

				if (NULL == ztextfield) {
					break;
				}

				/* element's value must be a string */
				if (Z_TYPE_P(val) != IS_STRING)
				{
#if PHP_VERSION_ID < 70000
					MAKE_STD_ZVAL(pzrealval);
#endif
					ZVAL_ZVAL(pzrealval, val, 1, 0);
					convert_to_string(pzrealval);
					val = pzrealval;
				}

				if (NULL != field) /* string key: field => val */
				{
					char optionText[300];
					int optionTextLen;
					int hasRows = zend_hash_num_elements(Z_ARRVAL_P(zparam_OPTIONS)) > 1;

					/* "field EQ 'val'" */
					optionTextLen = snprintf(optionText, sizeof(optionText), "%s%s EQ '%s'", hasRows ? "AND " : "", field, Z_STRVAL_P(val));

#if PHP_VERSION_ID >= 70000
					ZVAL_STRINGL(ztextfield, optionText, optionTextLen);
#else
					ZVAL_STRINGL(ztextfield, optionText, optionTextLen, 1);
#endif
				}
				else { /* custom WHERE clause, append 'as is' */
					ZVAL_ZVAL(ztextfield, val, 1, 0);
				}

				if (UNEXPECTED(val == pzrealval)) {
					my_zval_ptr_dtor(pzrealval);
				}
			}
			MY_ZEND_HASH_FOREACH_END();
		}
	}

	/* Call RFC_READ_TABLE */
	{
		int result;
		zval *pzresult;
#if PHP_VERSION_ID >= 70000
		zval zresult;

		pzresult = &zresult;

		result = sap_call_object_method(getThis(), sap_ce_SapFunction, "__invoke", NULL, &__invoke_args, pzresult);
#else
		result = sap_call_object_method(getThis(), sap_ce_SapFunction, "__invoke", NULL, &__invoke_args, &pzresult TSRMLS_CC);
#endif
		zval_dtor(&__invoke_args);
		RETVAL_FALSE;

		/* Convert the DATA table to a PHP readable array */
		if (result == SUCCESS && Z_TYPE_P(pzresult) == IS_ARRAY)
		{
			zval *zresultdata;
			zval *zresultfields;
			zval *zdatarow;

			if ((zresultdata = my_zend_hash_find_zval(Z_ARRVAL_P(pzresult), "DATA", sizeof("DATA") - 1)) && Z_TYPE_P(zresultdata) == IS_ARRAY
			&&	(zresultfields = my_zend_hash_find_zval(Z_ARRVAL_P(pzresult), "FIELDS", sizeof("FIELDS") - 1)) && Z_TYPE_P(zresultfields) == IS_ARRAY)
			{
				array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(zresultdata)));

				MY_ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(zresultdata), zdatarow)
				{
					zval *zresultrow;
					zval *wa;
					zval *zfieldrow;

					wa = my_zend_hash_find_zval(Z_ARRVAL_P(zdatarow), "WA", sizeof("WA") - 1);

					/* Fix bug: wa might be NULL also */
					if (NULL == wa || ( Z_TYPE_P(wa) != IS_STRING && Z_TYPE_P(wa) != IS_NULL) ) {
						continue;
					}

					zresultrow = my_zend_hash_next_index_insert_new_zval(Z_ARRVAL_P(return_value));

					if (NULL == zresultrow) {
						continue;
					}

					array_init_size(zresultrow, zend_hash_num_elements(Z_ARRVAL_P(zresultfields)));

					MY_ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(zresultfields), zfieldrow)
					{
						zval *zfieldname, *zfieldoffset, *zfieldlength, *zfieldtype;
						zval zlongoffset, zlonglength;

						if (Z_TYPE_P(zfieldrow) != IS_ARRAY) {
							continue; /* This should not happen */
						}

						if ((zfieldname = my_zend_hash_find_zval(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1)) && Z_TYPE_P(zfieldname) == IS_STRING
						&&	(zfieldoffset = my_zend_hash_find_zval(Z_ARRVAL_P(zfieldrow), "OFFSET", sizeof("OFFSET") - 1))
						&&	(zfieldlength = my_zend_hash_find_zval(Z_ARRVAL_P(zfieldrow), "LENGTH", sizeof("LENGTH") - 1))
						&&	(zfieldtype = my_zend_hash_find_zval(Z_ARRVAL_P(zfieldrow), "TYPE", sizeof("TYPE") - 1)) && Z_TYPE_P(zfieldtype) == IS_STRING)
						{
							zval *zfieldvalue;
							zval *zfieldalias;

							ZVAL_ZVAL(&zlongoffset, zfieldoffset, 1, 0);
							convert_to_long(&zlongoffset);

							ZVAL_ZVAL(&zlonglength, zfieldlength, 1, 0);
							convert_to_long(&zlonglength);

							if (Z_TYPE_P(zfields) == IS_ARRAY && (zfieldalias = my_zend_hash_find_zval(Z_ARRVAL_P(zfields), Z_STRVAL_P(zfieldname), Z_STRLEN_P(zfieldname)))) {
								/* is field alias */
								zfieldname = zfieldalias;
							}

							if (zfieldvalue = my_zend_hash_add_new_zval(Z_ARRVAL_P(zresultrow), Z_STRVAL_P(zfieldname), Z_STRLEN_P(zfieldname)))
							{
								char *substr;
								int substrlen;

								if (Z_TYPE_P(wa) == IS_STRING && SUCCESS == utf8_substr(Z_STRVAL_P(wa), Z_STRLEN_P(wa), Z_LVAL(zlongoffset), Z_LVAL(zlonglength), &substr, &substrlen))
								{
									if (PHP_SAP_GLOBALS(rtrim_export_strings))
									{
										char *substrtrimmed;
										int substrtrimmedlen;

										utf8_trim(substr, substrlen, &substrtrimmed, &substrtrimmedlen, TRIM_RIGHT);

										efree(substr);
										substr = substrtrimmed;
										substrlen = substrtrimmedlen;
									}
#if PHP_VERSION_ID >= 70000
									ZVAL_STRINGL(zfieldvalue, substr, substrlen);
									efree(substr);
#else
									ZVAL_STRINGL(zfieldvalue, substr, substrlen, 0);
#endif
								}
								else {
									ZVAL_NULL(zfieldvalue);
								}
							}
						}
					}
					MY_ZEND_HASH_FOREACH_END();
				}
				MY_ZEND_HASH_FOREACH_END();
			}
		}

		if (result == SUCCESS) {
			my_zval_ptr_dtor(pzresult);
		}
	}
}

PHP_MINIT_FUNCTION(sap)
{
	zend_class_entry ce;

	/* instatiate last error to NULL*/
	memset(&sap_last_error, 0, sizeof(SAPRFC_ERROR_INFO));

#if PHP_SAP_WITH_PTHREADS
	/* initialize conversion mutexes */
	rfc_utf8_to_sapuc_mutex = PTHREAD_MUTEX_INITIALIZER;
	rfc_sapuc_to_utf8_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

	/* Register resources */
	le_php_sap_connection = zend_register_list_destructors_ex(php_sap_connection_rsrc_dtor, NULL, PHP_SAP_CONNECTION_RES_NAME, module_number);

	ZEND_INIT_MODULE_GLOBALS(sap, ZEND_MODULE_GLOBALS_CTOR_N(sap), NULL);

	REGISTER_INI_ENTRIES();

	/* init zend_default_exception global variable */
#if PHP_VERSION_ID < 70000
	zend_default_exception = zend_exception_get_default(TSRMLS_C);
#else
	zend_default_exception = zend_ce_exception;
#endif

	/* Define Sap classes */

	//SapException
	INIT_CLASS_ENTRY(ce, "SapException", sap_exception_fe);
#if PHP_VERSION_ID < 70000
	sap_ce_SapException = zend_register_internal_class_ex(&ce, zend_default_exception, "Exception" TSRMLS_CC);
#else
	sap_ce_SapException = zend_register_internal_class_ex(&ce, zend_default_exception);
#endif
	
	zend_declare_property_string(sap_ce_SapException, "MSGTY",			sizeof("MSGTY") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "MSGID",			sizeof("MSGID") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "MSGNO",			sizeof("MSGNO") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "MSGV1",			sizeof("MSGV1") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "MSGV2",			sizeof("MSGV2") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "MSGV3",			sizeof("MSGV3") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "MSGV4",			sizeof("MSGV4") - 1,		"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "KEY",			sizeof("KEY") - 1,			"", ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "nwsdkfunction",	sizeof("nwsdkfunction") - 1,"", ZEND_ACC_PROTECTED TSRMLS_CC);

	//Sap
	INIT_CLASS_ENTRY(ce, "Sap", sap_fe_Sap);
	sap_ce_Sap = zend_register_internal_class(&ce TSRMLS_CC);

	sap_ce_Sap->create_object = sap_object_create_object;

	memcpy(&sap_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sap_object_handlers.clone_obj = sap_object_clone_object;
	
#if PHP_VERSION_ID >= 70000
	sap_object_handlers.free_obj = sap_object_free_object_storage;
	sap_object_handlers.offset = XtOffsetOf(sap_object, std);
#endif
	
	//SapFunction
	INIT_CLASS_ENTRY(ce, "SapFunction", sap_fe_SapFunction);
	sap_ce_SapFunction = zend_register_internal_class(&ce TSRMLS_CC);

	sap_ce_SapFunction->create_object = sap_function_create_object;

	memcpy(&sap_function_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sap_function_handlers.clone_obj = sap_function_clone_object;
	sap_function_handlers.cast_object = sap_function_cast_object;

#if PHP_VERSION_ID >= 70000
	sap_function_handlers.free_obj = sap_function_free_object_storage;
	sap_function_handlers.offset = XtOffsetOf(sap_function, std);
#endif
	
	//SapRfcReadTable
	INIT_CLASS_ENTRY(ce, "SapRfcReadTable", sap_fe_SapRfcReadTable);
#if PHP_VERSION_ID < 70000
	sap_ce_SapRfcReadTable = zend_register_internal_class_ex(&ce, sap_ce_SapFunction, "SapFunction" TSRMLS_CC);
#else
	sap_ce_SapRfcReadTable = zend_register_internal_class_ex(&ce, sap_ce_SapFunction);
#endif
	
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(sap)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(sap)
{
	unsigned int major, minor, patchlevel;
	const SAP_UC *sdkversionU;
	char *nwsdkversion;
	int nwsdkversionlen, res;

	php_info_print_table_start();
	php_info_print_table_header(2, "SAP Remote Functions call support", "enabled");
	php_info_print_table_row(2, "Version", PHP_SAP_VERSION);

	sdkversionU = RfcGetVersion(&major, &minor, &patchlevel);
	res = sapuc_to_utf8((SAP_UC*)sdkversionU, &nwsdkversion, &nwsdkversionlen, &sap_last_error);

	if (SUCCESS == res) {
		php_info_print_table_row(2, "Nw Rfc Sdk Version", nwsdkversion);
		efree(nwsdkversion);
	}

	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}