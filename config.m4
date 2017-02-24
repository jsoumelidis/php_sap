dnl $Id$
dnl config.m4 for sap extension

PHP_ARG_WITH(sap, for SAP Remote Function Call support,
[  --with-sap[=SAPNWRFC_DIR]     Enable SAP remote function calls support.
                                 SAPNWRFC_DIR is the SAP NWRFCSDK install directory (available from SAP Marketplace)
])


if test "$PHP_SAP" != "no"; then
  for i in $PHP_SAP; do
    if test -f $i/include/sapnwrfc.h; then
      SAPNWRFC_DIR=$i
    fi
  done
  
  if test -z "$SAPNWRFC_DIR"; then
    AC_MSG_ERROR(Could not locate sapnwrfc.h)
  fi      

  PHP_ADD_INCLUDE($SAPNWRFC_DIR/include)
  if test ! -f $SAPNWRFC_DIR/lib/libsapnwrfc.so; then
    AC_MSG_ERROR(Shared RFC Unicode library (libsapnwrfc.so) not found)
  fi
  
  PHP_ADD_LIBRARY_WITH_PATH(sapnwrfc, $SAPNWRFC_DIR/lib, SAPNWRFC_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(sapucum,  $SAPNWRFC_DIR/lib, SAPNWRFC_SHARED_LIBADD)

  PHP_BUILD_SHARED()
  
  AC_CANONICAL_HOST
  case "$host" in
    *-hp-*)
        PHP_ADD_LIBRARY_WITH_PATH(cl, $SAPNWRFC_DIR/lib, SAPNWRFC_SHARED_LIBADD)
        ;;
  esac	

  PHP_SUBST(SAPNWRFC_SHARED_LIBADD)
  
  AC_DEFINE(HAVE_SAP, 1, [Whether you have sap extension enabled])
  
  PHP_NEW_EXTENSION(sap, src/php_sap.c, $ext_shared)

  PHP_ADD_BUILD_DIR($SAPNWRFC_DIR/lib)
fi