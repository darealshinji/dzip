AC_INIT([dzip], [2.9], [https://github.com/darealshinji/dzip])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([source/main.c])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])

AC_PROG_CC
LT_INIT([win32-dll])

AC_CONFIG_FILES([Makefile
                 source/Makefile
                 dzip.1
                 dzip.pc])
AC_OUTPUT
