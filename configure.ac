#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([dzip], [2.9], [https://github.com/darealshinji/dzip])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([source/main.c])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
LT_INIT

# Check if compiler supports "-m32".
AX_CHECK_COMPILE_FLAG([-m32], [], [exit 1])

# Check if linker supports "-m32".
AX_CHECK_LINK_FLAG([-m32], [], [exit 1])

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stddef.h stdlib.h string.h unistd.h utime.h])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ftruncate memset strcasecmp strchr strdup strerror strpbrk utime])

AC_CONFIG_FILES([Makefile
                 source/Makefile
                 dzip.1
                 dzip.pc])
AC_OUTPUT
