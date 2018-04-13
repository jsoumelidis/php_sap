#ifndef PHP_SAP_H
#define PHP_SAP_H

#include "php.h"
#include "sapnwrfc.h"

#ifdef ZTS
#   include "TSRM.h"
#   define PHP_SAP_GLOBALS(v) TSRMG(sap_globals_id, zend_sap_globals*, v)
#else
#   define PHP_SAP_GLOBALS(v) (sap_globals.v)
#endif

#ifndef PHP_SAP_DEBUG
#   define PHP_SAP_DEBUG 0
#endif

#define PHP_SAP_VERSION "7.0.x"
#define PHP_SAP_CONNECTION_RES_NAME "SAP Connection"

#ifdef PHP_WIN32
#   define PHP_SAP_API __declspec(dllexport)
#else
#   define PHP_SAP_API
#endif

extern zend_module_entry sap_module_entry;

typedef struct _php_sap_connection {
    RFC_CONNECTION_HANDLE   handle;
    unsigned int            refCount;
} php_sap_connection;

typedef struct _php_sap_function    {
    RFC_FUNCTION_DESC_HANDLE    fdh;
    HashTable*                  params;
} php_sap_function;

PHP_SAP_API zend_class_entry*       php_sap_get_sap_ce(void);
PHP_SAP_API zend_object_handlers*   php_sap_get_sap_handlers(void);

PHP_SAP_API zend_class_entry*       php_sap_get_function_ce(void);
PHP_SAP_API zend_object_handlers*   php_sap_get_function_handlers(void);

PHP_SAP_API zend_class_entry*       php_sap_get_exception_ce(void);

PHP_MINIT_FUNCTION(sap);
PHP_MSHUTDOWN_FUNCTION(sap);
PHP_MINFO_FUNCTION(sap);

#endif /* PHP_SAP_H */
