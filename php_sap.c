#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "php_sap.h"

#if defined(SAPwithPTHREADS)
#   if defined(WIN32)
#       define HAVE_STRUCT_TIMESPEC
#   endif
#   include "pthread.h"
#endif

#include "php_ini.h"
#include "ext/standard/info.h"

#include "zend_objects.h"
#include "zend_exceptions.h"

#include "ext/spl/spl_exceptions.h"
#define php_sap_invalid_args_exception spl_ce_InvalidArgumentException

#include "ext/date/php_date.h"

#define _NEWZVAL &EG(uninitialized_zval)
#define sap_get_intern(_arg, type) (type*)((char *)Z_OBJ_P(_arg) - Z_OBJ_P(_arg)->handlers->offset)
#define sap_get_function_object(_arg) sap_get_intern(_arg, sap_function)
#define sap_get_sap_object(_arg) sap_get_intern(_arg, sap_object)
#define sap_read_property(prop) zend_read_property(Z_OBJCE_P(getThis()), getThis(), prop, sizeof(prop), 0, NULL)

#define SAP_ME_ARGS(classname, method) arginfo_##classname##_##method
#define SAP_FE_ARGS(func) arginfo_func_##func

#define SAP_ERROR_SET_RFCFUNCTION(_err, _func, _funclen) do {       \
    SAPRFC_ERROR_INFO *__err = (_err);                              \
    const char *__func = (_func);                                   \
    unsigned int __funclen = (_funclen);                            \
    memset(__err->nwsdkfunction, 0, sizeof(__err->nwsdkfunction));  \
    __err->l_nwsdkfunction = 0;                                     \
    if (_func != NULL) {                                            \
        __err->l_nwsdkfunction = __funclen;                         \
        memcpy(__err->nwsdkfunction, __func, __funclen);            \
    }                                                               \
} while (0);

#define SAP_ERROR_SET_FUNCTION_AND_RETURN(_err, _func, _retval) \
    SAP_ERROR_SET_RFCFUNCTION(_err, _func, strlen(_func));      \
    return _retval

#define SAP_ERR_RETURN_FAILURE(_err, _func) SAP_ERROR_SET_FUNCTION_AND_RETURN(_err, _func, FAILURE)
#define SAP_ERR_RETURN_NULL(_err, _func) SAP_ERROR_SET_FUNCTION_AND_RETURN(_err, _func, NULL)

#define SAP_THROW_SAPRFC_ERROR_EXCEPTION(_err) {    \
    SAPRFC_ERROR_INFO *__err = (_err);              \
    zval _ex;                                       \
    sap_error_to_exception(__err, &_ex);            \
    zend_throw_exception_object(&_ex);              \
}

#define XtSizeOf(type, member) sizeof(((type *)0)->member)

typedef enum _TRIM_TYPE {
    TRIM_NONE = 0x00,
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
    RFC_PARAMETER_DESC  param;
    SAPRFC_PARAM_STATE  state;
} SAPRFC_PARAMETER_DESC;

typedef struct _sap_function {
    php_sap_connection  *connection;
    php_sap_function    *function_descr;
    zend_object         std;
} sap_function;

typedef struct _sap_object {
    php_sap_connection  *connection;
    zend_class_entry    *func_ce;
    zend_object         std;
} sap_object;

ZEND_BEGIN_MODULE_GLOBALS(sap)
    int rtrim_export_strings;
ZEND_END_MODULE_GLOBALS(sap)

ZEND_DECLARE_MODULE_GLOBALS(sap)

static ZEND_MODULE_GLOBALS_CTOR_D(sap)
{
    sap_globals->rtrim_export_strings = 1;
}

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("sap.rtrim_export_strings", "Off", PHP_INI_ALL, OnUpdateBool, rtrim_export_strings, zend_sap_globals, sap_globals)
PHP_INI_END()

#ifdef SAPwithPTHREADS
pthread_mutex_t rfc_utf8_to_sapuc_mutex;
pthread_mutex_t rfc_sapuc_to_utf8_mutex;
#endif

/** php_sap module procedural functions -- begin **/

PHP_FUNCTION(sap_connect);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_FE_ARGS(sap_connect), IS_RESOURCE, NULL, 0)
    ZEND_ARG_ARRAY_INFO(0, logonParams, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(sap_invoke_function);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_FE_ARGS(sap_invoke_function), IS_ARRAY, NULL, 0)
    ZEND_ARG_TYPE_INFO(0, moduleName, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, connection, IS_RESOURCE, 0)
    ZEND_ARG_ARRAY_INFO(0, imports, 1)
ZEND_END_ARG_INFO()

zend_function_entry php_sap_module_function_entry[] = {
    PHP_FE(sap_connect,         SAP_FE_ARGS(sap_connect))
    PHP_FE(sap_invoke_function, SAP_FE_ARGS(sap_invoke_function))
    PHP_FE_END
};

/** php_sap module procedural functions -- end **/

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

#ifdef COMPILE_DL_SAP
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
PHP_SAP_API zend_class_entry * php_sap_get_sap_ce(void)
{
    return sap_ce_Sap;
}

PHP_SAP_API zend_object_handlers * php_sap_get_sap_handlers(void)
{
    return &sap_object_handlers;
}

PHP_SAP_API php_sap_connection * php_sap_get_connection(zval *zsap)
{
    sap_object *intern = sap_get_sap_object(zsap);

    return intern->connection;
}

PHP_SAP_API zend_class_entry * php_sap_get_function_ce(void)
{
    return sap_ce_SapFunction;
}

PHP_SAP_API zend_object_handlers * php_sap_get_function_handlers(void)
{
    return &sap_function_handlers;
}

PHP_SAP_API php_sap_function * php_sap_get_function_descr(zval *zfunction)
{
    sap_function *intern = sap_get_function_object(zfunction);

    return intern->function_descr;
}

PHP_SAP_API zend_class_entry * php_sap_get_exception_ce(void)
{
    return sap_ce_SapException;
}
/** Exporting functions - end **/

/** Class methods definition -- begin **/

/* {{{ */

/* SapException */

/* get message key */
PHP_METHOD(SapException, getMessageKey);

/* returns string */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageKey), IS_STRING, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get message type */
PHP_METHOD(SapException, getMessageType);

/* returns string :: MSGTY */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageType), IS_STRING, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get message id or class */
PHP_METHOD(SapException, getMessageId);

/* returns string :: MSGID */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageId), IS_STRING, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get message number */
PHP_METHOD(SapException, getMessageNumber);

/* returns int :: MSGNO */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageNumber), IS_LONG, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get error's variable 1 */
PHP_METHOD(SapException, getMessageVar1);

/* returns string|null :: MSGV */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageVar1), IS_STRING, NULL, 1)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get error's variable 2 */
PHP_METHOD(SapException, getMessageVar2);

/* returns string|null :: MSGV */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageVar2), IS_STRING, NULL, 1)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get error's variable 3 */
PHP_METHOD(SapException, getMessageVar3);

/* returns string|null :: MSGV */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageVar3), IS_STRING, NULL, 1)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get error's variable 4 */
PHP_METHOD(SapException, getMessageVar4);

/* returns string|null :: MSGV */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getMessageVar4), IS_STRING, NULL, 1)
    /* no arguments */
ZEND_END_ARG_INFO()

/* get the internal sdk function the error came from */
PHP_METHOD(SapException, getNwSdkFunction);

/* returns string|null :: function's name */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapException, getNwSdkFunction), IS_STRING, NULL, 1)
    /* no arguments */
ZEND_END_ARG_INFO()

const zend_function_entry sap_exception_fe[] = {
    PHP_ME(SapException, getMessageKey,     SAP_ME_ARGS(SapException, getMessageKey),       ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageType,    SAP_ME_ARGS(SapException, getMessageType),      ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageId,      SAP_ME_ARGS(SapException, getMessageId),        ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageNumber,  SAP_ME_ARGS(SapException, getMessageNumber),    ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageVar1,    SAP_ME_ARGS(SapException, getMessageVar1),      ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageVar2,    SAP_ME_ARGS(SapException, getMessageVar2),      ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageVar3,    SAP_ME_ARGS(SapException, getMessageVar3),      ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getMessageVar4,    SAP_ME_ARGS(SapException, getMessageVar4),      ZEND_ACC_PUBLIC)
    PHP_ME(SapException, getNwSdkFunction,  SAP_ME_ARGS(SapException, getNwSdkFunction),    ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* Sap */

/* Sap constructor */
PHP_METHOD(Sap, __construct);

/* return void, throws SapCommunicationException */
ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(Sap, __construct), 0)
    /* array: logon parameters */
    ZEND_ARG_ARRAY_INFO(0, logonParameters, 0)
ZEND_END_ARG_INFO()

/* sets the default class this connection uses for fetching remote function objects */
PHP_METHOD(Sap, setFunctionClass);

/* return void */
ZEND_BEGIN_ARG_INFO(SAP_ME_ARGS(Sap, setFunctionClass), 0)
    /* string: the class name derived from SapFunction */
    ZEND_ARG_TYPE_INFO(0, className, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* get the default class this connection uses for fetching remote function objects */
PHP_METHOD(Sap, getFunctionClass);

/* return string: class name */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(Sap, getFunctionClass), IS_STRING, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

/* directly invokes a remote function on the backend system */
PHP_METHOD(Sap, call);

/* return array: EXPORT/CHANGING/TABLE parameters, throws SapException */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(Sap, call), IS_ARRAY, NULL, 0)
    /* remote function name to be invoked, string, not null */
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    /* import parameters as array, optional, can be null */
    ZEND_ARG_ARRAY_INFO(0, imports, 1)
ZEND_END_ARG_INFO()

/* fetches a remote function description from the backend */
PHP_METHOD(Sap, fetchFunction);

/* return SapFunction: a "ready to call" SapFunction object, throws SapException */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SAP_ME_ARGS(Sap, fetchFunction), 0, 1, IS_OBJECT, "SapFunction", 0)
    /* string|SapFunction: remote function's name or a SapFunction object */
    ZEND_ARG_INFO(0, name)
    /* string|null: a class name derived from SapFunction to create the return object */
    ZEND_ARG_TYPE_INFO(0, moduleClass, IS_STRING, 1)
    /* array|null: constructor arguments, used in conjuction with the second parameter */
    ZEND_ARG_ARRAY_INFO(0, ctor_args, 1)
ZEND_END_ARG_INFO()

/* get connection attributes */
PHP_METHOD(Sap, getAttributes);

/* return array: connection attributes, throws SapException */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(Sap, getAttributes), IS_ARRAY, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

