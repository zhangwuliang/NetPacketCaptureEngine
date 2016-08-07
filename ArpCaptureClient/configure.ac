#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ArpCaptureClient], [1.0], [https://github.com/zhangwuliang/LibevTcpProj])
AM_INIT_AUTOMAKE(ArpCaptureClient, 1.0)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcpp_common':
AC_CHECK_LIB([cpp_common], [main])
# FIXME: Replace `main' with a function in `-lev':
AC_CHECK_LIB([ev], [main])
# FIXME: Replace `main' with a function in `-ljsoncpp':
AC_CHECK_LIB([jsoncpp], [main])
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([bzero ftruncate memmove memset socket strerror])

AC_CONFIG_FILES([Makefile
                 etc/Makefile
                 src/Makefile])
AC_OUTPUT
