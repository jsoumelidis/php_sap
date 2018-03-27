dnl $Id$
dnl config.m4 for sap extension

PHP_ARG_ENABLE(sap, for SAP remote function call support,
[  --enable-sap            Enable SAP remote function call support.], no, yes)

PHP_ARG_WITH(sap-nwrfcsdk, for nwrfcsdk optional path,
[  --with-sap-nwrfcsdk     SAP NetWeaver RFC SDK directory (available from SAP marketplace)], no, no)

PHP_ARG_ENABLE(sap-unicode, SAP unicode support,
[  --disable-sap-unicode   Disable unicode. Use if connecting to non-unicode R/3 system], yes, no)

PHP_ARG_ENABLE(sap-pthreads, Disable PThreads support,
[  --disable-sap-pthreads  Disable PThreads support], yes, no)

if test "$PHP_SAP" != "no"; then
  SAPNWRFCSDK="";
  SEARCH_PATH="/usr/nwrfcsdk /usr/local/nwrfcsdk /opt/nwrfcsdk"
  SEARCH_FOR="libsapnwrfc.so libsapucum.so"
  
  if test "$PHP_SAP_NWRFCSDK" != "no"; then
  
    if test "$PHP_SAP_NWRFCSDK" == "yes"; then
      AC_MSG_ERROR([You must specify a path when using --with-sap-nwrfcsdk])
    fi
    
    if test -z "$PHP_SAP_NWRFCSDK/include"; then
      AC_MSG_ERROR(Could not locate SAP NW RFC SDK include dir)
    fi
    
    if test -z "$PHP_SAP_NWRFCSDK/lib"; then
      AC_MSG_ERROR(Could not locate SAP NW RFC SDK lib dir)
    fi
    
    SEARCH_PATH="$SEARCH_PATH $PHP_SAP_NWRFCSDK"
  fi
  
  AC_MSG_CHECKING([for sapnwrfc.h])
  
  for i in $SEARCH_PATH; do
    if test -f $i/include/sapnwrfc.h; then
      SAPNWRFCSDK=$i
      AC_MSG_RESULT(in $i)
    fi
  done
  
  if test -z "$SAPNWRFCSDK"; then
    AC_MSG_ERROR([SAP NWRFCSDK not found])
  fi
  
  PHP_ADD_INCLUDE($SAPNWRFCSDK/include)
  
  if test "$PHP_SAP_UNICODE" != "no"; then
    AC_DEFINE(SAPwithUNICODE, 1, [SAP unicode conversion])
  fi
  
  AC_MSG_CHECKING([for libsapnwrfc.so])

  if test ! -f $SAPNWRFCSDK/lib/libsapnwrfc.so; then
     AC_MSG_RESULT(not found in $SAPNWRFCSDK/lib)
     AC_MSG_ERROR(Please fix nwrfcsdk installation)
  fi
  
  AC_MSG_RESULT(in $SAPNWRFCSDK/lib)
  PHP_ADD_LIBRARY_WITH_PATH(sapnwrfc, $SAPNWRFCSDK/lib, SAP_SHARED_LIBADD)
  
  AC_MSG_CHECKING([for libsapucum.so])

  if test ! -f $SAPNWRFCSDK/lib/libsapucum.so; then
     AC_MSG_RESULT(not found in $SAPNWRFCSDK/lib)
     AC_MSG_ERROR(Please fix nwrfcsdk installation)
  fi
  
  AC_MSG_RESULT(in $SAPNWRFCSDK/lib)
  PHP_ADD_LIBRARY_WITH_PATH(sapucum, $SAPNWRFCSDK/lib, SAP_SHARED_LIBADD)

  PHP_SUBST(SAP_SHARED_LIBADD)
  
  AC_DEFINE(HAVE_SAP, 1, [SAP remote function call support])
  
  if test "$PHP_SAP_PTHREADS" == "yes"; then
    if test "$PHP_ZTS" == "yes"; then
      AC_MSG_CHECKING([for pthread.h])
      AC_CHECK_HEADER([pthread.h], [
        AC_DEFINE(SAPwithPTHREADS, 1, [Compile SAP extension with pthreads support])
        AC_MSG_RESULT(yes. Pthreads support for SAP enabled)
      ], [
        AC_MSG_RESULT(no. Pthreads support for SAP not enabled)
      ])
    fi
  fi

  PHP_NEW_EXTENSION(sap, php_sap.c, $ext_shared)

  PHP_ADD_BUILD_DIR($SAPNWRFCSDK/lib)
fi