const zend_function_entry sap_fe_Sap[] = {
    PHP_ME(Sap, __construct,        SAP_ME_ARGS(Sap, __construct),      ZEND_ACC_PUBLIC)
    PHP_ME(Sap, setFunctionClass,   SAP_ME_ARGS(Sap, setFunctionClass), ZEND_ACC_PUBLIC)
    PHP_ME(Sap, call,               SAP_ME_ARGS(Sap, call),             ZEND_ACC_PUBLIC)
    PHP_ME(Sap, fetchFunction,      SAP_ME_ARGS(Sap, fetchFunction),    ZEND_ACC_PUBLIC)
    PHP_ME(Sap, getAttributes,      SAP_ME_ARGS(Sap, getAttributes),    ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* SapFunction */

/* get remote function's name */
PHP_METHOD(SapFunction, getName);

/* return string: remote function's name */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapFunction, getName), IS_STRING, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

/* sets a parameter active/inactive */
PHP_METHOD(SapFunction, setActive);

/* return bool: true on success or false on failure, throws UnexpectedValueException */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapFunction, setActive), _IS_BOOL, NULL, 0)
    /* string: parameter name */
    ZEND_ARG_INFO(0, param)
    /* bool|null: true for active, false for inactive, null for default state */
    ZEND_ARG_INFO(0, isActive)
ZEND_END_ARG_INFO()

PHP_METHOD(SapFunction, __invoke);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapFunction, __invoke), IS_ARRAY, NULL, 0)
    ZEND_ARG_ARRAY_INFO(0, imports, 1)
    ZEND_ARG_TYPE_INFO(0, rtrim, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(SapFunction, getParameters);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapFunction, getParameters), IS_ARRAY, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

PHP_METHOD(SapFunction, getTypeName);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapFunction, getTypeName), IS_STRING, NULL, 1)
    ZEND_ARG_TYPE_INFO(0, param, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(SapFunction, __toString);

const zend_function_entry sap_fe_SapFunction[] = {
    PHP_ME(SapFunction, __invoke,       SAP_ME_ARGS(SapFunction, __invoke),     ZEND_ACC_PUBLIC)
    PHP_ME(SapFunction, getName,        SAP_ME_ARGS(SapFunction, getName),      ZEND_ACC_PUBLIC)
    PHP_ME(SapFunction, getParameters,  SAP_ME_ARGS(SapFunction, getParameters),ZEND_ACC_PUBLIC)
    PHP_ME(SapFunction, setActive,      SAP_ME_ARGS(SapFunction, setActive),    ZEND_ACC_PUBLIC)
    PHP_ME(SapFunction, getTypeName,    SAP_ME_ARGS(SapFunction, getTypeName),  ZEND_ACC_PUBLIC)
    PHP_ME(SapFunction, __toString,     NULL,                                   ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* SapRfcReadTable */
PHP_METHOD(SapRfcReadTable, getName);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapRfcReadTable, getName), IS_STRING, NULL, 0)
    /* no arguments */
ZEND_END_ARG_INFO()

PHP_METHOD(SapRfcReadTable, select);

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(SAP_ME_ARGS(SapRfcReadTable, select), IS_ARRAY, NULL, 0)
    ZEND_ARG_INFO(0, fields)
    ZEND_ARG_TYPE_INFO(0, table, IS_STRING, 0)
    ZEND_ARG_ARRAY_INFO(0, options, 1)
    ZEND_ARG_TYPE_INFO(0, limit, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
ZEND_END_ARG_INFO()

const zend_function_entry sap_fe_SapRfcReadTable[] = {
    PHP_ME(SapRfcReadTable, getName,    SAP_ME_ARGS(SapRfcReadTable, getName),  ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(SapRfcReadTable, select,     SAP_ME_ARGS(SapRfcReadTable, select),   ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/** Class methods definition -- end **/

/* }}} */

static int sap_import(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err);

static int sap_export(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *rv, TRIM_TYPE strTrimType, SAPRFC_ERROR_INFO *err);

/* Common functions */

static unsigned int strlenU8(char *str, unsigned int str_len)
{
    unsigned int len = 0, pos = 0;

    for (pos = 0; pos < str_len && str[pos]; pos++) {
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

#if SAPwithPTHREADS
    /* Avoid concurrent access to the RfcSAPUCToUTF8 function */
    pthread_mutex_lock(&rfc_utf8_to_sapuc_mutex);
#endif
    res = RfcUTF8ToSAPUC((RFC_BYTE*)str, len, retval, &sapuc_num_chars, uc_len, (RFC_ERROR_INFO*)err);
#if SAPwithPTHREADS
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
                memset(retval, 0, uc_size);
            } else {
                retval = ecalloc(1, uc_size);
            }

            goto try_again;
        }
        case RFC_OK:
            if (NULL == retval) {
                /* retval will be NULL if str == "", so we have to allocate an empty SAP_UC string */
                retval = mallocU(0);
            }

            *uc = retval;

            return SUCCESS;
        default:
            if (NULL != retval) {
                efree(retval);
            }

            SAP_ERR_RETURN_FAILURE(err, "RfcUTF8ToSAPUC");
    }
}

static int utf8_to_sapuc(char *str, SAP_UC **uc, unsigned int *uc_len, SAPRFC_ERROR_INFO *err)
{
    return utf8_to_sapuc_l(str, strlen(str), uc, uc_len, err);
}

static int sapuc_to_utf8_l(SAP_UC *strU16, unsigned int strU16len, char **strU8, int *strU8len, SAPRFC_ERROR_INFO *err)
{
    char *utf8 = NULL;
    unsigned int utf8len = 0;
    RFC_RC res;

try_again:

#if SAPwithPTHREADS
    /* Avoid concurrent access to the RfcSAPUCToUTF8 function */
    pthread_mutex_lock(&rfc_sapuc_to_utf8_mutex);
#endif
    res = RfcSAPUCToUTF8(strU16, strU16len, (RFC_BYTE*)utf8, &utf8len, strU8len, (RFC_ERROR_INFO*)err);
#if SAPwithPTHREADS
    pthread_mutex_unlock(&rfc_sapuc_to_utf8_mutex);
#endif

    switch (res)
    {
        case RFC_BUFFER_TOO_SMALL: {
            /*
             * In case of RFC_BUFFER_TOO_SMALL, nw sdk fills 'utf8len' with the required number
             * of bytes for convertion (including the trailing null byte)
             */
            if (NULL != utf8) {
                utf8 = erealloc(utf8, utf8len);
                memset(utf8, 0, utf8len);
            } else {
                utf8 = ecalloc(1, utf8len);
            }

            goto try_again;
        }
        case RFC_OK: {

            if (NULL == utf8) {
                /* utf8 will be NULL if str == "", so we have to allocate an empty string */
                utf8 = estrndup("", 0);
            }

            *strU8 = utf8;

            return SUCCESS;
        }
        default: {
            if (NULL != utf8) {
                efree(utf8);
            }

            SAP_ERR_RETURN_FAILURE(err, "RfcUTF8ToSAPUC");
        }
    }
}

static int sapuc_to_utf8(SAP_UC *strU16, char **strU8, int *strU8len, SAPRFC_ERROR_INFO *err)
{
    return sapuc_to_utf8_l(strU16, strlenU(strU16), strU8, strU8len, err);
}

static int sap_call_object_method(zval *object, zend_class_entry *scope_ce, const char *func, zend_function *fn_proxy, zval *args, zval *rv)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    int retval;

    /* Setup function call info */
#if PHP_VERSION_ID < 70100
    fci.symbol_table = NULL;
    fci.function_table = &scope_ce->function_table;
#endif
    fci.no_separation = 1;
    fci.param_count = 0;
    fci.params = NULL;
    fci.size = sizeof(zend_fcall_info);

    fci.retval = rv;

    ZVAL_STRING(&fci.function_name, func);

    if (!fn_proxy && !(fn_proxy = zend_hash_find_ptr(&scope_ce->function_table, Z_STR(fci.function_name)))) {
        zend_error(E_CORE_ERROR, "Could not find implementation for method %s::%s", ZSTR_VAL(scope_ce->name), func);
    }

    /* Set function call arguments */
    zend_fcall_info_args(&fci, args);

    /* Setup function call cache */
    fcc.object = fci.object = Z_OBJ_P(object);
    fcc.initialized = 1;
    fcc.function_handler = fn_proxy;
    fcc.calling_scope = scope_ce;
    fcc.called_scope = scope_ce;

    /* Do call */
    retval = zend_call_function(&fci, &fcc);

    zval_dtor(&fci.function_name);

    return retval;
}

int format_datetime_object(zval *object, zval *return_value, const char *format)
{
    zval __args;
    zval *zformat_arg;
    int result;

    array_init(&__args);

    zformat_arg = zend_hash_next_index_insert_new(Z_ARRVAL(__args), _NEWZVAL);

    if (NULL == zformat_arg) {
        zval_dtor(&__args);
        return FAILURE;
    }

    ZVAL_STRING(zformat_arg, format);

    result = sap_call_object_method(object, Z_OBJCE_P(object), "format", NULL, &__args, return_value);

    zval_dtor(&__args);

    return result;
}

static inline zval sap_create_exception(zend_class_entry *ce, int code, char *format, ...)
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

static inline void sap_create_error(SAPRFC_ERROR_INFO *err, const char *key, RFC_RC code, char *format, ...)
{
    va_list args;
    char *message;
    unsigned int messageUlen = 512, keyUlen = 128,
        msgClassUlen = 21, msgNumberUlen = 4, msgTypeUlen = 2,
        msgVar1Ulen = 51, msgVar2Ulen = 51, msgVar3Ulen = 51, msgVar4Ulen = 51;
    RFC_ERROR_INFO e;

    /* Clear error */
    memset(err, 0, sizeof(SAPRFC_ERROR_INFO));

    va_start(args, format);
    vspprintf(&message, 0, format, args);
    va_end(args);

#if SAPwithPTHREADS
    pthread_mutex_lock(&rfc_utf8_to_sapuc_mutex);
#endif
    RfcUTF8ToSAPUC(message, strlen(message), err->err.message, &messageUlen, &messageUlen, &e);
    RfcUTF8ToSAPUC(key, strlen(key), err->err.key, &keyUlen, &keyUlen, &e);
    RfcUTF8ToSAPUC("PHP_SAP_ERROR", sizeof("PHP_SAP_ERROR") - 1, err->err.abapMsgClass, &msgClassUlen, &msgClassUlen, &e);
    RfcUTF8ToSAPUC("000", sizeof("000") - 1, err->err.abapMsgNumber, &msgNumberUlen, &msgNumberUlen, &e);
    RfcUTF8ToSAPUC("E", sizeof("E") - 1, err->err.abapMsgType, &msgTypeUlen, &msgTypeUlen, &e);
    RfcUTF8ToSAPUC("", sizeof("") - 1, err->err.abapMsgV1, &msgVar1Ulen, &msgVar1Ulen, &e);
    RfcUTF8ToSAPUC("", sizeof("") - 1, err->err.abapMsgV2, &msgVar2Ulen, &msgVar2Ulen, &e);
    RfcUTF8ToSAPUC("", sizeof("") - 1, err->err.abapMsgV3, &msgVar3Ulen, &msgVar3Ulen, &e);
    RfcUTF8ToSAPUC("", sizeof("") - 1, err->err.abapMsgV4, &msgVar4Ulen, &msgVar4Ulen, &e);
#if SAPwithPTHREADS
    pthread_mutex_unlock(&rfc_utf8_to_sapuc_mutex);
#endif

    err->err.code = code;
}

static inline zval sap_error_to_exception(SAPRFC_ERROR_INFO *err)
{
    zval exception;
    char *key, *msgType, *msgId, *msgNumber, *msgv1, *msgv2, *msgv3, *msgv4, *message;
    int keyLen, msgTypeLen, msgIdLen, msgNumberLen, msgv1Len, msgv2Len, msgv3Len, msgv4Len, messageLen;
    SAPRFC_ERROR_INFO e;

    object_init_ex(&exception, sap_ce_SapException);

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgType, &msgType, &msgTypeLen, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGTY", sizeof("MSGTY") - 1, msgType, msgTypeLen);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgClass, &msgId, &msgIdLen, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGID", sizeof("MSGID") - 1, msgId, msgIdLen);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgNumber, &msgNumber, &msgNumberLen, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGNO", sizeof("MSGNO") - 1, msgNumber, msgNumberLen);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.message, &message, &messageLen, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "message", sizeof("message") - 1, message, messageLen);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV1, &msgv1, &msgv1Len, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGV1", sizeof("MSGV1") - 1, msgv1, msgv1Len);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV2, &msgv2, &msgv2Len, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGV2", sizeof("MSGV2") - 1, msgv2, msgv2Len);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV3, &msgv3, &msgv3Len, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGV3", sizeof("MSGV3") - 1, msgv3, msgv3Len);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.abapMsgV4, &msgv4, &msgv4Len, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "MSGV4", sizeof("MSGV4") - 1, msgv4, msgv4Len);
    }

    if (SUCCESS == sapuc_to_utf8(err->err.key, &key, &keyLen, &e)) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "KEY", sizeof("KEY") - 1, key, keyLen);
    }

    if (err->l_nwsdkfunction > 0) {
        zend_update_property_stringl(sap_ce_SapException, &exception, "nwsdkfunction", sizeof("nwsdkfunction") - 1, err->nwsdkfunction, err->l_nwsdkfunction);
    }

    zend_update_property_long(sap_ce_SapException, &exception, "code", sizeof("code") - 1, err->err.code);

    return exception;
}

