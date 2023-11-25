AC_INIT
AC_CONFIG_SRCDIR([common/msglog.c])
AC_CONFIG_HEADERS([config.h])

AC_CHECK_TOOL(AR, ar)
AC_PROG_CC
AC_C_BIGENDIAN
AC_CHECK_HEADERS(byteswap.h sys/stat.h)
AC_CHECK_DECLS([__builtin_bswap32,bswap_32],,,[[
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif
]])
AC_CHECK_FUNCS(pread stat)

AC_CONFIG_FILES([Makefile common/Makefile network/Makefile server/Makefile
          datasource/Makefile cmdline/Makefile])
AC_OUTPUT
