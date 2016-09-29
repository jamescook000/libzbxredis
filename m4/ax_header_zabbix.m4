#
# SYNOPSIS
#
#   AX_HEADERS_ZABBIX
#
# DESCRIPTION
#
#   This macro provides tests of availability of Zabbix headers.
#
#   AX_HEADERS_ZABBIX macro takes no arguments.
#
#   The --with-zabbix-src option takes one of three possible values:
#
#   no - do not check for Zabbix headers
#
#   yes - do check for Zabbix headers in the default location
#
#   path - complete path to Zabbix headers
#
#   This macro calls:
#
#     AC_SUBST(ZABBIX_CPPFLAGS)
#
#   And sets:
#
#     HAVE_ZABBIX_HEADERS
#
# LICENSE
#
#   Copyright (c) 2016 James Cook <james.cook000@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_HEADERS_ZABBIX],
[
    ZABBIX_HEADERS="/usr/src/zabbix/include"

    AC_ARG_WITH([zabbix-headers],
        AS_HELP_STRING([--with-zabbix-headers=@<:@PATH@:>@],
            [Zabbix Headers @<:@default=/usr/src/zabbix/include@:>@]
        ),
        [
        if test "$withval" = "no"; then
            want_zabbix_headers="no"
        elif test "$withval" = "yes"; then
            want_zabbix_headers="yes"
        else
            want_zabbix_headers="yes"
            ZABBIX_HEADERS="$withval"
        fi
        ],
        [want_zabbix_headers="yes"]
    )

    ZABBIX_CPPFLAGS=""

    dnl
    dnl Check Zabbix headers
    dnl

    if test "$want_zabbix_headers" = "yes"; then        
        AC_MSG_CHECKING([for Zabbix headers])

        if test ! -f "$ZABBIX_HEADERS/module.h"; then
            found_zabbix_headers="no"
            AC_MSG_RESULT([no])

            AC_MSG_ERROR([$ZABBIX_HEADERS/module.h does not exist])
            ZABBIX_HEADERS="no"
        fi

        if test "$ZABBIX_HEADERS" != "no"; then
            ZABBIX_CPPFLAGS="-I$ZABBIX_HEADERS"
            AC_DEFINE([HAVE_ZABBIX_HEADERS], [1],
                [Define to 1 if Zabbix headers are available])

            found_zabbix_headers="yes"
            AC_MSG_RESULT([yes])
        fi

    fi

    AC_SUBST([ZABBIX_CPPFLAGS])
])