static void sap_rfc_parameter_ptr_dtor(zval *zparam)
{
    SAPRFC_PARAMETER_DESC *p = Z_PTR_P(zparam);
    efree(p);
}

static HashTable * sap_function_description_to_array(RFC_FUNCTION_DESC_HANDLE fdh, SAPRFC_ERROR_INFO *err)
{
    unsigned int paramCount, i;
    HashTable *retval;
    SAPRFC_PARAMETER_DESC *sp;

    if (RFC_OK != RfcGetParameterCount(fdh, &paramCount, (RFC_ERROR_INFO*)err)) {
        SAP_ERR_RETURN_NULL(err, "RfcGetParameterCount");
    }
    
    /* Allocate parameters' hashtable */
    ALLOC_HASHTABLE(retval);
    zend_hash_init(retval, paramCount, NULL, sap_rfc_parameter_ptr_dtor, 0);

    for (i = 0; i < paramCount; i++)
    {
        char *paramName; int paramNameLen;

        /* Allocate memory for current parameter */
        sp = ecalloc(1, sizeof(SAPRFC_PARAMETER_DESC));

        if (RFC_OK != RfcGetParameterDescByIndex(fdh, i, &sp->param, (RFC_ERROR_INFO*)err)) {
            SAP_ERR_RETURN_NULL(err, "RfcGetParameterDescByIndex");
        }

        /* Set default parameter's state */
        sp->state = SAPRFC_PARAM_DEFAULT;

        /* The key of the hashtable's new entry will be the utf8-converted parameter's name */
        if (SUCCESS != sapuc_to_utf8(sp->param.name, &paramName, &paramNameLen, err)) {
            return NULL;
        }

        /* Append parameter to parameters' hashtable */
        if (NULL == zend_hash_str_add_ptr(retval, paramName, paramNameLen, sp)) {
            sap_create_error(err, "SAP_INTERNAL_ERROR", RFC_UNKNOWN_ERROR, "Internal error: sap_function_description_to_array::zend_hash_str_add_ptr");
            SAP_ERR_RETURN_NULL(err, NULL);
        }

        efree(paramName);
    }

    return retval;
}

static php_sap_connection * sap_create_connection_resource(void)
{
    php_sap_connection *retval;

    retval = ecalloc(1, sizeof(php_sap_connection));

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

        efree(connection);
    }
}

static ZEND_RSRC_DTOR_FUNC(php_sap_connection_rsrc_dtor)
{
    php_sap_connection *connection = res->ptr;

    php_sap_connection_ptr_dtor(connection);
}

static php_sap_function * sap_create_function_from_descr_handle(RFC_FUNCTION_DESC_HANDLE fdh, SAPRFC_ERROR_INFO *err)
{
    php_sap_function *retval;

    retval = ecalloc(1, sizeof(php_sap_function));

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
    php_sap_function *func = res->ptr;

    php_sap_function_ptr_dtor(func);
}

static void sap_object_free_object_storage(zend_object *object)
{
    sap_object *intern = (sap_object*)((char*)(object) - object->handlers->offset);

    zend_object_std_dtor(&intern->std);

    if (intern->connection) {
        php_sap_connection_ptr_dtor(intern->connection);
        intern->connection = NULL;
    }
}

static zend_object * sap_object_create_object_ex(zend_class_entry *ce, sap_object **sap)
{
    size_t sz = sizeof(sap_object) + zend_object_properties_size(ce);
    sap_object *intern;

    intern = *sap = ecalloc(1, sz);

    intern->func_ce = sap_ce_SapFunction;

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &sap_object_handlers;

    return &intern->std;
}

static zend_object * sap_object_create_object(zend_class_entry *ce)
{
    sap_object *tmp;

    return sap_object_create_object_ex(ce, &tmp);
}

static zend_object * sap_object_clone_object(zval *object)
{
    zend_object *retval;
    sap_object *clintern, *intern = sap_get_sap_object(object);

    retval = sap_object_create_object_ex(Z_OBJCE_P(object), &clintern);

    if (intern->connection) {
        clintern->connection = intern->connection;
        clintern->connection->refCount++;
    }

    /* Clone properties */
    zend_objects_clone_members(&clintern->std, &intern->std);

    return retval;
}

static void sap_function_free_object_storage(zend_object *object)
{
    sap_function *intern = (sap_function*)((char*)(object) - object->handlers->offset);

    zend_object_std_dtor(&intern->std);

    if (intern->function_descr) {
        php_sap_function_ptr_dtor(intern->function_descr);
    }

    if (intern->connection) {
        php_sap_connection_ptr_dtor(intern->connection);
    }
}

static zend_object * sap_function_create_object_ex(zend_class_entry *ce, sap_function **func)
{
    sap_function *intern;

    intern = *func = ecalloc(1, sizeof(sap_function) + zend_object_properties_size(ce));

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &sap_function_handlers;

    return &intern->std;
}

static zend_object * sap_function_create_object(zend_class_entry *ce)
{
    sap_function *tmp;

    return sap_function_create_object_ex(ce, &tmp);
}

static zend_object * sap_function_clone_object(zval *object)
{
    zend_object *retval;
    sap_function *clintern, *intern = sap_get_function_object(object);
    SAPRFC_ERROR_INFO error;

    retval = sap_function_create_object_ex(Z_OBJCE_P(object), &clintern);

    /* Clone Function Parameters */
    if (intern->function_descr)
    {
        clintern->function_descr = sap_create_function_from_descr_handle(intern->function_descr->fdh, &error);

        if (NULL == clintern->function_descr) {
            zend_error(E_ERROR, "Could not clone function");
        }
    }

    if (intern->connection) {
        clintern->connection = intern->connection;
        clintern->connection->refCount++;
    }

    /* Clone properties */
    zend_objects_clone_members(&clintern->std, &intern->std);

    return retval;
}

static int sap_connection_open(php_sap_connection *connection, HashTable *lparams, SAPRFC_ERROR_INFO *err)
{
    RFC_CONNECTION_PARAMETER *params, *p;
    unsigned int paramCount, i;
    unsigned int l;

    paramCount = zend_hash_num_elements(lparams);
    params = p = ecalloc(paramCount, sizeof(RFC_CONNECTION_PARAMETER));

    do {
        zend_string *lpkey;
        zval *zlpvalue;

        ZEND_HASH_FOREACH_STR_KEY_VAL(lparams, lpkey, zlpvalue)
        {
            zval zcopy, *zcopy_p = &zcopy;

            /* We are only interested on parameters with a string key */
            if (NULL == lpkey) {
                paramCount--;
                continue;
            }

            /* param value must be string because it has to be converted later to a SAP_UC string */
            if (Z_TYPE_P(zlpvalue) != IS_STRING)
            {
                ZVAL_ZVAL(zcopy_p, zlpvalue, 1, 0);
                convert_to_string(zcopy_p);
                zlpvalue = zcopy_p;
            }

            if (SUCCESS != utf8_to_sapuc_l(Z_STRVAL_P(zlpvalue), Z_STRLEN_P(zlpvalue), (SAP_UC**)&p->value, &l, err)) {
                return FAILURE;
            }

            if (SUCCESS != utf8_to_sapuc_l(lpkey->val, lpkey->len, (SAP_UC**)&p->name, &l, err)) {
                return FAILURE;
            }

            if (UNEXPECTED(zcopy_p == zlpvalue)) {
                zval_ptr_dtor(zcopy_p);
            }

            p++;
        }
        ZEND_HASH_FOREACH_END();
    } while (0);

    connection->handle = RfcOpenConnection(params, paramCount, (RFC_ERROR_INFO*)err);

    for (i = 0, p = params; i < paramCount; i++, p++) {
        efree((void*)p->name); efree((void*)p->value);
    }

    efree(params);

    if (NULL == connection->handle) {
        SAP_ERR_RETURN_FAILURE(err, "RfcOpenConnection");
    }

    return SUCCESS;
}

