#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(libflm, 0.3, bug@libflm.me)
AC_CONFIG_SRCDIR([src/obj.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
AM_PATH_CHECK()
AM_PROG_CC_C_O

# Doxygen
DX_INIT_DOXYGEN($PACKAGE_NAME, doxygen.cfg)

# Checks for programs.
#AC_PROG_CC
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([  limits.h		\
		    stdlib.h		\
		    string.h		\
		    sys/time.h		\
		    unistd.h		\
		    arpa/inet.h		\
		    fcntl.h		\
		    netdb.h		\
		    netinet/in.h	\
		    string.h		\
		    sys/socket.h	\
		    sys/time.h		\
		    sys/epoll.h		\
		    sys/select.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday localtime_r memset])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 include/flm/Makefile
                 include/flm/core/Makefile
                 include/flm/core/public/Makefile
                 include/flm/core/private/Makefile
                 tests/Makefile])

AC_CHECK_FUNCS([memset socket select epoll_ctl epoll_wait])

AC_OUTPUT
