#ifndef PHP_SAP_H
#define PHP_SAP_H

#include "php.h"
#include "sapnwrfc.h"

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_SAP_DEBUG 0
#define PHP_SAP_VERSION "0.01"

#define PHP_SAP_CONNECTION_RES_NAME "SAP R/3 Connection"

extern zend_module_entry sap_module_entry;

PHP_MINIT_FUNCTION(sap);
PHP_MSHUTDOWN_FUNCTION(sap);

PHP_MINFO_FUNCTION(sap);

#endif