static int sap_import_scalar(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err)
{
    switch (type)
    {
        case RFCTYPE_INT:
        case RFCTYPE_INT1:
        case RFCTYPE_INT2:
        case RFCTYPE_INT8:
        {
            zval zcopy, *zcopy_p = &zcopy;

            if (Z_TYPE_P(zvalue) != IS_LONG)
            {
                ZVAL_ZVAL(zcopy_p, zvalue, 1, 0);
                convert_to_long(zcopy_p);
                zvalue = zcopy_p;
            }

            if (Z_TYPE_P(zvalue) != IS_LONG)
            {
                SAP_UC *scalarTypeU16 = (SAP_UC*)RfcGetTypeAsString(type);
                char *scalarTypeU8;
                int scalarTypeU8Len;

                if (SUCCESS != sapuc_to_utf8(scalarTypeU16, &scalarTypeU8, &scalarTypeU8Len, err)) {
                    return FAILURE;
                }

                sap_create_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for type %s must be integer (%s given)", scalarTypeU8, zend_get_type_by_const(Z_TYPE_P(zvalue)));

                efree(scalarTypeU8);

                return FAILURE;
            }
            
            if (RFC_OK != RfcSetInt(dh, name, Z_LVAL_P(zvalue), (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcSetInt");
            }

            if (UNEXPECTED(zvalue == zcopy_p)) {
                zval_dtor(zcopy_p); /* Do we really need this? */
            }

            break;
        }
        case RFCTYPE_BCD:
        case RFCTYPE_DECF16:
        case RFCTYPE_DECF34:
        case RFCTYPE_FLOAT:
        {
            zval zcopy, *zcopy_p = &zcopy;

            if (Z_TYPE_P(zvalue) != IS_DOUBLE)
            {
                ZVAL_ZVAL(zcopy_p, zvalue, 1, 0);
                convert_to_double(zcopy_p);
                zvalue = zcopy_p;
            }

            if (Z_TYPE_P(zvalue) != IS_DOUBLE)
            {
                SAP_UC *scalarTypeU16 = (SAP_UC*)RfcGetTypeAsString(type);
                char *scalarTypeU8;
                int scalarTypeU8Len;

                if (SUCCESS != sapuc_to_utf8(scalarTypeU16, &scalarTypeU8, &scalarTypeU8Len, err)) {
                    return FAILURE;
                }

                sap_create_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for type %s must be double (%s given)", scalarTypeU8, zend_get_type_by_const(Z_TYPE_P(zvalue)));

                efree(scalarTypeU8);

                return FAILURE;
            }

            if (RFC_OK != RfcSetFloat(dh, name, Z_DVAL_P(zvalue), (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcSetFloat");
            }

            if (UNEXPECTED(zvalue == zcopy_p)) {
                zval_dtor(zcopy_p); /* Do we really need this? */
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

                sap_create_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for type %s must be a string (%s given)", scalarTypeU8, zend_get_type_by_const(Z_TYPE_P(zvalue)));

                return FAILURE;
            }

            if (RFC_OK != RfcSetBytes(dh, name, (SAP_RAW*)Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcSetBytes");
            }

            break;
        }
        case RFCTYPE_TIME:
        case RFCTYPE_DATE:
        {
            if (Z_TYPE_P(zvalue) == IS_OBJECT && instanceof_function(Z_OBJCE_P(zvalue), php_date_get_date_ce()))
            {
                zval rv;
                char *format = type == RFCTYPE_TIME ? "His" : "Ymd";
                RFC_CHAR *valU;
                unsigned int valUlen;

                if (format_datetime_object(zvalue, &rv, format) != SUCCESS)
                {
                    sap_create_error(err, "SAPRFC_CONVERSION_FAILURE", RFC_UNKNOWN_ERROR, "Could not format object of class %s to '%s'", Z_OBJCE_P(zvalue)->name->val, format);
                    return FAILURE;
                }

                if (SUCCESS != utf8_to_sapuc_l(Z_STRVAL(rv), Z_STRLEN(rv), &valU, &valUlen, err)) {
                    return FAILURE;
                }

                zval_dtor(&rv);

                if (RFC_OK != RfcSetChars(dh, name, valU, valUlen, (RFC_ERROR_INFO*)err)) {
                    SAP_ERR_RETURN_FAILURE(err, "RfcSetChars");
                }

                break;
            }
            else if (Z_TYPE_P(zvalue) == IS_STRING && Z_STRLEN_P(zvalue) == 0) {
                return SUCCESS;
            }
            /* else: go to case default */
        }
        default:
        {
            zval zcopy, *zcopy_p = &zcopy;
            unsigned int strU8len;
            SAP_UC *strU;
            unsigned int strU16len;

            if (Z_TYPE_P(zvalue) != IS_STRING)
            {
                ZVAL_ZVAL(zcopy_p, zvalue, 1, 0);
                convert_to_string(zcopy_p);
                zvalue = zcopy_p;
            }

            strU8len = strlenU8(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue));

            if (strU8len > length)
            {
                char *sub;
                int sublen;

                if (SUCCESS == utf8_substr(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), 0, length, &sub, &sublen))
                {
                    zval_dtor(zcopy_p);
                    ZVAL_STRINGL(zcopy_p, sub, sublen);
                    efree(sub);
                    zvalue = zcopy_p;
                }
            }

            if (SUCCESS != utf8_to_sapuc_l(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), &strU, &strU16len, err)) {
                return FAILURE;
            }

            if (UNEXPECTED(zvalue == zcopy_p)) {
                zval_dtor(zcopy_p);
            }

            if (RFC_OK != RfcSetChars(dh, name, strU, strU16len, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcSetChars");
            }

            break;
        }
    }
    return SUCCESS;
}

static int sap_import_structure(RFC_STRUCTURE_HANDLE sh, RFC_TYPE_DESC_HANDLE tdh, HashTable *fields, SAPRFC_ERROR_INFO *err)
{
    unsigned int fieldCount, i;

    if (RFC_OK != RfcGetFieldCount(tdh, &fieldCount, (RFC_ERROR_INFO*)err)) {
        SAP_ERR_RETURN_FAILURE(err, "RfcGetFieldCount");
    }

    for (i = 0; i < fieldCount; i++)
    {
        char *fname;
        int fnamelen;
        RFC_FIELD_DESC field;
        zval *zfvalue;

        if (RFC_OK != RfcGetFieldDescByIndex(tdh, i, &field, (RFC_ERROR_INFO*)err)) {
            SAP_ERR_RETURN_FAILURE(err, "RfcGetFieldDescByIndex");
        }

        if (SUCCESS != sapuc_to_utf8(field.name, &fname, &fnamelen, err)) {
            return FAILURE;
        }

        
        if (zfvalue = zend_hash_str_find(fields, fname, fnamelen)) {
            if (SUCCESS != sap_import(sh, field.name, field.type, field.typeDescHandle, field.nucLength, zfvalue, err)) {
                return FAILURE;
            }
        }

        efree(fname);
    }

    return SUCCESS;
}

static int sap_import_table(RFC_TABLE_HANDLE th, RFC_TYPE_DESC_HANDLE tdh, HashTable *rows, SAPRFC_ERROR_INFO *err)
{
    zval *zrow;

    ZEND_HASH_FOREACH_VAL(rows, zrow)
    {
        RFC_STRUCTURE_HANDLE sh;

        ZVAL_DEREF(zrow);

        /* ignore non-array table rows */
        if (Z_TYPE_P(zrow) != IS_ARRAY) {
            continue;
        }

        if (NULL == (sh = RfcAppendNewRow(th, (RFC_ERROR_INFO*)err))) {
            SAP_ERR_RETURN_FAILURE(err, "RfcAppendNewRow");
        }

        if (SUCCESS != sap_import_structure(sh, tdh, Z_ARRVAL_P(zrow), err)) {
            return FAILURE;
        }
    }
    ZEND_HASH_FOREACH_END();

    return SUCCESS;
}

static int sap_import(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *zvalue, SAPRFC_ERROR_INFO *err)
{
    ZVAL_DEREF(zvalue);

    switch (type)
    {
        case RFCTYPE_TABLE:
        {
            RFC_TABLE_HANDLE th;

            if (UNEXPECTED(Z_TYPE_P(zvalue) != IS_ARRAY)) {
                sap_create_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for a RFCTYPE_TABLE must be an array (%s given)", zend_get_type_by_const(Z_TYPE_P(zvalue)));
                return FAILURE;
            }

            if (RFC_OK != RfcGetTable(dh, name, &th, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetTable");
            }

            return sap_import_table(th, tdh, Z_ARRVAL_P(zvalue), err);
        }
        case RFCTYPE_STRUCTURE:
        {
            RFC_STRUCTURE_HANDLE sh;

            if (Z_TYPE_P(zvalue) != IS_ARRAY) {
                sap_create_error(err, "SAPRFC_INVALID_PARAMETER", -1, "Value for a RFCTYPE_STRUCTURE must be an array (%s given)", zend_get_type_by_const(Z_TYPE_P(zvalue)));
                return FAILURE;
            }

            if (RFC_OK != RfcGetStructure(dh, name, &sh, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetStructure");
            }

            return sap_import_structure(sh, tdh, Z_ARRVAL_P(zvalue), err);
        }
        default:
        {
            if (Z_TYPE_P(zvalue) == IS_NULL) {
                return SUCCESS; /* Leave default value */
            }

            return sap_import_scalar(dh, name, type, length, zvalue, err);
        }
    }
}

static int sap_export_scalar(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, unsigned int nucLength, zval *rv, TRIM_TYPE strTrimType, SAPRFC_ERROR_INFO *err)
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
                SAP_ERR_RETURN_FAILURE(err, "RfcGetInt");
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
                SAP_ERR_RETURN_FAILURE(err, "RfcGetFloat");
            }

            ZVAL_DOUBLE(rv, fval);

            break;
        }
        case RFCTYPE_BYTE:
        {
            RFC_BYTE *buffer = ecalloc(1, nucLength * sizeof(RFC_BYTE));

            if (RFC_OK != RfcGetBytes(dh, name, buffer, nucLength, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetBytes");
            }

            ZVAL_STRINGL(rv, (char*)buffer, nucLength);
            efree(buffer);

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

            rawBuffer = ecalloc(1, rawBufferLen);

            switch (RfcGetXString(dh, name, rawBuffer, rawBufferLen, &rawBufferLen, (RFC_ERROR_INFO*)err))
            {
                case RFC_BUFFER_TOO_SMALL:
                    /*
                    * if RFC_BUFFER_TOO_SMALL then the rawBufferLen variable contains the required number of bytes
                    * to properly export this RFCTYPE_XSTRING value
                    */
                    efree(rawBuffer);

                    goto try_again_xstring;

                case RFC_OK: break;

                default:
                    efree(rawBuffer);
                    SAP_ERR_RETURN_FAILURE(err, "RfcGetXString");
            }

            ZVAL_STRINGL(rv, (char*)rawBuffer, rawBufferLen);
            efree(rawBuffer);

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

            /* The result will be null-terminated by the nw library so we have to allocate one more SAP_UC char */
            bufferU16len++;
            bufferU16 = ecalloc(1, bufferU16len * sizeof(SAP_UC));

            switch (RfcGetString(dh, name, bufferU16, bufferU16len, &bufferU16len, (RFC_ERROR_INFO*)err))
            {
                case RFC_BUFFER_TOO_SMALL:
                    /*
                    * if RFC_BUFFER_TOO_SMALL then the numChars variable contains the required number of SAP_UC characters
                    * to properly export this RFCTYPE_STRING value
                    */
                    efree(bufferU16);

                    goto try_again_string;
                case RFC_OK: break;

                default:
                    efree(bufferU16);
                    SAP_ERR_RETURN_FAILURE(err, "RfcGetString");
            }

            if (SUCCESS != sapuc_to_utf8_l(bufferU16, bufferU16len, &str, &len, err)) {
                return FAILURE;
            }

            efree(bufferU16);

            if (len > 0) {
                ZVAL_STRINGL(rv, str, len);
            }
            else {
                ZVAL_NULL(rv);
            }

            efree(str);

            break;
        }
        case RFCTYPE_DATE:
        {
            RFC_DATE dt;
            char *str;
            int len;

            memset(dt, 0, sizeof(RFC_DATE));

            if (RFC_OK != RfcGetDate(dh, name, dt, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetDate");
            }

            if (SUCCESS != sapuc_to_utf8_l(dt, nucLength, &str, &len, err)) {
                return FAILURE;
            }

            if (len > 0) {
                ZVAL_STRINGL(rv, str, len);
            }
            else {
                ZVAL_NULL(rv);
            }

            efree(str);

            break;
        }
        default:
        {
            RFC_CHAR *uChars = ecalloc(1, nucLength * sizeof(RFC_CHAR));
            char *str;
            int len;

            if (RFC_OK != RfcGetChars(dh, name, uChars, nucLength, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetChars");
            }

            if (SUCCESS != sapuc_to_utf8_l(uChars, nucLength, &str, &len, err)) {
                return FAILURE;
            }

            efree(uChars);

            if (strTrimType != TRIM_NONE)
            {
                char *trimmed_str = NULL;
                int trimmed_str_len;

                utf8_trim(str, len, &trimmed_str, &trimmed_str_len, strTrimType);

                if (NULL != trimmed_str)
                {
                    efree(str);

                    str = trimmed_str;
                    len = trimmed_str_len;
                }
            }

            if (len > 0) {
                ZVAL_STRINGL(rv, str, len);
            }
            else {
                ZVAL_NULL(rv);
            }

            efree(str);

            break;
        }
    }

    return SUCCESS;
}

static int sap_export_structure(RFC_STRUCTURE_HANDLE sh, RFC_TYPE_DESC_HANDLE tdh, zval *rv, TRIM_TYPE strTrimType, SAPRFC_ERROR_INFO *err)
{
    int isArrayAccess = FALSE;
    unsigned int fieldCount, i;

    if (RFC_OK != RfcGetFieldCount(tdh, &fieldCount, (RFC_ERROR_INFO*)err)) {
        SAP_ERR_RETURN_FAILURE(err, "RfcGetFieldCount");
    }

    array_init_size(rv, fieldCount);

    for (i = 0; i < fieldCount; i++)
    {
        RFC_FIELD_DESC field;
        char *fname;
        int fnamelen;
        zval *pfval = _NEWZVAL;

        if (RFC_OK != RfcGetFieldDescByIndex(tdh, i, &field, (RFC_ERROR_INFO*)err)) {
            SAP_ERR_RETURN_FAILURE(err, "RfcGetFieldDescByIndex");
        }

        if (SUCCESS != sapuc_to_utf8(field.name, &fname, &fnamelen, err)) {
            return FAILURE;
        }

        pfval = zend_hash_str_add_new(Z_ARRVAL_P(rv), fname, fnamelen, pfval);

        efree(fname);

        if (NULL == pfval) { /* Out of memory, full table etc... */
            break;
        }

        if (SUCCESS != sap_export(sh, field.name, field.type, field.typeDescHandle, field.nucLength, pfval, strTrimType, err)) {
            return FAILURE;
        }
    }

    return SUCCESS;
}

static int sap_export_table(RFC_TABLE_HANDLE th, RFC_TYPE_DESC_HANDLE tdh, zval *rv, TRIM_TYPE strTrimType, SAPRFC_ERROR_INFO *err)
{
    int retval = SUCCESS;
    unsigned int rowCount;
    RFC_ERROR_INFO e;
    RFC_RC rc;

    if (RFC_OK != RfcGetRowCount(th, &rowCount, (RFC_ERROR_INFO*)err)) {
        SAP_ERR_RETURN_FAILURE(err, "RfcGetRowCount");
    }

    array_init_size(rv, rowCount);

    for (rc = RfcMoveToFirstRow(th, &e); rc == RFC_OK; rc = RfcMoveToNextRow(th, &e))
    {
        RFC_STRUCTURE_HANDLE sh;
        zval *pzrow;

        if (NULL == (sh = RfcGetCurrentRow(th, (RFC_ERROR_INFO*)err))) {
            SAP_ERR_RETURN_FAILURE(err, "RfcGetCurrentRow");
        }

        
        if (!(pzrow = zend_hash_next_index_insert_new(Z_ARRVAL_P(rv), _NEWZVAL))) { /* Out of memory, full hashtable etc... */
            break;
        }

        if (SUCCESS != sap_export_structure(sh, tdh, pzrow, strTrimType, err TSRMLS_CC)) {
            return FAILURE;
        }
    }

    return retval;
}

static int sap_export(DATA_CONTAINER_HANDLE dh, SAP_UC *name, RFCTYPE type, RFC_TYPE_DESC_HANDLE tdh, unsigned int length, zval *rv, TRIM_TYPE strTrimType, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
    switch (type)
    {
        case RFCTYPE_TABLE:
        {
            RFC_TABLE_HANDLE th;

            if (RFC_OK != RfcGetTable(dh, name, &th, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetTable");
            }

            return sap_export_table(th, tdh, rv, strTrimType, err TSRMLS_CC);
        }
        case RFCTYPE_STRUCTURE:
        {
            RFC_STRUCTURE_HANDLE sh;

            if (RFC_OK != RfcGetStructure(dh, name, &sh, (RFC_ERROR_INFO*)err)) {
                SAP_ERR_RETURN_FAILURE(err, "RfcGetTable");
            }

            return sap_export_structure(sh, tdh, rv, strTrimType, err TSRMLS_CC);
        }
        default: {
            return sap_export_scalar(dh, name, type, length, rv, strTrimType, err TSRMLS_CC);
        }
    }
}

static php_sap_function * sap_fetch_function(char *name, int nameLen, php_sap_connection *connection, SAPRFC_ERROR_INFO *err TSRMLS_DC)
{
    SAP_UC *nameU;
    unsigned int nameUlen;
    RFC_FUNCTION_DESC_HANDLE fdh;
    int connectionIsValid = 0;

    if (NULL == connection->handle ||
        RFC_OK != RfcIsConnectionHandleValid(connection->handle, &connectionIsValid, (RFC_ERROR_INFO*)err) ||
        !connectionIsValid
    ) {
        sap_create_error(err, "SAPRFC_INVALID_CONNECTION", -1, "There is no active connection to a SAP System");
        return NULL;
    }

    if (SUCCESS != utf8_to_sapuc_l(name, nameLen, &nameU, &nameUlen, err)) {
        return NULL;
    }

    fdh = RfcGetFunctionDesc(connection->handle, nameU, (RFC_ERROR_INFO*)err);
    efree(nameU);

    if (NULL == fdh) {
        SAP_ERR_RETURN_NULL(err, "RfcGetFunctionDesc");
    }

    return sap_create_function_from_descr_handle(fdh, err TSRMLS_CC);
}

static int sap_function_invoke(php_sap_function *function, php_sap_connection *connection, HashTable *imports, HashTable *exports, TRIM_TYPE strTrimType, SAPRFC_ERROR_INFO *err)
{
    RFC_FUNCTION_HANDLE fh;
    zend_string *pkey;
    SAPRFC_PARAMETER_DESC *sp;

    if (NULL == (fh = RfcCreateFunction(function->fdh, (RFC_ERROR_INFO*)err))) {
        SAP_ERR_RETURN_FAILURE(err, "RfcCreateFunction");
    }

    ZEND_HASH_FOREACH_STR_KEY_PTR(function->params, pkey, sp)
    {
        zval *zpvalue;

        if (sp->state != SAPRFC_PARAM_DEFAULT && RFC_OK != RfcSetParameterActive(fh, sp->param.name, sp->state, (RFC_ERROR_INFO*)err)) {
            SAP_ERR_RETURN_FAILURE(err, "RfcSetParameterActive");
        }

        if (NULL == imports || !(sp->param.direction & RFC_IMPORT) || sp->state == SAPRFC_PARAM_INACTIVE) {
            continue;
        }


        if (NULL == (zpvalue = zend_hash_find(imports, pkey))) {
            continue;
        }

        if (SUCCESS != sap_import(fh, sp->param.name, sp->param.type, sp->param.typeDescHandle, sp->param.nucLength, zpvalue, err)) {
            return FAILURE;
        }
    }
    ZEND_HASH_FOREACH_END();

    if (RFC_OK != RfcInvoke(connection->handle, fh, (RFC_ERROR_INFO*)err)) {
        SAP_ERR_RETURN_FAILURE(err, "RfcInvoke");
    }

    ZEND_HASH_FOREACH_STR_KEY_PTR(function->params, pkey, sp)
    {
        zval *ev;

        if (!(sp->param.direction & RFC_EXPORT) || sp->state == SAPRFC_PARAM_INACTIVE) {
            continue;
        }
        
        if (NULL == (ev = zend_hash_add(exports, pkey, _NEWZVAL))) { /* out of memory, full hashtable etc */
            break;
        }

        if (SUCCESS != sap_export(fh, sp->param.name, sp->param.type, sp->param.typeDescHandle, sp->param.nucLength, ev, strTrimType, err)) {
            return FAILURE;
        }
    }
    ZEND_HASH_FOREACH_END();

    RfcDestroyFunction(fh, (RFC_ERROR_INFO*)err);

    return SUCCESS;
}

/**
 * Creates a connection to a R/3 backend system
 * 
 * @param array $logonParameters
 * 
 * @return resource
 *
 * @throws SapCommunicationException
 */
PHP_FUNCTION(sap_connect)
{
    zval *logonParameters;
    php_sap_connection *crsrc;
    SAPRFC_ERROR_INFO err;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &logonParameters) == FAILURE) {
        return;
    }

    if (zend_hash_num_elements(Z_ARRVAL_P(logonParameters)) == 0)
    {
        zval ex = sap_create_exception(php_sap_invalid_args_exception, -1, "Logon parameters array must not be empty");

        zend_throw_exception_object(&ex);

        return;
    }

    crsrc = sap_create_connection_resource();
    crsrc->refCount++;

    if (SUCCESS != sap_connection_open(crsrc, Z_ARRVAL_P(logonParameters), &err))
    {
        zval ex = sap_error_to_exception(&err);

        php_sap_connection_ptr_dtor(crsrc);

        zend_throw_exception_object(&ex);

        return;
    }

    ZVAL_RES(return_value, zend_register_resource(crsrc, le_php_sap_connection));
}

/**
 * Invokes a remote function module to a backend system
 *
 * @param string        $name       remote function module's name
 * @param resource      $connection SAP RFC connection
 * @param array|null    $imports    IMPORT/CHANGING/TABLE parameters
 *
 * @return array|null               EXPORT/CHANGING/TABLE parameters
 *
 * @throws InvalidArgumentException
 * @throws SapException
 */
PHP_FUNCTION(sap_invoke_function)
{
    char *name;
    int namelen;
    zval *zcrsrc, *zimports = NULL;
    zend_bool rtrim = PHP_SAP_GLOBALS(rtrim_export_strings);
    HashTable *imports = NULL;
    php_sap_function *frsrc;
    php_sap_connection *crsrc;
    TRIM_TYPE trimType = TRIM_NONE;
    int result;
    SAPRFC_ERROR_INFO err;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "sr|ab", &name, &namelen, &zcrsrc, &zimports, &rtrim) == FAILURE) {
        return;
    }

    if (NULL == (crsrc = zend_fetch_resource(Z_RES_P(zcrsrc), PHP_SAP_CONNECTION_RES_NAME, le_php_sap_connection))) {
        zval ex = sap_create_exception(php_sap_invalid_args_exception, -1, "Invalid connection resource");
        zend_throw_exception_object(&ex);
        return;
    }

    if (NULL == (frsrc = sap_fetch_function(name, namelen, crsrc, &err))) {
        zval ex = sap_error_to_exception(&err);
        zend_throw_exception_object(&ex);
        return;
    }

    if (NULL != zimports) {
        imports = Z_ARRVAL_P(zimports);
    }

    array_init(return_value);
    
    if (rtrim) {
        trimType = TRIM_RIGHT;
    }

    result = sap_function_invoke(frsrc, crsrc, imports, Z_ARRVAL_P(return_value), trimType, &err);

    php_sap_function_ptr_dtor(frsrc);

    if (SUCCESS != result) {
        zval ex = sap_error_to_exception(&err);
        zend_throw_exception_object(&ex);
        return;
    }
}

PHP_METHOD(SapException, getMessageKey)
{
    zval *zMessageKey = sap_read_property("KEY");

    RETURN_ZVAL(zMessageKey, 1, 0);
}

PHP_METHOD(SapException, getMessageType)
{
    zval *zMessageType = sap_read_property("MSGTY");
    
    RETURN_ZVAL(zMessageType, 1, 0);
}

PHP_METHOD(SapException, getMessageId)
{
    zval *zMessageId = sap_read_property("MSGID"); 

    RETURN_ZVAL(zMessageId, 1, 0);
}

PHP_METHOD(SapException, getMessageNumber)
{
    zval *zMessageNumber = sap_read_property("MSGNO"); 

    RETURN_ZVAL(zMessageNumber, 1, 0);
}

PHP_METHOD(SapException, getMessageVar1)
{
    zval *zMessageVar1 = sap_read_property("MSGV1"); 

    RETURN_ZVAL(zMessageVar1, 1, 0);
}

PHP_METHOD(SapException, getMessageVar2)
{
    zval *zMessageVar2 = sap_read_property("MSGV2"); 

    RETURN_ZVAL(zMessageVar2, 1, 0);
}

PHP_METHOD(SapException, getMessageVar3)
{
    zval *zMessageVar3 = sap_read_property("MSGV3"); 

    RETURN_ZVAL(zMessageVar3, 1, 0);
}

PHP_METHOD(SapException, getMessageVar4)
{
    zval *zMessageVar4 = sap_read_property("MSGV4"); 

    RETURN_ZVAL(zMessageVar4, 1, 0);
}

PHP_METHOD(SapException, getNwSdkFunction)
{
    zval *zNwSdkFunction = sap_read_property("nwsdkfunction"); 

    RETURN_ZVAL(zNwSdkFunction, 1, 0);
}

PHP_METHOD(Sap, __construct)
{
    sap_object *intern;
    zval *zlogonParameters = NULL;
    SAPRFC_ERROR_INFO error;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &zlogonParameters) == FAILURE) {
        return;
    }

    intern = sap_get_sap_object(getThis());

    /* if constructor is called again, destroy previous connection */
    if (intern->connection) {
        php_sap_connection_ptr_dtor(intern->connection);
    }

    intern->connection = sap_create_connection_resource();
    intern->connection->refCount++;

    if (SUCCESS != sap_connection_open(intern->connection, Z_ARRVAL_P(zlogonParameters), &error)) {
        zval ex = sap_error_to_exception(&error);
        zend_throw_exception_object(&ex);
        return;
    }
}

