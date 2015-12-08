#include "php_sap.h"

#include "php_ini.h"
#include "ext\standard\info.h"

#include "zend_objects.h"
#include "zend_exceptions.h"


#if PHP_VERSION_ID < 70000
	#define zend_default_exception zend_exception_get_default(TSRMLS_C)
#else
	#define zend_default_exception zend_ce_exception
#endif

#ifdef HAVE_SPL
	#include "ext\spl\spl_exceptions.h"
	#include "ext\spl\spl_iterators.h"

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
	__err->errFunctionLen = __funclen;							\
	memcpy(__err->errFunction, __func, __funclen);				\
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

#define SAP_HASH_FOREACH(ht) do {													\
	HashTable *__ht = (ht);															\
	HashPosition __pos;																\
	void **__data;																	\
	for (zend_hash_internal_pointer_reset_ex(__ht, &__pos);							\
		 zend_hash_get_current_data_ex(__ht, (void**)&__data, &__pos) == SUCCESS;	\
		 zend_hash_move_forward_ex(__ht, &__pos))									\
	{

#define SAP_HASH_FOREACH_VAL(ht, _zval) SAP_HASH_FOREACH(ht) _zval = *__data;
#define SAP_HASH_FOREACH_KEY_VAL(ht, _h, _key, _keylen, _zval) SAP_HASH_FOREACH_VAL(ht, _zval) _h = __pos->h; _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define SAP_HASH_FOREACH_STR_KEY(ht, _key, _keylen) SAP_HASH_FOREACH(ht) _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define SAP_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _zval) SAP_HASH_FOREACH_VAL(ht, _zval) _key = __pos->nKeyLength ? (char*)__pos->arKey : NULL; _keylen = __pos->nKeyLength - 1;
#define SAP_HASH_FOREACH_STR_KEY_PTR(ht, _key, _keylen, _ptr) SAP_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _ptr);

#define sap_read_object_property(_object, _prop, _scope) zend_read_property(_scope, _object, _prop, strlen(_prop) + 1, 0 TSRMLS_CC)
#define sap_read_object_property_ex(_object, _prop, _proplen, _scope) zend_read_property(_scope, _object, _prop, _proplen, 0 TSRMLS_CC)

#define sap_fetch_connection_rsrc(_object) zend_fetch_resource(&_object TSRMLS_CC, -1, PHP_SAP_CONNECTION_RES_NAME, NULL, 1, le_php_sap_connection)

#if PHP_VERSION_ID < 50400
#define sap_make_resource(_zval, _ptr, _rsrc_id) zend_register_resource(_zval, _ptr, _rsrc_id)
#else
#define sap_make_resource(_zval, _ptr, _rsrc_id) zend_register_resource(_zval, _ptr, _rsrc_id TSRMLS_CC)
#endif

#define sap_get_str_val(_str) (char*)(_str)

#define sap_zval_ptr_dtor(_pzval) zval_ptr_dtor(&(_pzval))

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

#define SAP_HASH_FOREACH(ht) do {							\
	HashTable *__ht = (ht);									\
	HashPosition __pos;										\
	zval *_z;												\
	for (zend_hash_internal_pointer_reset_ex(__ht, &__pos);	\
		 _z = zend_hash_get_current_data_ex(__ht, &__pos);	\
		 zend_hash_move_forward_ex(__ht, &__pos))			\
	{														\
		uint32_t _idx = __pos;								\
		Bucket *_p = __ht->arData + _idx;

#define SAP_HASH_FOREACH_VAL(ht, _zval) SAP_HASH_FOREACH(ht) _zval = _z;
#define SAP_HASH_FOREACH_KEY_VAL(ht, _h, _key, _keylen, _zval) SAP_HASH_FOREACH_VAL(ht, _zval) _h = _p->h; _key = _p->key ? _p->key->val : NULL; _keylen = _p->key ? _p->key->len : 0;
#define SAP_HASH_FOREACH_STR_KEY(ht, _key, _keylen) SAP_HASH_FOREACH(ht) _key = _p->key ? _p->key->val : NULL; _keylen = _p->key ? _p->key->len : 0;
#define SAP_HASH_FOREACH_STR_KEY_VAL(ht, _key, _keylen, _zval) SAP_HASH_FOREACH_VAL(ht, _zval) _key = _p->key ? _p->key->val : NULL; _keylen = _p->key ? _p->key->len : 0;
#define SAP_HASH_FOREACH_STR_KEY_PTR(ht, _key, _keylen, _ptr) SAP_HASH_FOREACH_STR_KEY(ht, _key, _keylen) _ptr = Z_PTR_P(_z);

#define sap_read_object_property(_object, _prop, _scope) zend_read_property(_scope, _object, _prop, strlen(_prop) + 1, 0, NULL)
#define sap_read_object_property_ex(_object, _prop, _proplen, _scope) zend_read_property(_scope, _object, _prop, _proplen, 0, NULL)

#define sap_fetch_connection_rsrc(_object) zend_fetch_resource(Z_RES_P(_object), PHP_SAP_CONNECTION_RES_NAME, le_php_sap_connection)

#define sap_make_resource(_zval, _ptr, _rsrc_id) ZVAL_RES(_zval, zend_register_resource(_ptr, _rsrc_id))

#define sap_get_str_val(_zstr) (char*)ZSTR_VAL(_zstr)

#define sap_zval_ptr_dtor(_pzval) zval_ptr_dtor(_pzval)

#endif

#define SAP_HASH_FOREACH_END()								\
	}														\
} while(0)

typedef enum _TRIM_TYPE {
	TRIM_LEFT = 0x01,
	TRIM_RIGHT = 0x02,
	TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
} TRIM_TYPE;

