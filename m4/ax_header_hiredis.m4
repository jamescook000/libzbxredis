#
# SYNOPSIS
#
#   AX_HEADERS_HIREDIS
#
# DESCRIPTION
#
#   This macro provides tests of availability of Hiredis headers.
#
#   AX_HEADERS_HIREDIS macro takes no arguments.
#
#   The --with-hiredis-headers option takes one of three possible values:
#
#   no - do not check for Hiredis headers
#
#   yes - do check for Hiredis headers in the default location
#
#   path - complete path to Hiredis headers
#
#   This macro calls:
#
#     AC_SUBST(HIREDIS_CPPFLAGS)
#
#   And sets:
#
#     HAVE_HIREDIS_HEADERS
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

AC_DEFUN([AX_HEADERS_HIREDIS],
[
    HIREDIS_HEADERS="/usr/include/hiredis"

    AC_ARG_WITH([hiredis-headers],
        AS_HELP_STRING([--with-hiredis-headers=@<:@PATH@:>@],
            [Hiredis Headers @<:@default=/usr/include/hiredis@:>@]
        ),
        [
        if test "$withval" = "no"; then
            want_hiredis_headers="no"
        elif test "$withval" = "yes"; then
            want_hiredis_headers="yes"
        else
            want_hiredis_headers="yes"
            HIREDIS_HEADERS="$withval"
        fi
        ],
        [want_hiredis_headers="yes"]
    )

    HIREDIS_CPPFLAGS=""

    dnl
    dnl Check Hiredis headers
    dnl

    if test "$want_hiredis_headers" = "yes"; then        
        AC_MSG_CHECKING([for Hiredis headers])

        if test ! -f "$HIREDIS_HEADERS/hiredis.h"; then
            found_hiredis_headers="no"
            AC_MSG_RESULT([no])

            AC_MSG_ERROR([$HIREDIS_HEADERS/hiredis.h does not exist])
            HIREDIS_HEADERS="no"
        fi

        if test "$HIREDIS_HEADERS" != "no"; then
            HIREDIS_CPPFLAGS="-I$HIREDIS_HEADERS"
            AC_DEFINE([HAVE_HIREDIS_HEADERS], [1],
                [Define to 1 if Hiredis headers are available])

            found_hiredis_headers="yes"
            AC_MSG_RESULT([yes])
        fi

    fi

    AC_SUBST([HIREDIS_CPPFLAGS])
])