PHP_METHOD(Sap, setFunctionClass)
{
    sap_object *intern;
    zend_class_entry *fce = php_sap_get_function_ce();

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "C", &fce) == FAILURE) {
        return;
    }

    intern = sap_get_sap_object(getThis());
    intern->func_ce = fce;
}

PHP_METHOD(Sap, call)
{
    char *name;
    int namelen;
    zval *zimports = NULL;
    zend_bool rtrim = PHP_SAP_GLOBALS(rtrim_export_strings);
    sap_object *intern;
    HashTable *imports = NULL;
    php_sap_function *func;
    TRIM_TYPE trimType = TRIM_NONE;
    int cresult;
    SAPRFC_ERROR_INFO error;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s|ab", &name, &namelen, &zimports, &rtrim) == FAILURE) {
        return;
    }

    intern = sap_get_sap_object(getThis());

    if (NULL == intern->connection) {
        zval ex = sap_create_exception(spl_ce_LogicException, -1, "There is no connection to a SAP R/3 system");
        zend_throw_exception_object(&ex);
        return;
    }

    /* Fetch function's information */
    if (NULL == (func = sap_fetch_function(name, namelen, intern->connection, &error))) {
        zval ex = sap_error_to_exception(&error);
        zend_throw_exception_object(&ex);
        return;
    }

    if (NULL != zimports) {
        imports = Z_ARRVAL_P(zimports);
    }

    /* Exports will be stored in return_value's hashtable */
    array_init(return_value);

    if (rtrim) {
        trimType = TRIM_RIGHT;
    }

    /* Invoke the function module */
    cresult = sap_function_invoke(func, intern->connection, imports, Z_ARRVAL_P(return_value), trimType, &error);

    /* Free function's resources */
    php_sap_function_ptr_dtor(func);

    /* Call failed */
    if (SUCCESS != cresult) {
        zval ex = sap_error_to_exception(&error);
        zend_throw_exception_object(&ex);
        return;
    }
}

