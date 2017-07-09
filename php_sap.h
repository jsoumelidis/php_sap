#ifndef PHP_SAP_H
#	define PHP_SAP_H

#	include "php.h"
#	include "sapnwrfc.h"

#	ifdef ZTS
#		include "TSRM.h"
#		define PHP_SAP_GLOBALS(v) TSRMG(sap_globals_id, zend_sap_globals*, v)
#	else
#		define PHP_SAP_GLOBALS(v) (sap_globals.v)
#	endif

#	ifndef PHP_SAP_DEBUG
#		define PHP_SAP_DEBUG 0
#	endif

#	define PHP_SAP_VERSION "0.01"

#	define PHP_SAP_CONNECTION_RES_NAME "SAP R/3 Connection"

#	ifdef PHP_WIN32
#		define SAP_PHP_API __declspec(dllexport)
#	else
#		define SAP_PHP_API
#	endif

extern zend_module_entry sap_module_entry;

typedef struct _php_sap_connection {
	RFC_CONNECTION_HANDLE	handle;
	HashTable				*lparams;
	unsigned int			refCount;
} php_sap_connection;

typedef struct _php_sap_function {
	RFC_FUNCTION_DESC_HANDLE	fdh;
	HashTable					*params;
} php_sap_function;

SAP_PHP_API zend_class_entry * php_sap_get_sap_ce(void);
SAP_PHP_API zend_object_handlers * php_sap_get_sap_handlers(void);
SAP_PHP_API php_sap_connection * php_sap_get_connection(zval *zsap TSRMLS_DC);

SAP_PHP_API zend_class_entry * php_sap_get_function_ce(void);
SAP_PHP_API zend_object_handlers * php_sap_get_function_handlers(void);
SAP_PHP_API php_sap_function * php_sap_get_function_descr(zval *zfunction TSRMLS_DC);

SAP_PHP_API zend_class_entry * php_sap_get_exception_ce(void);

PHP_MINIT_FUNCTION(sap);
PHP_MSHUTDOWN_FUNCTION(sap);
PHP_MINFO_FUNCTION(sap);

#endif
