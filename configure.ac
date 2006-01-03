# Process this file with autoconf to produce a configure script.
AC_INIT(arrayprobe, 2.0, [spark@knoware.nl])
AM_INIT_AUTOMAKE(arrayprobe, 2.0)
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

dnl Check for kernel sources
AC_MSG_CHECKING(for kernel sources)
found_kernel_dir=""
for kernel_dir in /usr/src/linux /usr/src/linux-2.4
do
  if test -d $kernel_dir/drivers/block ; then
    CFLAGS="$CFLAGS -I$kernel_dir/include -I$kernel_dir/drivers/block"
    CPPFLAGS="$CPPFLAGS -I$kernel_dir/include -I$kernel_dir/drivers/block"
    found_kernel_dir="$kernel_dir"
  fi
done

if test "$found_kernel_dir" != "" ; then
  AC_MSG_RESULT(found $found_kernel_dir )
else
  AC_MSG_ERROR(Kernel sources not found)
fi

dnl Check Headers
AC_CHECK_HEADERS(ida_ioctl.h ida_cmd.h cpqarray.h,,
  AC_MSG_ERROR(You need to have the complete kernel sources.),[[#include <linux/compiler.h>
  #include <unistd.h>]])

dnl Check version of SmartArray driver
AC_MSG_CHECKING(SmartArray driver version)
AC_EGREP_HEADER(blk_cnt, ida_ioctl.h,AC_MSG_RESULT(ok),
  AC_MSG_ERROR(You need to have the SmartArray driver version 1.0.1 or higher installed.))
                                                                       
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
