#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([dzip], [2.9])
AC_CONFIG_SRCDIR([source/main.c])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stddef.h stdlib.h string.h unistd.h utime.h])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ftruncate memset strcasecmp strchr strdup strerror strpbrk utime])

AC_CONFIG_FILES([Makefile
                 source/Makefile])
AC_OUTPUT