typedef struct _SAPRFC_ERROR_INFO {
	RFC_ERROR_INFO err;
	char errFunction[30];
	unsigned int errFunctionLen;
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
const zend_function_entry sap_exception_fe[] = {
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
	PHP_ME(SapRfcReadTable,	getName,		NULL,										ZEND_ACC_PUBLIC)
	PHP_ME(SapRfcReadTable,	select,			SAP_ME_ARGS(SapRfcReadTable, select),		ZEND_ACC_PUBLIC)
	PHP_FE_END
};
/**	Class methods definition -- end		**/

/* }}} */

static int sap_import(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err TSRMLS_DC);

static int sap_export(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *rv, SAPRFC_ERROR_INFO *err TSRMLS_DC);

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

static int utf8_to_sapuc(char *str, int len, SAP_UC **uc, unsigned int *uc_len, SAPRFC_ERROR_INFO *err)
{
#if FALSE /* Option 1: let the NW library decide the appropriate buffer length we need for the conversion */
	SAP_UC *retval = NULL;
	unsigned int uclen = 0;
	size_t ucsize;

try_again:
	/* +1 'cause the nw library will make string null-terminated */
	ucsize = (uclen + 1) * sizeof(SAP_UC);
	
	retval = *uc = emalloc(ucsize);
	memset(retval, 0, ucsize);

	switch (RfcUTF8ToSAPUC((RFC_BYTE*)str, len, retval, &uclen, uc_len, (RFC_ERROR_INFO*)err))
	{
		case RFC_BUFFER_TOO_SMALL:
			/* In case of RFC_BUFFER_TOO_SMALL the NW library fills 'uclen' with the required SAP_UC characters (not include the last null terminating SAP_UC char) */
			efree(retval);
			goto try_again;
		case RFC_OK:
			return SUCCESS;
		default:
			efree(retval);
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcUTF8ToSAPUC", FAILURE);
	}
#else /* Option 2: We need exactly 'utf8-len' + 1 SAP_UC chars */
	SAP_UC *retval;
	unsigned int uclen = strlenU8(str, len) + 1;
	size_t ucsize = uclen * sizeof(SAP_UC);

	retval = *uc = emalloc(ucsize);
	memset(retval, 0, ucsize);

	if (RFC_OK == RfcUTF8ToSAPUC((RFC_BYTE*)str, len, retval, &uclen, uc_len, (RFC_ERROR_INFO*)err)) {
		return SUCCESS;
	}

	efree(retval);

	SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcUTF8ToSAPUC", FAILURE);
#endif
}

static int sapuc_to_utf8_l(SAP_UC *strU16, unsigned int strU16len, char **strU8, int *strU8len, SAPRFC_ERROR_INFO *err)
{
	char *utf8;
	unsigned int utf8len = 0;

try_again:
	utf8 = emalloc(utf8len + 1); /* +1 because the nw library will make output null-terminated */
	memset(utf8, 0, utf8len + 1);

	switch (RfcSAPUCToUTF8(strU16, strU16len, (RFC_BYTE*)utf8, &utf8len, strU8len, (RFC_ERROR_INFO*)err))
	{
		case RFC_BUFFER_TOO_SMALL: {
			/* In case of RFC_BUFFER_TOO_SMALL the NW library fills 'utf8len' with the required number of bytes for convertion (not including the last null terminating byte) */
			efree(utf8);
			goto try_again;
		}
		case RFC_OK: {
			*strU8 = utf8;
			return SUCCESS;
		}
		default: {
			efree(utf8);
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
	zval args;
	zval *format_arg;
	int call_result;
#if PHP_VERSION_ID < 70000
	zval *return_value_ptr = NULL;

	array_init(&args);

	MAKE_STD_ZVAL(format_arg);
	ZVAL_STRING(format_arg, format, 1);

	zend_hash_next_index_insert(Z_ARRVAL(args), &format_arg, sizeof(zval*), NULL);

	call_result = sap_call_object_method(object, Z_OBJCE_P(object), "format", NULL, &args, &return_value_ptr TSRMLS_CC);

	if (call_result == SUCCESS) {
		ZVAL_ZVAL(return_value, return_value_ptr, 1, 1);
	}
#else
	array_init(&args);

	if (NULL != (format_arg = zend_hash_next_index_insert(Z_ARRVAL(args), &EG(uninitialized_zval)))) {
		ZVAL_STRING(&args, format);
	}

	call_result = sap_call_object_method(object, Z_OBJCE_P(object), "format", NULL, &args, return_value);
#endif
	zval_dtor(&args);

	return call_result;
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
	char *key, *msgType, *msgNumber, *msgv1, *msgv2, *msgv3, *msgv4, *message;
	int keyLen, msgTypeLen, msgNumberLen, msgv1Len, msgv2Len, msgv3Len, msgv4Len, messageLen;
	SAPRFC_ERROR_INFO e;

	object_init_ex(exception, sap_ce_SapException);

	if (SUCCESS == sapuc_to_utf8(err->err.key, &key, &keyLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "key", sizeof("key") - 1, key, keyLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgType, &msgType, &msgTypeLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "msgClass", sizeof("msgClass") - 1, msgType, msgTypeLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgNumber, &msgNumber, &msgNumberLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "msgNumber", sizeof("msgNumber") - 1, msgNumber, msgNumberLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.message, &message, &messageLen, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "message", sizeof("message") - 1, message, messageLen TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV1, &msgv1, &msgv1Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "msgv1", sizeof("msgv1") - 1, msgv1, msgv1Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV2, &msgv2, &msgv2Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "msgv2", sizeof("msgv2") - 1, msgv2, msgv2Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV3, &msgv3, &msgv3Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "msgv3", sizeof("msgv3") - 1, msgv3, msgv3Len TSRMLS_CC);
	}

	if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV4, &msgv4, &msgv4Len, &e)) {
		zend_update_property_stringl(sap_ce_SapException, exception, "msgv4", sizeof("msgv4") - 1, msgv4, msgv1Len TSRMLS_CC);
	}

	if (err->errFunctionLen > 0) {
		zend_update_property_stringl(sap_ce_SapException, exception, "rfcFunction", sizeof("rfcFunction") - 1, err->errFunction, err->errFunctionLen TSRMLS_CC);
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

static void * sap_hash_find_ptr(HashTable *ht, char *key, int keylen)
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

static zval * sap_hash_find_zval(HashTable *ht, char *key, int keylen)
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

static void * sap_hash_update_ptr(HashTable *ht, char *key, int keylen, void *ptr, size_t size)
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

static zval * sap_hash_update_zval(HashTable *ht, char *key, int keylen, zval *z)
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

static void * sap_hash_add_ptr(HashTable *ht, char *key, int keylen, void *ptr, size_t size)
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

static zval * sap_hash_add_zval(HashTable *ht, char *key, int keylen, zval *z)
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

static zval * sap_hash_add_new_zval(HashTable *ht, char *key, int keylen)
{
#if PHP_VERSION_ID < 70000
	zval *znewentry = NULL;

	MAKE_STD_ZVAL(znewentry);

	if (znewentry && !(znewentry = sap_hash_add_zval(ht, key, keylen, znewentry))) {
		zval_ptr_dtor(&znewentry);
	}

	return znewentry;
#else
	return zend_hash_str_add(ht, key, keylen, &EG(uninitialized_zval));
#endif
}

static void * sap_hash_next_index_insert_ptr(HashTable *ht, void *ptr, size_t size)
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

static zval * sap_hash_next_index_insert_zval(HashTable *ht, zval *z)
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

static zval * sap_hash_next_index_insert_new_zval(HashTable *ht)
{
#if PHP_VERSION_ID < 70000
	zval *znewentry = NULL;

	MAKE_STD_ZVAL(znewentry);

	if (znewentry && !(znewentry = sap_hash_next_index_insert_zval(ht, znewentry))) {
		zval_ptr_dtor(&znewentry);
	}

	return znewentry;
#else
	return zend_hash_next_index_insert(ht, &EG(uninitialized_zval));
#endif
}

static HashTable * sap_function_description_to_array(RFC_FUNCTION_DESC_HANDLE fdh, SAPRFC_ERROR_INFO *err)
{
	HashTable *retval;
	unsigned int paramCount, i;

	if (RFC_OK != RfcGetParameterCount(fdh, &paramCount, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetParameterCount", NULL);
	}

	ALLOC_HASHTABLE(retval);
	zend_hash_init(retval, paramCount, NULL, SAPRFC_PARAMETER_PTR_DTOR, 0);

	for (i = 0; i < paramCount; i++)
	{
		SAPRFC_PARAMETER_DESC *sp;
		char *paramName;
		int paramNameLen;

		sp = emalloc(sizeof(SAPRFC_PARAMETER_DESC));
		memset(sp, 0, sizeof(SAPRFC_PARAMETER_DESC));

		if (RFC_OK != RfcGetParameterDescByIndex(fdh, i, &sp->param, (RFC_ERROR_INFO*)err)) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetParameterDescByIndex", NULL);
		}

		sp->state = SAPRFC_PARAM_DEFAULT;

		if (SUCCESS != sapuc_to_utf8(sp->param.name, &paramName, &paramNameLen, err)) {
			return NULL;
		}

		sap_hash_add_ptr(retval, paramName, paramNameLen, sp, sizeof(SAPRFC_PARAMETER_DESC*));

		efree(paramName);
	}

	return retval;
}

static void sap_connection_set_parameters(php_sap_connection *connection, HashTable *lparams)
{
	char *lparam;
	int lparamlen;
	zval *zlpvalue;

	zend_hash_clean(connection->lparams);

	SAP_HASH_FOREACH_STR_KEY_VAL(lparams, lparam, lparamlen, zlpvalue)
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
		if (UNEXPECTED(Z_TYPE_P(zlpvalue) != IS_STRING))
		{
#if PHP_VERSION_ID < 70000
			MAKE_STD_ZVAL(pcopy);
#endif
			ZVAL_ZVAL(pcopy, zlpvalue, 1, 0);
			convert_to_string(pcopy);
			zlpvalue = pcopy;
		}

		if (z = sap_hash_add_zval(connection->lparams, lparam, lparamlen, zlpvalue)) {
			Z_ADDREF_P(z);
		}

		if (UNEXPECTED(zlpvalue == pcopy)) {
			sap_zval_ptr_dtor(pcopy);
		}
	}
	SAP_HASH_FOREACH_END();
}

static php_sap_connection * sap_create_connection_resource(HashTable *logonParameters)
{
	php_sap_connection *retval;

	retval = emalloc(sizeof(php_sap_connection));
	memset(retval, 0, sizeof(php_sap_connection));

	ALLOC_HASHTABLE(retval->lparams);
	zend_hash_init(retval->lparams, 0, NULL, ZVAL_PTR_DTOR, 0);

	if (NULL != logonParameters) {
		sap_connection_set_parameters(retval, logonParameters);
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

static php_sap_function * sap_create_function(void)
{
	php_sap_function *retval;

	retval = emalloc(sizeof(php_sap_function));
	memset(retval, 0, sizeof(php_sap_function));

	return retval;
}

static php_sap_function * sap_create_function_resource(RFC_FUNCTION_DESC_HANDLE fdh, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
	php_sap_function *retval;
	unsigned int paramCount = 0;

	retval = sap_create_function();
	
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
	
	retval = sap_function_create_object_ex(Z_OBJCE_P(object), &clone_intern TSRMLS_CC);

	/* Clone Function Parameters */
	if (intern->function_descr)
	{
		clone_intern->function_descr = sap_create_function_resource(intern->function_descr->fdh, &sap_last_error TSRMLS_CC);

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

	do {
		char *lparam;
		unsigned int lparamlen;
		zval *lpvalue;

		SAP_HASH_FOREACH_STR_KEY_VAL(connection->lparams, lparam, lparamlen, lpvalue)
		{
			if (NULL == lparam || Z_TYPE_P(lpvalue) != IS_STRING) {
				continue;
			}

			if (SUCCESS != utf8_to_sapuc(Z_STRVAL_P(lpvalue), Z_STRLEN_P(lpvalue), (SAP_UC**)&p->value, &l, err)) {
				return FAILURE;
			}

			if (SUCCESS != utf8_to_sapuc(lparam, lparamlen, (SAP_UC**)&p->name, &l, err)) {
				return FAILURE;
			}

			p++;
		}
		SAP_HASH_FOREACH_END();
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

				if (SUCCESS != utf8_to_sapuc(Z_STRVAL(rv), Z_STRLEN(rv), &valU, &valUlen, err)) {
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

			if (SUCCESS != utf8_to_sapuc(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), &strU, &strU16len, err)) {
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

		if (zfvalue = sap_hash_find_zval(fields, fname, fnamelen)) {
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

	SAP_HASH_FOREACH_VAL(rows, zrow)
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
	SAP_HASH_FOREACH_END();

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
		pfval = sap_hash_add_zval(Z_ARRVAL_P(rv), fname, fnamelen, pfval);

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

		if (!(pzrow = sap_hash_next_index_insert_new_zval(Z_ARRVAL_P(rv)))) { /* Out of memory, full hashtable etc... */ 
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
	
	if (SUCCESS != utf8_to_sapuc(name, nameLen, &nameU, &nameUlen, err)) {
		return NULL;
	}

	fdh = RfcGetFunctionDesc(connection->handle, nameU, (RFC_ERROR_INFO*)err);
	efree(nameU);

	if (NULL == fdh) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcGetFunctionDesc", NULL);
	}
	
	return sap_create_function_resource(fdh, err TSRMLS_CC);
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

	SAP_HASH_FOREACH_STR_KEY_PTR(function->params, pname, pnamelen, sp)
	{
		zval *zpvalue;

		if (sp->state != SAPRFC_PARAM_DEFAULT && RFC_OK != RfcSetParameterActive(fh, sp->param.name, sp->state, (RFC_ERROR_INFO*)err)) {
			SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcSetParameterActive", FAILURE);
		}

		if (NULL == imports || !(sp->param.direction & RFC_IMPORT) || sp->state == SAPRFC_PARAM_INACTIVE) {
			continue;
		}

		if (!(zpvalue = sap_hash_find_zval(imports, pname, pnamelen))) {
			continue;
		}
		
		if (SUCCESS != sap_import(fh, sp->param.name, sp->param.type, sp->param.typeDescHandle, sp->param.nucLength, zpvalue, err TSRMLS_CC)) {
			return FAILURE;
		}
	}
	SAP_HASH_FOREACH_END();
	
	if (RFC_OK != RfcInvoke(connection->handle, fh, (RFC_ERROR_INFO*)err)) {
		SAP_ERROR_SET_FUNCTION_AND_RETURN(err, "RfcInvoke", FAILURE);
	}
	
	SAP_HASH_FOREACH_STR_KEY_PTR(function->params, pname, pnamelen, sp)
	{
		zval *ev = NULL;

		if (!(sp->param.direction & RFC_EXPORT) || sp->state == SAPRFC_PARAM_INACTIVE) {
			continue;
		}

#if PHP_VERSION_ID < 70000
		MAKE_STD_ZVAL(ev);
#else
		ev = &EG(uninitialized_zval);
#endif
		if (!(ev = sap_hash_add_zval(exports, pname, pnamelen, ev))) { /* out of memory, full hashtable etc */
			break;
		}

		if (SUCCESS != sap_export(fh, sp->param.name, sp->param.type, sp->param.typeDescHandle, sp->param.nucLength, ev, err TSRMLS_CC)) {
			return FAILURE;
		}
	}
	SAP_HASH_FOREACH_END();
	
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
	char *key, *msgType, *msgNumber, *msgv1, *msgv2, *msgv3, *msgv4, *message;
	int keyLen, msgTypeLen, msgNumberLen, msgv1Len, msgv2Len, msgv3Len, msgv4Len, messageLen;
	SAPRFC_ERROR_INFO e;

	array_init(return_value);

	add_assoc_long(return_value, "code", sap_last_error.err.code);

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.key, &key, &keyLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "key", key, keyLen);
		efree(key);
#else
		add_assoc_stringl(return_value, "key", key, keyLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgType, &msgType, &msgTypeLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "msgType", msgType, msgTypeLen);
		efree(msgType);
#else
		add_assoc_stringl(return_value, "msgType", msgType, msgTypeLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgNumber, &msgNumber, &msgNumberLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "msgNumber", msgNumber, msgNumberLen);
		efree(msgNumber);
#else
		add_assoc_stringl(return_value, "msgNumber", msgNumber, msgNumberLen, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV1, &msgv1, &msgv1Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "msgv1", msgv1, msgv1Len);
		efree(msgv1);
#else
		add_assoc_stringl(return_value, "msgv1", msgv1, msgv1Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV2, &msgv2, &msgv2Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "msgv2", msgv2, msgv2Len);
		efree(msgv2);
#else
		add_assoc_stringl(return_value, "msgv2", msgv2, msgv2Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV3, &msgv3, &msgv3Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "msgv3", msgv3, msgv3Len);
		efree(msgv3);
#else
		add_assoc_stringl(return_value, "msgv3", msgv3, msgv3Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.abapMsgV4, &msgv4, &msgv4Len, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "msgv4", msgv4, msgv4Len);
		efree(msgv4);
#else
		add_assoc_stringl(return_value, "msgv4", msgv4, msgv4Len, 0);
#endif
	}

	if (SUCCESS == sapuc_to_utf8(sap_last_error.err.message, &message, &messageLen, &e)) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "message", message, messageLen);
		efree(message);
#else
		add_assoc_stringl(return_value, "message", message, messageLen, 0);
#endif
	}

	if (sap_last_error.errFunctionLen > 0) {
#if PHP_VERSION_ID >= 70000
		add_assoc_stringl(return_value, "rfcFunction", sap_last_error.errFunction, sap_last_error.errFunctionLen);
#else
		add_assoc_stringl(return_value, "rfcFunction", sap_last_error.errFunction, sap_last_error.errFunctionLen, 1);
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

PHP_METHOD(Sap, __construct)
{
	sap_object *intern;
	int res;
	zval *zlogonParameters = NULL;
	HashTable *logonParameters = NULL;
	int autoConnect = 1;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "|ab", &zlogonParameters, &autoConnect);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		return;
	}

	if (NULL != zlogonParameters) {
		logonParameters = Z_ARRVAL_P(zlogonParameters);
	}

	intern = sap_get_sap_object(getThis());

	if (intern->connection) { /* Called constructor again?? */
		php_sap_connection_ptr_dtor(intern->connection);
	}

	intern->connection = sap_create_connection_resource(logonParameters);
	intern->connection->refCount++;

	if (NULL != zlogonParameters && autoConnect && SUCCESS != sap_connection_open(intern->connection, &sap_last_error)) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
	}
}

PHP_METHOD(Sap, setFunctionClass)
{
	int res;
	zend_class_entry *fce = sap_ce_SapFunction;
	sap_object *intern = sap_get_sap_object(getThis());

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "C", &fce);
	PHP_SAP_PARSE_PARAMS_END();

	if (FAILURE == res) {
		return;
	}

	intern->func_ce = fce;

	RETURN_ZVAL(getThis(), 1, 0);
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

	if (NULL == (func = sap_fetch_function(name, namelen, intern->connection, &sap_last_error TSRMLS_CC))) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
	}

	if (NULL != zimports) {
		imports = Z_ARRVAL_P(zimports);
	}

	array_init(return_value);

	cresult = sap_function_invoke(func, intern->connection, imports, Z_ARRVAL_P(return_value), &sap_last_error TSRMLS_CC);

	php_sap_function_ptr_dtor(func);

	if (SUCCESS != cresult) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
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
	php_sap_function *func_descr;
	sap_function *func;

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
				zval *retval_ptr;
#if PHP_VERSION_ID >= 70000
				zval rv;

				retval_ptr = &rv;

				/* all functions/methods are stored in lower case */
				if (SUCCESS != sap_call_object_method(zfunction, Z_OBJCE_P(zfunction), "getname", NULL, NULL, retval_ptr TSRMLS_CC)) {
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

				sap_zval_ptr_dtor(retval_ptr);

				break;
			}
		default:
		{
			sap_throw_exception("Argument 1 of Sap::fetchFunction() must be a string or a SapFunction object", -1, zend_invalid_args_exception);
			RETURN_FALSE;
		}
	}

	intern = sap_get_sap_object(getThis());

	/* Get function description from SAP backend */
	func_descr = sap_fetch_function(function_name, function_len, intern->connection, &sap_last_error TSRMLS_CC);
	efree(function_name);

	if (NULL == func_descr) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
		RETURN_FALSE;
	}

	if (Z_TYPE_P(zfunction) == IS_OBJECT) {
		RETVAL_ZVAL(zfunction, 1, 0);
	}
	else {
		if (ZEND_NUM_ARGS() < 2) {
			fce = intern->func_ce;
		}
		/* Create SapFunction object */
		object_init_ex(return_value, fce);
	}
	
	/* Call object's constructor, if exists */
	if (Z_TYPE_P(zfunction) != IS_OBJECT && fce->constructor)
	{
		int retval;
#if PHP_VERSION_ID >= 70000
		zval rv;

		retval = sap_call_object_method(return_value, fce, ZSTR_VAL(fce->constructor->common.function_name), fce->constructor, zargs, &rv TSRMLS_CC);
		/* we are not interested in constructor's return value*/
		zval_ptr_dtor(&rv);
#else
		zval *rv_ptr;

		retval = sap_call_object_method(return_value, fce, fce->constructor->common.function_name, fce->constructor, zargs, &rv_ptr TSRMLS_CC);
		/* we are not interested in constructor's return value*/
		zval_ptr_dtor(&rv_ptr);
#endif
		
		if (SUCCESS != retval) {
			php_error(E_ERROR, "Could not call '%s' object's constructor", sap_get_str_val(fce->name));
			RETURN_FALSE;
		}
	}

	func = sap_get_function(return_value);

	if (UNEXPECTED(NULL != func->function_descr)) {
		php_sap_function_ptr_dtor(func->function_descr);
	}

	func->function_descr = func_descr;

	if (UNEXPECTED(NULL != func->connection)) {
		php_sap_connection_ptr_dtor(func->connection);
	}

	func->connection = intern->connection;
	func->connection->refCount++;
}

