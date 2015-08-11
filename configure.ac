# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT(simvacation, m4_esyscmd([bash version.sh]), simta@umich.edu)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([simunvacation.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Set up variables
AC_ARG_WITH(vacationdb, AC_HELP_STRING([--with-vacationdb=DB], [default: /var/lib/simvacation]), [], with_vacationdb="/var/lib/simvacation")
AC_DEFINE_UNQUOTED(VDBDIR, ["$with_vacationdb"], [vacation db location])

AC_ARG_WITH(syslog, AC_HELP_STRING([--with-syslog=FACILITY], [default: LOG_MAIL]), [], with_syslog="LOG_MAIL")
AC_DEFINE_UNQUOTED(LOG_VACATION, [$with_syslog], [syslog facility])

AC_ARG_WITH(backend, AC_HELP_STRING([--with-backend=VDB], [default: lmdb]), [], with_backend="lmdb")

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99

# Checks for libraries.
AS_IF([test x$with_backend = xbdb],
[AX_PATH_BDB([4], [
    AC_DEFINE([HAVE_BDB], [1], [Define if BDB exists.])
    AC_SUBST(BDB_LDFLAGS)
    AC_SUBST(BDB_CPPFLAGS)
    AC_SUBST(BDB_LIBS)
])])

AS_IF([test x$with_backend = xlmdb],
[AX_CHECK_LIBRARY(LMDB, [lmdb.h], [lmdb], [], [AC_MSG_ERROR([Unable to find LMDB])])])

AM_CONDITIONAL(BACKEND_BDB, [test x$with_backend = xbdb])
AM_CONDITIONAL(BACKEND_LMDB, [test x$with_backend = xlmdb])
AM_CONDITIONAL(BACKEND_NULL, [test x$with_backend = xnull])

AS_IF([test x$with_backend != null ],
[AX_CHECK_LIBRARY(LDAP, [ldap.h], [ldap], [], [AC_MSG_ERROR([Unable to find OpenLDAP])])])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h strings.h sys/param.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2 memset strcasecmp strdup strerror strncasecmp strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
