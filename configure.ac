#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([s98ml], [0.0], [autch@autch.net])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([s98d_header.c])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

# Checks for libraries.
#AC_CHECK_LIB([fl], [yywrap])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h memory.h stddef.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strchr strdup])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