PHP_METHOD(SapFunction, getName)
{
	sap_function *intern;
	RFC_ABAP_NAME funcNameU;
	char *name;
	int namelen;

	intern = sap_get_function(getThis());

	if (NULL == intern->function_descr) {
#if PHP_VERSION_ID >= 70000
		RETURN_STRING("");
#else
		RETURN_STRING("", 0);
#endif
	}

	if (RFC_OK != RfcGetFunctionName(intern->function_descr->fdh, funcNameU, (RFC_ERROR_INFO*)&sap_last_error)) {
		SAP_ERROR_SET_RFCFUNCTION(&sap_last_error, "RfcGetFunctionName", sizeof("RfcGetFunctionName") - 1);
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
	}

	if (SUCCESS != sapuc_to_utf8(funcNameU, &name, &namelen, &sap_last_error)) {
		SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
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

	if (sp = sap_hash_find_ptr(intern->function_descr->params, pname, pnamelen)) {
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

	if (SUCCESS != sap_function_invoke(intern->function_descr, intern->connection, imports, Z_ARRVAL_P(return_value), &sap_last_error TSRMLS_CC)) {
		SAPRFC_ERROR_INFO *err = &sap_last_error;

		zval_dtor(return_value);

		SAP_THROW_SAPRFC_ERROR_EXCEPTION(err);
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

	SAP_HASH_FOREACH_STR_KEY(intern->function_descr->params, pname, pnamelen)
	{
#if PHP_VERSION_ID >= 70000
		add_next_index_stringl(return_value, pname, pnamelen);
#else
		add_next_index_stringl(return_value, pname, pnamelen, 1);
#endif
	}
	SAP_HASH_FOREACH_END();
}

PHP_METHOD(SapFunction, getTypeName)
{
	int res;
	char *pname;
	int pnamelen;
	sap_function *intern;
	SAPRFC_PARAMETER_DESC *sp;

	PHP_SAP_PARSE_PARAMS_BEGIN()
		res = PHP_SAP_PARSE_PARAMS(ZEND_NUM_ARGS() TSRMLS_CC, "s", &pname, &pnamelen);
	PHP_SAP_PARSE_PARAMS_END();

	RETVAL_FALSE;

	if (FAILURE == res) {
		return;
	}

	intern = sap_get_function(getThis());

	if (!intern->function_descr || !(sp = sap_hash_find_ptr(intern->function_descr->params, pname, pnamelen))) {
		return;
	}

	if (sp->param.type == RFCTYPE_TABLE || sp->param.type == RFCTYPE_STRUCTURE)
	{
		RFC_ABAP_NAME typeNameU;
		char *typeName;
		int typeNameLen;

		memset(&sap_last_error, 0, sizeof(SAPRFC_ERROR_INFO));

		if (RFC_OK != RfcGetTypeName(sp->param.typeDescHandle, typeNameU, (RFC_ERROR_INFO*)&sap_last_error)) {
			SAP_ERROR_SET_RFCFUNCTION(&sap_last_error, "RfcGetTypeName", sizeof("RfcGetTypeName") - 1);
			SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
			return;
		}

		if (SUCCESS != sapuc_to_utf8(typeNameU, &typeName, &typeNameLen, &sap_last_error)) {
			SAP_THROW_SAPRFC_ERROR_EXCEPTION(&sap_last_error);
			return;
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

	array_init(&__invoke_args);

	if (!(pzimports = sap_hash_next_index_insert_new_zval(Z_ARRVAL(__invoke_args)))) {
		zval_dtor(&__invoke_args);
		RETURN_FALSE;
	}

	array_init(pzimports);

	/* set requested table */
	{
		zval *ztablename;

		if ((ztablename = sap_hash_add_new_zval(Z_ARRVAL_P(pzimports), "QUERY_TABLE", sizeof("QUERY_TABLE") - 1))) {
#if PHP_VERSION_ID >= 70000
			ZVAL_STRINGL(ztablename, table_name, table_name_len);
#else
			ZVAL_STRINGL(ztablename, table_name, table_name_len, 1);
#endif
		}
	}

	/* set delimiter = "" */
	{
		zval *zdelimiter;

		if ((zdelimiter = sap_hash_add_new_zval(Z_ARRVAL_P(pzimports), "DELIMITER", sizeof("DELIMITER") - 1))) {
#if PHP_VERSION_ID >= 70000
			ZVAL_STRING(zdelimiter, "");
#else
			ZVAL_STRING(zdelimiter, "", 1);
#endif
		}
	}

	/* set rowcount */
	if (rowCount > 0)
	{
		zval *zrowCount;

		if ((zrowCount = sap_hash_add_new_zval(Z_ARRVAL_P(pzimports), "ROWCOUNT", sizeof("ROWCOUNT") - 1))) {
			ZVAL_LONG(zrowCount, rowCount);
		}
	}

	/* set rowskips */
	if (offset > 0)
	{
		zval *zrowskips;

		if ((zrowskips = sap_hash_add_new_zval(Z_ARRVAL_P(pzimports), "ROWSKIPS", sizeof("ROWSKIPS") - 1))) {
			ZVAL_LONG(zrowskips, offset);
		}
	}

	/* set requested fields */
	{
		zval *zexportfields;

		if ((zexportfields = sap_hash_add_new_zval(Z_ARRVAL_P(pzimports), "FIELDS", sizeof("FIELDS") - 1)))
		{
			array_init(zexportfields);

			if (Z_TYPE_P(zfields) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(zfields)) > 0)
			{
				zval *zfieldname;
				ulong h;
				char *falias;
				int faliaslen;

				SAP_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(zfields), h, falias, faliaslen, zfieldname)
				{
					zval *zfieldrow;
					zval *zfieldalias = NULL;
#if PHP_VERSION_ID >= 70000
					zval fieldalias;
#endif

					/* Field names must be strings */
					if (Z_TYPE_P(zfieldname) != IS_STRING) {
						php_error(E_WARNING, "Ignoring incorrect field %s", falias ? "alias" : "name");
						continue;
					}

					if (falias) { /* Field alias */
#if PHP_VERSION_ID >= 70000
						zfieldalias = &fieldalias;
						ZVAL_STRINGL(zfieldalias, falias, faliaslen);
#else
						MAKE_STD_ZVAL(zfieldalias);
						ZVAL_STRINGL(zfieldalias, falias, faliaslen, 1);
#endif
						Z_DELREF_P(zfieldalias);
						zfieldname = zfieldalias;
					}

					/* Append field */
					if (zfieldrow = sap_hash_next_index_insert_new_zval(Z_ARRVAL_P(zexportfields)))
					{
						array_init_size(zfieldrow, 1);

						if (sap_hash_add_zval(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1, zfieldname)) {
							Z_ADDREF_P(zfieldname);
						}
					}
				}
				SAP_HASH_FOREACH_END();
			}
			else if (Z_TYPE_P(zfields) == IS_STRING && Z_STRLEN_P(zfields) > 0)
			{
				zval *zfieldname = zfields;

				if (memcmp(Z_STRVAL_P(zfieldname), "*", sizeof("*") - 1))
				{
					/* Select single field */
					zval *zfieldrow;

					if (zfieldrow = sap_hash_next_index_insert_new_zval(Z_ARRVAL_P(zexportfields)))
					{
						array_init_size(zfieldrow, 1);

						if (sap_hash_add_zval(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1, zfieldname)) {
							Z_ADDREF_P(zfieldname);
						}
					}
				}
			}
			else {
				sap_throw_exception("Argument 1 of SapRfcReadTable::select(). Must be '*' or non-empty string|array", -1, sap_ce_SapException);
				return;
			}
		}
	}

	/* Create the 'WHERE' clause */
	if (NULL != zwhere && zend_hash_num_elements(Z_ARRVAL_P(zwhere)) > 0)
	{
		char *field;
		int field_len;
		ulong h;
		zval *val;
		zval *zoptions = NULL;

		if ((zoptions = sap_hash_add_new_zval(Z_ARRVAL_P(pzimports), "OPTIONS", sizeof("OPTIONS") - 1)))
		{
			array_init(zoptions);

			SAP_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(zwhere), h, field, field_len, val)
			{
				zval *pzrealval = NULL;
				zval *zoptionsrow = NULL;
				zval *zoptiontext = NULL;
#if PHP_VERSION_ID >= 70000
				zval zrealval;
#endif

				/* append new row to table 'OPTIONS' */
				if (!(zoptionsrow = sap_hash_next_index_insert_new_zval(Z_ARRVAL_P(zoptions)))) {
					break;
				}

				array_init_size(zoptionsrow, 1);

				/* append field 'TEXT' to the new row */
				if (!(zoptiontext = sap_hash_add_new_zval(Z_ARRVAL_P(zoptionsrow), "TEXT", sizeof("TEXT") - 1))) {
					break;
				}

				/* element's value must be a string */
				if (Z_TYPE_P(val) != IS_STRING)
				{
#if PHP_VERSION_ID < 70000
					MAKE_STD_ZVAL(pzrealval);
#else
					pzrealval = &zrealval;
#endif
					ZVAL_ZVAL(pzrealval, val, 1, 0);
					convert_to_string(pzrealval);
					val = pzrealval;
				}

				if (NULL != field) /* string key: field => val */
				{
					char optionText[300];
					int optionTextLen;
					int hasRows = zend_hash_num_elements(Z_ARRVAL_P(zoptions)) > 1;

					/* "field EQ 'val'" */
					optionTextLen = snprintf(optionText, sizeof(optionText), "%s%s EQ '%s'", hasRows ? "AND " : "", field, Z_STRVAL_P(val));

#if PHP_VERSION_ID >= 70000
					ZVAL_STRINGL(zoptiontext, optionText, optionTextLen);
#else
					ZVAL_STRINGL(zoptiontext, optionText, optionTextLen, 1);
#endif
				}
				else { /* custom WHERE clause, append 'as is' */
					ZVAL_ZVAL(zoptiontext, val, 1, 0);
				}

				if (UNEXPECTED(val == pzrealval)) {
					sap_zval_ptr_dtor(pzrealval);
				}
			}
			SAP_HASH_FOREACH_END();
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

			if ((zresultdata = sap_hash_find_zval(Z_ARRVAL_P(pzresult), "DATA", sizeof("DATA") - 1)) && Z_TYPE_P(zresultdata) == IS_ARRAY
			&&	(zresultfields = sap_hash_find_zval(Z_ARRVAL_P(pzresult), "FIELDS", sizeof("FIELDS") - 1)) && Z_TYPE_P(zresultfields) == IS_ARRAY)
			{
				array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(zresultdata)));

				SAP_HASH_FOREACH_VAL(Z_ARRVAL_P(zresultdata), zdatarow)
				{
					zval *zresultrow;
					zval *wa;
					zval *zfieldrow;

					if ((wa = sap_hash_find_zval(Z_ARRVAL_P(zdatarow), "WA", sizeof("WA") - 1)) && Z_TYPE_P(wa) == IS_STRING
					&&	(zresultrow = sap_hash_next_index_insert_new_zval(Z_ARRVAL_P(return_value))))
					{
						array_init_size(zresultrow, zend_hash_num_elements(Z_ARRVAL_P(zresultfields)));

						SAP_HASH_FOREACH_VAL(Z_ARRVAL_P(zresultfields), zfieldrow)
						{
							zval *zfieldname, *zfieldoffset, *zfieldlength, *zfieldtype;
							zval zlongoffset, zlonglength;

							if (Z_TYPE_P(zfieldrow) != IS_ARRAY) {
								continue; /* This should not happen */
							}

							if ((zfieldname = sap_hash_find_zval(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1)) && Z_TYPE_P(zfieldname) == IS_STRING
							&&	(zfieldoffset = sap_hash_find_zval(Z_ARRVAL_P(zfieldrow), "OFFSET", sizeof("OFFSET") - 1))
							&&	(zfieldlength = sap_hash_find_zval(Z_ARRVAL_P(zfieldrow), "LENGTH", sizeof("LENGTH") - 1))
							&&	(zfieldtype = sap_hash_find_zval(Z_ARRVAL_P(zfieldrow), "TYPE", sizeof("TYPE") - 1)) && Z_TYPE_P(zfieldtype) == IS_STRING)
							{
								zval *zfieldvalue;
								zval *zfieldalias;

								ZVAL_ZVAL(&zlongoffset, zfieldoffset, 1, 0);
								convert_to_long(&zlongoffset);

								ZVAL_ZVAL(&zlonglength, zfieldlength, 1, 0);
								convert_to_long(&zlonglength);

								if (Z_TYPE_P(zfields) == IS_ARRAY && (zfieldalias = sap_hash_find_zval(Z_ARRVAL_P(zfields), Z_STRVAL_P(zfieldname), Z_STRLEN_P(zfieldname)))) {
									/* is field alias */
									zfieldname = zfieldalias;
								}

								if (zfieldvalue = sap_hash_add_new_zval(Z_ARRVAL_P(zresultrow), Z_STRVAL_P(zfieldname), Z_STRLEN_P(zfieldname)))
								{
									char *substr;
									int substrlen;

									if (SUCCESS == utf8_substr(Z_STRVAL_P(wa), Z_STRLEN_P(wa), Z_LVAL(zlongoffset), Z_LVAL(zlonglength), &substr, &substrlen))
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
									}
#if PHP_VERSION_ID >= 70000
									ZVAL_STRINGL(zfieldvalue, substr, substrlen);
									efree(substr);
#else
									ZVAL_STRINGL(zfieldvalue, substr, substrlen, 0);
#endif
								}
							}
						}
						SAP_HASH_FOREACH_END();
					}
				}
				SAP_HASH_FOREACH_END();
			}
		}

		if (result == SUCCESS) {
			sap_zval_ptr_dtor(pzresult);
		}
	}
}

