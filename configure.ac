# Process this file with autoconf to produce a configure script.
AC_INIT(arrayprobe, 1.0, [spark@knoware.nl])
AM_INIT_AUTOMAKE(arrayprobe, 1.0)
AC_CONFIG_SRCDIR([probe.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h unistd.h])

dnl Check for compile.h (2.6.x kernels only?
AC_CHECK_HEADERS(linux/compiler.h)
                                                                       
dnl Check for CCISS header file
AC_CHECK_HEADERS(linux/cciss_ioctl.h,,
  AC_MSG_ERROR(You need to have the CCISS driver in the kernel.),[[#include <linux/compiler.h>]])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostbyname gethostname memset strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