PHP_METHOD(Sap, fetchFunction)
{
    zval *zfunction;
    char *function_name;
    int function_len;
    zend_class_entry *fce = sap_ce_SapFunction;
    zval *zargs = NULL;
    sap_object *intern;
    php_sap_function *function_descr_rsrc;
    sap_function *func;
    SAPRFC_ERROR_INFO error;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z|Ca", &zfunction, &fce, &zargs) == FAILURE) {
        return;
    }

    switch (Z_TYPE_P(zfunction))
    {
        case IS_STRING:
            function_name = estrndup(Z_STRVAL_P(zfunction), Z_STRLEN_P(zfunction));
            function_len = Z_STRLEN_P(zfunction);
            break;
        case IS_OBJECT:
            if (instanceof_function(Z_OBJCE_P(zfunction), sap_ce_SapFunction))
            {
                /**
                * If the first argument is a SapFunction object, get function's name
                * by calling SapFunction::getName method
                */
                zval rv;

                /* all functions/methods for php objects are stored in lower case */
                if (SUCCESS != sap_call_object_method(zfunction, Z_OBJCE_P(zfunction), "getname", NULL, NULL, &rv)) {
                    return;
                }

                if (Z_TYPE(rv) != IS_STRING) {
                    zend_error(E_ERROR, "Method %s::getName() should return a string (%s returned)", fce->name->val, zend_get_type_by_const(Z_TYPE(rv)));
                    return;
                }

                function_name = estrndup(Z_STRVAL(rv), Z_STRLEN(rv));
                function_len = Z_STRLEN(rv);

                zval_ptr_dtor(&rv);

                break;
            }
        default: {
            zval ex = sap_create_exception(
                php_sap_invalid_args_exception, 
                -1, 
                "Argument 1 of Sap::fetchFunction() must be a string or a SapFunction object (%s given)",
                zend_get_type_by_const(Z_TYPE_P(zfunction))
            );

            zend_throw_exception_object(&ex);
            return;
        }
    }

    intern = sap_get_sap_object(getThis());

    /* Get function description from SAP backend */
    function_descr_rsrc = sap_fetch_function(function_name, function_len, intern->connection, &error);

    efree(function_name);

    if (NULL == function_descr_rsrc) {
        zval ex = sap_error_to_exception(&error);
        zend_throw_exception_object(&ex);
        return;
    }

    if (Z_TYPE_P(zfunction) == IS_OBJECT) {
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
            const char *constructor_func_name = ZSTR_VAL(fce->constructor->common.function_name);
            zval rv;

            retval = sap_call_object_method(return_value, fce, constructor_func_name, fce->constructor, zargs, &rv TSRMLS_CC);

            /* we are not interested in constructor's return value */
            zval_ptr_dtor(&rv);

            if (SUCCESS != retval) {
                php_error(E_ERROR, "Could not call '%s' object's constructor", fce->name->val);
                return;
            }
        }
    }

    /* Fetch the sap_function object from the return value */
    func = sap_get_function_object(return_value);

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

PHP_METHOD(Sap, getAttributes)
{
    sap_object *intern = sap_get_sap_object(getThis());
    RFC_ATTRIBUTES attributes;
    SAPRFC_ERROR_INFO err;
    char *utf8;
    int utf8len;

    if (NULL == intern->connection) {
        zend_error_noreturn(E_ERROR, "No active connection found");
    }

    if (RFC_OK != RfcGetConnectionAttributes(intern->connection->handle, &attributes, (RFC_ERROR_INFO*)&err))
    {
        zval ex;

        SAP_ERROR_SET_RFCFUNCTION(&err, "RfcGetConnectionAttributes", sizeof("RfcGetConnectionAttributes") - 1);

        ex = sap_error_to_exception(&err);
        zend_throw_exception_object(&ex);
        return;
    }

    array_init(return_value);

    if (SUCCESS == sapuc_to_utf8(attributes.client, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "client", utf8, utf8len);
        efree(utf8);
    }
    
    if (SUCCESS == sapuc_to_utf8(attributes.codepage, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "codepage", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.cpicConvId, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "cpicConvId", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.dest, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "dest", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.host, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "host", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.isoLanguage, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "isoLanguage", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.kernelRel, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "kernelRel", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.language, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "language", utf8, utf8len);
        efree(utf8);
    }
    /*
    if (SUCCESS == sapuc_to_utf8(attributes.partnerBytesPerChar, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "partnerBytesPerChar", utf8, utf8len);
        efree(utf8);
    }
    */
    if (SUCCESS == sapuc_to_utf8(attributes.partnerCodepage, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "partnerCodepage", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.partnerHost, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "partnerHost", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.partnerRel, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "partnerRel", utf8, utf8len);
        efree(utf8);
    }
    /*
    if (SUCCESS == sapuc_to_utf8(attributes.partnerSystemCodepage, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "partnerSystemCodepage", utf8, utf8len);
        efree(utf8);
    }
    */
    if (SUCCESS == sapuc_to_utf8(attributes.partnerType, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "partnerType", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.progName, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "progName", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.rel, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "rel", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.rfcRole, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "rfcRole", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.sysId, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "sysId", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.sysNumber, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "sysNumber", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.trace, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "trace", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.type, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "type", utf8, utf8len);
        efree(utf8);
    }

    if (SUCCESS == sapuc_to_utf8(attributes.user, &utf8, &utf8len, &err)) {
        add_assoc_stringl(return_value, "user", utf8, utf8len);
        efree(utf8);
    }
}

