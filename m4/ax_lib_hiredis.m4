#
# SYNOPSIS
#
#   AX_LIBS_HIREDIS
#
# DESCRIPTION
#
#   This macro provides tests of availability of Hiredis libs.
#
#   AX_LIBS_HIREDIS macro takes no arguments.
#
#   The --with-hiredis-lib option takes one of three possible values:
#
#   no - do not check for Hiredis libs
#
#   yes - do check for Hiredis libs in the default location
#
#   path - complete path to Hiredis libs
#
#   This macro calls:
#
#     AC_SUBST(HIREDIS_LDFLAGS)
#
#   And sets:
#
#     HAVE_HIREDIS_LIBS
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

AC_DEFUN([AX_LIBS_HIREDIS],
[

    HIREDIS_LIBS="/usr/lib64"

    # Set the commands full path name
    AC_PATH_PROG([CMD_LD], [ld], [])
    AC_PATH_PROG([CMD_GREP], [grep], [])
    AC_PATH_PROG([CMD_SED], [sed], [])
    AC_PATH_PROG([CMD_ECHO], [echo], [])

    # Get the library search dirs from the ld command
    LIB_DIRS=`$CMD_LD --verbose 2>&1 | $CMD_GREP SEARCH_DIR 2>&1`

    # For the library search dirs
    for HIREDIS_LIBS_TMP in $LIB_DIRS; do

        # Strip out the directory
        HIREDIS_LIBS_TMP=`$CMD_ECHO $HIREDIS_LIBS_TMP | $CMD_SED -e 's/SEARCH_DIR//g' -e 's/(//g' -e 's/)//g' -e 's/=//g' -e 's/[";]*//g'` 

        # If the library is present
        if test -f "$HIREDIS_LIBS_TMP/libhiredis.so"; then
            HIREDIS_LIBS=$HIREDIS_LIBS_TMP
            break;
        fi

    done

    AC_ARG_WITH([hiredis-libs],
        AS_HELP_STRING([--with-hiredis-libs=@<:@PATH@:>@],
            [Hiredis Libs @<:@default=/usr/lib64@:>@]
        ),
        [
        if test "$withval" = "no"; then
            want_hiredis_libs="no"
        elif test "$withval" = "yes"; then
            want_hiredis_libs="yes"
        else
            want_hiredis_libs="yes"
            HIREDIS_LIBS="$withval"
        fi
        ],
        [want_hiredis_libs="yes"]
    )

    HIREDIS_LDFLAGS=""

    dnl
    dnl Check Hiredis libs
    dnl

    if test "$want_hiredis_libs" = "yes"; then        
        AC_MSG_CHECKING([for Hiredis libs])

        if test ! -f "$HIREDIS_LIBS/libhiredis.so"; then
            found_hiredis_libs="no"
            AC_MSG_RESULT([no])

            AC_MSG_ERROR([$HIREDIS_LIBS/libhiredis.so does not exist])
            HIREDIS_LIBS="no"
        fi

        if test "$HIREDIS_LIBS" != "no"; then
            HIREDIS_LDFLAGS="-L$HIREDIS_LIBS -lhiredis"
            AC_DEFINE([HAVE_HIREDIS], [1],
                [Define to 1 if Hiredis libs are available])

            found_hiredis_libs="yes"
            AC_MSG_RESULT([yes])
        fi

    fi

    AC_SUBST([HIREDIS_LDFLAGS])
])
