AC_INIT([libzbxredis], [0.1.0.0], [james.cook000@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability foreign])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR
AC_PROG_LIBTOOL # seem to need this on CentOS 5 (libtool v1.5)
LT_INIT
AC_PROG_CC
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# check for stdlib header files
AC_HEADER_STDC

# Checking for Zabbix headers
AX_HEADERS_ZABBIX
if test ! "x$found_zabbix_headers" = "xyes"; then
	AC_MSG_ERROR([Zabbix headers not found])
fi

# Checking for Hiredis libs
AX_LIBS_HIREDIS
if test ! "x$found_hiredis_libs" = "xyes"; then
	AC_MSG_ERROR([Hiredis libs not found])
fi

# Checking for Hiredis headers
AX_HEADERS_HIREDIS
if test ! "x$found_hiredis_headers" = "xyes"; then
	AC_MSG_ERROR([Hiredis headers not found])
fi

# output
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