PHP_METHOD(SapFunction, getName)
{
    sap_function *intern;
    RFC_ABAP_NAME funcNameU;
    char *name;
    int namelen;
    SAPRFC_ERROR_INFO error;

    intern = sap_get_function_object(getThis());

    if (NULL == intern->function_descr) {
        RETURN_STRING("");
    }

    if (RFC_OK != RfcGetFunctionName(intern->function_descr->fdh, funcNameU, (RFC_ERROR_INFO*)&error))
    {
        zval ex;

        SAP_ERROR_SET_RFCFUNCTION(&error, "RfcGetFunctionName", sizeof("RfcGetFunctionName") - 1);

        ex = sap_error_to_exception(&error);
        zend_throw_exception_object(&ex);
        return;
    }

    if (SUCCESS != sapuc_to_utf8(funcNameU, &name, &namelen, &error)) {
        zval ex = sap_error_to_exception(&error);
        zend_throw_exception_object(&ex);
        return;
    }

    ZVAL_STRINGL(return_value, name, namelen);

    efree(name);
}

PHP_METHOD(SapFunction, setActive)
{
    zend_string *pname;
    zend_bool isActive = SAPRFC_PARAM_DEFAULT;
    sap_function *intern;
    SAPRFC_PARAMETER_DESC *sp;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S|b", &pname, &isActive) == FAILURE) {
        return;
    }

    intern = sap_get_function_object(getThis());

    if (!intern->function_descr) {
        RETURN_FALSE;
    }

    if (sp = zend_hash_find_ptr(intern->function_descr->params, pname)) {
        sp->state = isActive;
        RETURN_TRUE;
    }

    RETURN_FALSE;
}

PHP_METHOD(SapFunction, __invoke)
{
    zval *args = NULL;
    zend_bool rtrim = PHP_SAP_GLOBALS(rtrim_export_strings);
    HashTable *imports = NULL;
    TRIM_TYPE trimType = TRIM_NONE;
    sap_function *intern;
    SAPRFC_ERROR_INFO error;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|ab", &args, &rtrim) == FAILURE) {
        return;
    }

    if (NULL != args) {
        imports = Z_ARRVAL_P(args);
    }

    intern = sap_get_function_object(getThis());

    if (NULL == intern->function_descr || NULL == intern->connection) {
        zval ex = sap_create_exception(spl_ce_LogicException, -1, "Function's descriptions has not been fetced");
        zend_throw_exception_object(&ex);
        return;
    }

    if (rtrim) {
        trimType = TRIM_RIGHT;
    }

    array_init(return_value);

    if (SUCCESS != sap_function_invoke(intern->function_descr, intern->connection, imports, Z_ARRVAL_P(return_value), trimType, &error))
    {
        zval ex = sap_error_to_exception(&error);

        zval_dtor(return_value);
        zend_throw_exception_object(&ex);
        return;
    }
}

PHP_METHOD(SapFunction, getParameters)
{
    sap_function *intern;
    zend_string *pkey;

    intern = sap_get_function_object(getThis());

    if (!intern->function_descr) {
        zval ex = sap_create_exception(spl_ce_LogicException, -1, "Function's descriptions has not been fetced");
        zend_throw_exception_object(&ex);
        return;
    }

    array_init_size(return_value, zend_hash_num_elements(intern->function_descr->params));

    ZEND_HASH_FOREACH_STR_KEY(intern->function_descr->params, pkey)
    {
        zval zpkey;

        ZVAL_STR(&zpkey, pkey);
        
        if (NULL == zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &zpkey)) {
            zval_ptr_dtor(&zpkey);
        }
    }
    ZEND_HASH_FOREACH_END();
}

PHP_METHOD(SapFunction, getTypeName)
{
    zend_string *pname;
    sap_function *intern;
    SAPRFC_PARAMETER_DESC *sp;
    SAPRFC_ERROR_INFO error;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S", &pname) == FAILURE) {
        return;
    }

    intern = sap_get_function_object(getThis());

    if (!intern->function_descr) {
        zval ex = sap_create_exception(spl_ce_LogicException, -1, "Function's descriptions has not been fetced");
        zend_throw_exception_object(&ex);
        return;
    }

    if (NULL == (sp = zend_hash_find_ptr(intern->function_descr->params, pname))) {
        zval ex = sap_create_exception(spl_ce_LogicException, -1, "Parameter not found");
        zend_throw_exception_object(&ex);
        return;
    }

    if (sp->param.type == RFCTYPE_TABLE || sp->param.type == RFCTYPE_STRUCTURE)
    {
        RFC_ABAP_NAME typeNameU;
        char *typeName;
        int typeNameLen;

        memset(&error, 0, sizeof(SAPRFC_ERROR_INFO));

        if (RFC_OK != RfcGetTypeName(sp->param.typeDescHandle, typeNameU, (RFC_ERROR_INFO*)&error)) {
            zval ex = sap_error_to_exception(&error);
            zend_throw_exception_object(&ex);
            return;
        }

        if (SUCCESS != sapuc_to_utf8(typeNameU, &typeName, &typeNameLen, &error)) {
            zval ex = sap_error_to_exception(&error);
            zend_throw_exception_object(&ex);
            return;
        }

        RETURN_STRINGL(typeName, typeNameLen);
        efree(typeName);
    }
}

PHP_METHOD(SapFunction, __toString)
{
    sap_call_object_method(getThis(), Z_OBJCE_P(getThis()), "getname", NULL, NULL, return_value);
}

PHP_METHOD(SapRfcReadTable, getName)
{
    RETURN_STRING("RFC_READ_TABLE");
}