PHP_MINIT_FUNCTION(sap)
{
	zend_class_entry exception_ce, sap_ce, function_ce, rfcreadtable_ce;
	
	/* instatiate last error to NULL*/
	memset(&sap_last_error, 0, sizeof(SAPRFC_ERROR_INFO));

	/* Register resources */
	le_php_sap_connection = zend_register_list_destructors_ex(php_sap_connection_rsrc_dtor, NULL, PHP_SAP_CONNECTION_RES_NAME, module_number);

	ZEND_INIT_MODULE_GLOBALS(sap, ZEND_MODULE_GLOBALS_CTOR_N(sap), NULL);

	REGISTER_INI_ENTRIES();

	INIT_CLASS_ENTRY(exception_ce, "SapException", sap_exception_fe);
#if PHP_VERSION_ID < 70000
	sap_ce_SapException = zend_register_internal_class_ex(&exception_ce, zend_default_exception, "Exception" TSRMLS_CC);
#else
	sap_ce_SapException = zend_register_internal_class_ex(&exception_ce, zend_default_exception);
#endif
	
	zend_declare_property_string(sap_ce_SapException, "rfcFunction", sizeof("rfcFunction") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgClass", sizeof("msgClass") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgNumber", sizeof("msgNumber") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgType", sizeof("msgType") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgVar1", sizeof("msgVar1") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgVar2", sizeof("msgVar2") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgVar3", sizeof("msgVar3") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "msgVar4", sizeof("msgVar4") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(sap_ce_SapException, "key", sizeof("key") - 1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	
	INIT_CLASS_ENTRY(sap_ce, "Sap", sap_fe_Sap);
	sap_ce_Sap = zend_register_internal_class(&sap_ce TSRMLS_CC);

	memcpy(&sap_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sap_object_handlers.clone_obj = sap_object_clone_object;
	
#if PHP_VERSION_ID >= 70000
	sap_object_handlers.free_obj = sap_object_free_object_storage;
	sap_object_handlers.offset = XtOffsetOf(sap_object, std);
#endif

	sap_ce_Sap->create_object = sap_object_create_object;

	INIT_CLASS_ENTRY(function_ce, "SapFunction", sap_fe_SapFunction);
	sap_ce_SapFunction = zend_register_internal_class(&function_ce TSRMLS_CC);
	
	memcpy(&sap_function_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	sap_function_handlers.clone_obj = sap_function_clone_object;
#if PHP_VERSION_ID >= 70000
	sap_function_handlers.free_obj = sap_function_free_object_storage;
	sap_function_handlers.offset = XtOffsetOf(sap_function, std);
#endif
	
	sap_ce_SapFunction->create_object = sap_function_create_object;

	INIT_CLASS_ENTRY(rfcreadtable_ce, "SapRfcReadTable", sap_fe_SapRfcReadTable);
#if PHP_VERSION_ID < 70000
	sap_ce_SapRfcReadTable = zend_register_internal_class_ex(&rfcreadtable_ce, sap_ce_SapFunction, "SapFunction" TSRMLS_CC);
#else
	sap_ce_SapRfcReadTable = zend_register_internal_class_ex(&rfcreadtable_ce, sap_ce_SapFunction);
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
	const SAP_UC *sdkversionU = RfcGetVersion(&major, &minor, &patchlevel);
	char *nwsdkversion = "";
	int nwsdkversionlen, res;

	res = sapuc_to_utf8((SAP_UC*)sdkversionU, &nwsdkversion, &nwsdkversionlen, &sap_last_error);

	php_info_print_table_start();
	php_info_print_table_header(2, "SAP Remote Functions call support", "enabled");
	php_info_print_table_row(2, "Version", PHP_SAP_VERSION);
	php_info_print_table_row(2, "Nw Rfc Sdk Version", nwsdkversion);
	php_info_print_table_end();

	if (SUCCESS == res) {
		efree(nwsdkversion);
	}

	DISPLAY_INI_ENTRIES();
}