PHP_METHOD(SapRfcReadTable, select)
{
    zval *zfields, *zfieldshelper = NULL;
    char *tableName;
    int tableNameLen;
    zval *zwhere = NULL;
    int rowCount = 0;
    int offset = 0;
    zend_bool rtrim = PHP_SAP_GLOBALS(rtrim_export_strings);
    zval __invoke_args;
    zval *pzimports;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "zs|allb", &zfields, &tableName, &tableNameLen, &zwhere, &rowCount, &offset, &rtrim) == FAILURE) {
        return;
    }

    /* The array of arguments that will be used to call the __invoke method of this SapRfcReadTable object */
    array_init(&__invoke_args);

    /* The first (and only) argument of __invoke_args array, will be the RFC_READ_TABLE import parameters */
    if (NULL == (pzimports = zend_hash_next_index_insert_new(Z_ARRVAL(__invoke_args), _NEWZVAL))) {
        /* Appending a zval to __invoke_args array failed */
        zval_dtor(&__invoke_args);
        return;
    }

    /* Initilize the import parameters array */
    array_init(pzimports);

    /* Create the QUERY_TABLE parameter */
    {
        zval *zparam_QUERY_TABLE = zend_hash_str_add_new(Z_ARRVAL_P(pzimports), "QUERY_TABLE", sizeof("QUERY_TABLE") - 1, _NEWZVAL);

        if (NULL != zparam_QUERY_TABLE) {
            ZVAL_STRINGL(zparam_QUERY_TABLE, tableName, tableNameLen);
        }
    }

    /* Create the DELIMITER parameter and set it to "" */
    {
        zval *zparam_DELIMITER = zend_hash_str_add_new(Z_ARRVAL_P(pzimports), "DELIMITER", sizeof("DELIMITER") - 1, _NEWZVAL);

        if (NULL != zparam_DELIMITER) {
            ZVAL_STRING(zparam_DELIMITER, "");
        }
    }

    /* Create the ROWCOUNT parameter */
    if (rowCount > 0)
    {
        zval *zparam_ROWCOUNT = zend_hash_str_add_new(Z_ARRVAL_P(pzimports), "ROWCOUNT", sizeof("ROWCOUNT") - 1, _NEWZVAL);

        if (NULL != zparam_ROWCOUNT) {
            ZVAL_LONG(zparam_ROWCOUNT, rowCount);
        }
    }

    /* Create the ROWSKIPS parameter */
    if (offset > 0)
    {
        zval *zparam_ROWSKIPS = zend_hash_str_add_new(Z_ARRVAL_P(pzimports), "ROWSKIPS", sizeof("ROWSKIPS") - 1, _NEWZVAL);

        if (NULL != zparam_ROWSKIPS) {
            ZVAL_LONG(zparam_ROWSKIPS, offset);
        }
    }

    /* Create the FIELDS parameter */
    {
        zval *zparam_FIELDS = zend_hash_str_add_new(Z_ARRVAL_P(pzimports), "FIELDS", sizeof("FIELDS") - 1, _NEWZVAL);

        if (NULL != zparam_FIELDS)
        {
            /* FIELDS parameter is a table */
            array_init(zparam_FIELDS);

            if (Z_TYPE_P(zfields) == IS_STRING)
            {
                zval *zfieldname = zfields;

                if (Z_STRLEN_P(zfieldname) == 0) {
                    zend_throw_exception(php_sap_invalid_args_exception, "SapRfcReadTable::select() : Empty string as query field is not allowed", -1);
                    return;
                }

                /**
                * If user provided a string as query fields then we will convert it to array as follows:
                * 1) if the string equals to sql star (*), we will convert to an empty array,
                *    as an empty FIELDS parameter will select all table's fields
                * 2) otherwise we will convert to an array with only one element (the provided field name)
                */
                
                zfields = zfieldshelper = ecalloc(1, sizeof(zval));

                array_init(zfields);

                if (0 != memcmp(Z_STRVAL_P(zfieldname), "*", sizeof("*") - 1)) {
                    zend_hash_next_index_insert(Z_ARRVAL_P(zfields), zfieldname);
                    Z_ADDREF_P(zfieldname);
                }
            }
            else if (Z_TYPE_P(zfields) != IS_ARRAY)
            {
                zend_throw_exception_ex(
                    php_sap_invalid_args_exception,
                    -1,
                    "Argument 1 of SapRfcReadTable::select() must be a string or array. %s given",
                    zend_get_type_by_const(Z_TYPE_P(zfields))
                );

                return;
            }

            /* Append requested fields to the FIELDS table parameter */
            do
            {
                ulong h;
                zend_string *alias;
                zval *zfieldname;

                ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(zfields), h, alias, zfieldname)
                {
                    zval *zfieldrow;
                    zval zalias;

                    /* Field names must be strings */
                    if (Z_TYPE_P(zfieldname) != IS_STRING) {
                        zend_throw_exception_ex(
                            spl_ce_LogicException, 
                            -1, 
                            "Field names must be strings (%s detected)", 
                            zend_get_type_by_const(Z_TYPE_P(zfieldname))
                        );
                        return;
                    }

                    /**
                    * If a string key is set for the current element then this will be used as field name
                    * and the zfieldname will contain the alias name that will be used later on export
                    */
                    if (alias)
                    {
                        ZVAL_STRINGL(&zalias, alias->val, alias->len);

                        /* set reference count to zero for now, it will be increased later */
                        Z_DELREF_P(&zalias);

                        /* replace field's name */
                        zfieldname = &zalias;
                    }

                    /* Append a new row to FIELDS table */
                    if (NULL != (zfieldrow = zend_hash_next_index_insert_new(Z_ARRVAL_P(zparam_FIELDS), _NEWZVAL)))
                    {
                        /* Each FIELDS table row, is an array itself */
                        array_init_size(zfieldrow, 1);

                        if (zend_hash_str_add(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1, zfieldname)) {
                            Z_ADDREF_P(zfieldname);
                        }
                    }
                }
                ZEND_HASH_FOREACH_END();
            } while (0);
        }
    }

    /* Create the 'where' clause (OPTIONS parameter) */
    if (NULL != zwhere && zend_hash_num_elements(Z_ARRVAL_P(zwhere)) > 0)
    {
        ulong h;
        zend_string *wfield;
        zval *zwval;
        zval *zparam_OPTIONS = zend_hash_str_add_new(Z_ARRVAL_P(pzimports), "OPTIONS", sizeof("OPTIONS") - 1, _NEWZVAL);

        if (NULL != zparam_OPTIONS)
        {
            /* Parameter OPTIONS is a table */
            array_init(zparam_OPTIONS);

            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(zwhere), h, wfield, zwval)
            {
                zval *zoptionsrow = NULL;
                zval *ztextfield = NULL;
                zval zwstrval, *pzwstrval = &zwstrval;

                /* append new row to table 'OPTIONS' */
                zoptionsrow = zend_hash_next_index_insert_new(Z_ARRVAL_P(zparam_OPTIONS), _NEWZVAL);

                if (NULL == zoptionsrow) {
                    break;
                }

                array_init_size(zoptionsrow, 1);

                /* append field 'TEXT' to the new row */
                ztextfield = zend_hash_str_add_new(Z_ARRVAL_P(zoptionsrow), "TEXT", sizeof("TEXT") - 1, _NEWZVAL);

                if (NULL == ztextfield) {
                    break;
                }

                /* element's value must be a string */
                if (Z_TYPE_P(zwval) != IS_STRING)
                {
                    ZVAL_ZVAL(pzwstrval, zwval, 1, 0);
                    convert_to_string(pzwstrval);
                    zwval = pzwstrval;
                }

                if (NULL != wfield) /* string key: field => val */
                {
                    char optionText[300];
                    int optionTextLen;
                    int hasRows = zend_hash_num_elements(Z_ARRVAL_P(zparam_OPTIONS)) > 1;

                    /* "field EQ 'val'" */
                    optionTextLen = snprintf(optionText, sizeof(optionText), "%s%s EQ '%s'", hasRows ? "AND " : "", wfield->val, Z_STRVAL_P(zwval));

                    ZVAL_STRINGL(ztextfield, optionText, optionTextLen);
                }
                else { /* custom WHERE clause, append 'as is' */
                    ZVAL_ZVAL(ztextfield, zwval, 1, 0);
                }

                if (UNEXPECTED(zwval == pzwstrval)) {
                    zval_ptr_dtor(pzwstrval);
                }
            }
            ZEND_HASH_FOREACH_END();
        }
    }

    /* Call RFC_READ_TABLE */
    {
        int result;
        zval zresult;

        result = sap_call_object_method(getThis(), sap_ce_SapFunction, "__invoke", NULL, &__invoke_args, &zresult TSRMLS_CC);

        zval_dtor(&__invoke_args);

        if (SUCCESS != result) {
            zend_error_noreturn(E_ERROR, "__invoke method invokation failed");
        }

        if (Z_TYPE(zresult) != IS_ARRAY) {
            zend_throw_exception_ex(
                spl_ce_LogicException, 
                -1, 
                "Expected an array return value from __invoke (got %s)", 
                zend_get_type_by_const(Z_TYPE(zresult))
            );
            return;
        }

        /* Convert the DATA table to a PHP readable array */
        {
            zval *zresdata;
            zval *zresfields;

            if (NULL != (zresdata = zend_hash_str_find(Z_ARRVAL(zresult), "DATA", sizeof("DATA") - 1))
                && Z_TYPE_P(zresdata) == IS_ARRAY
                && NULL != (zresfields = zend_hash_str_find(Z_ARRVAL(zresult), "FIELDS", sizeof("FIELDS") - 1))
                && Z_TYPE_P(zresfields) == IS_ARRAY
            ) {
                zval *zdatarow;

                array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_P(zresdata)));

                ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(zresdata), zdatarow)
                {
                    zval *zresow, *wa, *zfieldrow;

                    wa = zend_hash_str_find(Z_ARRVAL_P(zdatarow), "WA", sizeof("WA") - 1);

                    /* Fix bug: wa might be NULL also */
                    if (NULL == wa || (Z_TYPE_P(wa) != IS_STRING && Z_TYPE_P(wa) != IS_NULL)) {
                        continue;
                    }

                    if (NULL == (zresow = zend_hash_next_index_insert_new(Z_ARRVAL_P(return_value), _NEWZVAL))) {
                        continue;
                    }

                    array_init_size(zresow, zend_hash_num_elements(Z_ARRVAL_P(zresfields)));

                    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(zresfields), zfieldrow)
                    {
                        zval *zfieldname, *zfieldoffset, *zfieldlength, *zfieldtype;
                        zval zlongoffset, zlonglength;

                        if (Z_TYPE_P(zfieldrow) != IS_ARRAY) {
                            continue; /* This should not happen */
                        }

                        if (NULL != (zfieldname = zend_hash_str_find(Z_ARRVAL_P(zfieldrow), "FIELDNAME", sizeof("FIELDNAME") - 1))
                            && Z_TYPE_P(zfieldname) == IS_STRING
                            && (zfieldoffset = zend_hash_str_find(Z_ARRVAL_P(zfieldrow), "OFFSET", sizeof("OFFSET") - 1))
                            && (zfieldlength = zend_hash_str_find(Z_ARRVAL_P(zfieldrow), "LENGTH", sizeof("LENGTH") - 1))
                            && (zfieldtype = zend_hash_str_find(Z_ARRVAL_P(zfieldrow), "TYPE", sizeof("TYPE") - 1))
                            && Z_TYPE_P(zfieldtype) == IS_STRING)
                        {
                            zval *zfieldvalue;
                            zval *zfieldalias;

                            ZVAL_ZVAL(&zlongoffset, zfieldoffset, 1, 0);
                            convert_to_long(&zlongoffset);

                            ZVAL_ZVAL(&zlonglength, zfieldlength, 1, 0);
                            convert_to_long(&zlonglength);

                            if (NULL != (zfieldalias = zend_hash_find(Z_ARRVAL_P(zfields), Z_STR_P(zfieldname)))) {
                                /* is field alias */
                                zfieldname = zfieldalias;
                            }

                            if (NULL != (zfieldvalue = zend_hash_add_new(Z_ARRVAL_P(zresow), Z_STR_P(zfieldname), _NEWZVAL)))
                            {
                                char *substr;
                                int substrlen;

                                if (Z_TYPE_P(wa) == IS_STRING &&
                                    SUCCESS == utf8_substr(Z_STRVAL_P(wa), Z_STRLEN_P(wa), Z_LVAL(zlongoffset), Z_LVAL(zlonglength), &substr, &substrlen)
                                ) {
                                    if (rtrim)
                                    {
                                        char *substrtrimmed;
                                        int substrtrimmedlen;

                                        utf8_trim(substr, substrlen, &substrtrimmed, &substrtrimmedlen, TRIM_RIGHT);

                                        efree(substr);

                                        substr = substrtrimmed;
                                        substrlen = substrtrimmedlen;
                                    }

                                    ZVAL_STRINGL(zfieldvalue, substr, substrlen);
                                    efree(substr);
                                }
                                else {
                                    ZVAL_NULL(zfieldvalue);
                                }
                            }
                        }
                    }
                    ZEND_HASH_FOREACH_END();
                }
                ZEND_HASH_FOREACH_END();
            }
        }

        if (result == SUCCESS) {
            zval_ptr_dtor(&zresult);
        }
    }
}

PHP_MINIT_FUNCTION(sap)
{
    zend_class_entry ce;

#if SAPwithPTHREADS
    /* initialize conversion mutexes */
    rfc_utf8_to_sapuc_mutex = PTHREAD_MUTEX_INITIALIZER;
    rfc_sapuc_to_utf8_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

    /* Register resources */
    le_php_sap_connection = zend_register_list_destructors_ex(php_sap_connection_rsrc_dtor, NULL, PHP_SAP_CONNECTION_RES_NAME, module_number);

    ZEND_INIT_MODULE_GLOBALS(sap, ZEND_MODULE_GLOBALS_CTOR_N(sap), NULL);

    REGISTER_INI_ENTRIES();

    /* Define Sap classes */

    //SapException
    INIT_CLASS_ENTRY(ce, "SapException", sap_exception_fe);
    sap_ce_SapException = zend_register_internal_class_ex(&ce, zend_ce_exception);

    zend_declare_property_string(sap_ce_SapException, "MSGTY", sizeof("MSGTY") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "MSGID", sizeof("MSGID") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "MSGNO", sizeof("MSGNO") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "MSGV1", sizeof("MSGV1") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "MSGV2", sizeof("MSGV2") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "MSGV3", sizeof("MSGV3") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "MSGV4", sizeof("MSGV4") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "KEY", sizeof("KEY") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_string(sap_ce_SapException, "nwsdkfunction", sizeof("nwsdkfunction") - 1, "", ZEND_ACC_PROTECTED TSRMLS_CC);

    //Sap
    INIT_CLASS_ENTRY(ce, "Sap", sap_fe_Sap);
    sap_ce_Sap = zend_register_internal_class(&ce TSRMLS_CC);

    sap_ce_Sap->create_object = sap_object_create_object;

    memcpy(&sap_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    sap_object_handlers.clone_obj = sap_object_clone_object;
    sap_object_handlers.free_obj = sap_object_free_object_storage;
    sap_object_handlers.offset = XtOffsetOf(sap_object, std);

    //SapFunction
    INIT_CLASS_ENTRY(ce, "SapFunction", sap_fe_SapFunction);
    sap_ce_SapFunction = zend_register_internal_class(&ce TSRMLS_CC);

    sap_ce_SapFunction->create_object = sap_function_create_object;

    memcpy(&sap_function_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    sap_function_handlers.clone_obj = sap_function_clone_object;
    sap_function_handlers.free_obj = sap_function_free_object_storage;
    sap_function_handlers.offset = XtOffsetOf(sap_function, std);

    //SapRfcReadTable
    INIT_CLASS_ENTRY(ce, "SapRfcReadTable", sap_fe_SapRfcReadTable);
    sap_ce_SapRfcReadTable = zend_register_internal_class_ex(&ce, sap_ce_SapFunction);

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
    int nwsdkversionlen;
    SAPRFC_ERROR_INFO err;

    php_info_print_table_start();
    php_info_print_table_header(2, "SAP Remote Functions call support", "enabled");
    php_info_print_table_row(2, "Version", PHP_SAP_VERSION);

    sdkversionU = RfcGetVersion(&major, &minor, &patchlevel);

    if (SUCCESS == sapuc_to_utf8((SAP_UC*)sdkversionU, &nwsdkversion, &nwsdkversionlen, &err)) {
        php_info_print_table_row(2, "Nw Rfc Sdk Version", nwsdkversion);
        efree(nwsdkversion);
    }

    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
