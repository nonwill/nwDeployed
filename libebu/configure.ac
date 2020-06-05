dnl *
dnl * Process this file with autoconf to produce a configure script.
dnl *
AC_INIT(ebu, 4.4.3-20200310)
AC_CONFIG_SRCDIR(ebu/eb.c)
AC_PREREQ(2.54)
AM_INIT_AUTOMAKE

dnl *
dnl * Library version info.
dnl *
LIBEB_VERSION_INFO=16:0:0
AC_SUBST(LIBEB_VERSION_INFO)

EB_VERSION_MAJOR=4
EB_VERSION_MINOR=4
AC_SUBST(EB_VERSION_MAJOR)
AC_SUBST(EB_VERSION_MINOR)

dnl *
dnl * Set mailing address.
dnl *
MAILING_ADDRESS='kzhr@d1.dion.ne.jp'
AC_SUBST(MAILING_ADDRESS)
AC_DEFINE_UNQUOTED(MAILING_ADDRESS, "$MAILING_ADDRESS", [Mailing address])

dnl *
dnl * Programs
dnl *
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_INSTALL

AC_OPENMP
CFLAGS="$CFLAGS $OPENMP_CFLAGS"

AC_PATH_PROG(PERL, perl, no)
if test "$PERL" = no; then
    AC_MSG_WARN(perl not found)
    PERL=/usr/bin/perl
fi

dnl *
dnl * large file support.
dnl *
eb_SYS_LARGEFILE

dnl * 
dnl * GNU gettext
dnl * 
eb_GNU_GETTEXT(external)

dnl *
dnl * --enable-samples option.
dnl *
AC_ARG_ENABLE(samples,
[  --enable-samples        compile sample programs [default=no]],
[case "${enableval}" in
   yes) samples=true  ;;
   no)  samples=false ;;
   *)   AC_MSG_ERROR(invalid argument to --enable-samples) ;;
esac], samples=false)
AM_CONDITIONAL(ENABLE_SAMPLES, test ${samples} = true)

dnl *
dnl * --enable-pthread option.
dnl *
AC_ARG_ENABLE(pthread,
AC_HELP_STRING([--enable-pthread], [build pthread safe libraries [[no]]]),
[case "${enableval}" in
   yes) ENABLE_PTHREAD=yes  ;;
   no)  ENABLE_PTHREAD=no ;;
   *)   AC_MSG_ERROR(invalid argument to --enable-pthread) ;;
esac], ENABLE_PTHREAD=no)

dnl *
dnl * --with-pthread-cppflags option.
dnl *
AC_ARG_WITH(pthread-cppflags,
AC_HELP_STRING([--with-pthread-cppflags=FLAGS],
    [additional CPPFLAGS for pthread support]),
[PTHREAD_CPPFLAGS="${withval}"], [PTHREAD_CPPFLAGS=''])

dnl *
dnl * --with-pthread-cflags option.
dnl *
AC_ARG_WITH(pthread-cflags,
AC_HELP_STRING([--with-pthread-cflags=FLAGS], 
    [additional CFLAGS for pthread support]),
[PTHREAD_CFLAGS="${withval}"], [PTHREAD_CFLAGS=''])

dnl *
dnl * --with-pthread-ldflags option.
dnl *
AC_ARG_WITH(pthread-ldflags,
AC_HELP_STRING([--with-pthread-ldflags=FLAGS],
    [additional LDFLAGS for pthread support]),
[PTHREAD_LDFLAGS="${withval}"], [PTHREAD_LDFLAGS=''])

dnl *
dnl * --with-pkgdocdir option.
dnl *
AC_ARG_WITH(pkgdocdir,
AC_HELP_STRING([--with-pkgdocdir=DIR],
    [HTML documents in DIR [[default=DATADIR/ebu/doc]]]),
[pkgdocdir="${withval}"],[pkgdocdir='${datadir}/ebu/doc'])
AC_SUBST(pkgdocdir)

dnl *
dnl * Check for pthread.
dnl *
AC_MSG_CHECKING(for pthread)
if test $ENABLE_PTHREAD = no; then
    try_pthread=no
else
    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    save_LDFLAGS=$LDFLAGS
    CPPFLAGS="$CPPFLAGS $PTHREAD_CPPFLAGS"
    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
    LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
    AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <pthread.h>
static pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER; 

int
main()
{
    pthread_mutex_lock(&m);
    return 0;
}
])], 
        try_pthread=yes, try_pthread=no)
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    LDFLAGS=$save_LDFLAGS
fi
AC_MSG_RESULT($try_pthread)
if test $ENABLE_PTHREAD = yes; then
    if test $try_pthread = no; then
        AC_MSG_ERROR(pthread not available)
    fi
fi

if test $ENABLE_PTHREAD = yes; then
    AC_DEFINE(ENABLE_PTHREAD, 1, [Define if pthread support is enabled.])
    CPPFLAGS="$CPPFLAGS $PTHREAD_CPPFLAGS"
    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
    LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
fi

dnl *
dnl * --enable-libdeflate option.
dnl *
AC_ARG_ENABLE(libdeflate,
AC_HELP_STRING([--enable-libdeflate], [build with libdeflate library [[no]]]),
[case "${enableval}" in
   yes) ENABLE_LIBDEFLATE=yes  ;;
   no)  ENABLE_LIBDEFLATE=no ;;
   *)   AC_MSG_ERROR(invalid argument to --enable-libdeflate) ;;
esac], ENABLE_LIBDEFLATE=no)

dnl *
dnl * --with-libdeflate-includes option.
dnl *
AC_ARG_WITH(libdeflate-includes,
AC_HELP_STRING([--with-libdeflate-includes=DIR], [libdeflate include files are in DIR]),
[libdeflate_includedir="${withval}"], [libdeflate_includedir=''])

if test "X$libdeflate_includedir" != X; then
    LIBDEFLATEINCS="-I$libdeflate_includedir"
else
    LIBDEFLATEINCS=''
fi

dnl *
dnl * --with-libdeflate-libraries option.
dnl *
AC_ARG_WITH(libdeflate-libraries,
AC_HELP_STRING([--with-libdeflate-libraries=DIR], [libdeflate library files are in DIR]),
[libdeflate_libdir="${withval}"], [libdeflate_libdir=''])

if test "X$libdeflate_libdir" != X; then
   LIBDEFLATELIBS="-L$libdeflate_libdir -ldeflate"
   LIBDEFLATEDEPS=''
else
    LIBDEFLATELIBS='-ldeflate'
    LIBDEFLATEDEPS=''
fi

dnl *
dnl * Check for libdeflate.
dnl *
AC_MSG_CHECKING(for libdeflate)
save_CPPFLAGS=$CPPFLAGS
save_LIBS=$LIBS
CPPFLAGS="$CPPFLAGS $LIBDEFLATEINCS"
LIBS="$LIBS $LIBDEFLATELIBS"
AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <libdeflate.h>

int
main()
{
    struct libdeflate_compressor *c; c = libdeflate_alloc_compressor(12);
    return 0;
}
])],
          try_libdeflate=yes, try_libdeflate=no)
CPPFLAGS=$save_CPPFLAGS
LIBS=$save_LIBS
AC_MSG_RESULT($try_libdeflate)
if test $try_libdeflate = no; then
    if test $ENABLE_LIBDEFLATE = yes; then
        AC_MSG_ERROR(libdeflate not found)
    fi
fi
if test $ENABLE_LIBDEFLATE = yes; then
    AC_DEFINE(ENABLE_LIBDEFLATE, 1, [Define if libdeflate support is enabled.])
fi

dnl *
dnl * --with-zlib-includes option.
dnl *
AC_ARG_WITH(zlib-includes,
AC_HELP_STRING([--with-zlib-includes=DIR], [zlib include files are in DIR]),
[z_includedir="${withval}"], [z_includedir=''])

if test "X$z_includedir" != X; then
    ZLIBINCS="-I$z_includedir"
else
    ZLIBINCS=''
fi

dnl *
dnl * --with-zlib-libraries option.
dnl *
AC_ARG_WITH(zlib-libraries,
AC_HELP_STRING([--with-zlib-libraries=DIR], [zlib library files are in DIR]),
    [z_libdir="${withval}"], [z_libdir=''])

if test "X$z_libdir" != X; then
    ZLIBLIBS="-L$z_libdir -lz"
    ZLIBDEPS=''
else
    ZLIBLIBS='-lz'
    ZLIBDEPS=''
fi

dnl *
dnl * Check for zlib.
dnl *
AC_MSG_CHECKING(for zlib)
save_CPPFLAGS=$CPPFLAGS
save_LIBS=$LIBS
CPPFLAGS="$CPPFLAGS $ZLIBINCS"
LIBS="$LIBS $ZLIBLIBS"
AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <zlib.h>

int
main()
{
    z_stream stream; inflate(&stream, Z_FINISH);
    return 0;
}
])], 
     try_zlib=yes, try_zlib=no)
CPPFLAGS=$save_CPPFLAGS
LIBS=$save_LIBS
AC_MSG_RESULT($try_zlib)
if test $try_zlib = no; then
    if test $try_libdeflate = no; then
        AC_MSG_ERROR(zlib not found)
    fi
fi
if test $ENABLE_LIBDEFLATE = yes; then
   ZLIBINCS=$LIBDEFLATEINCS
   ZLIBLIBS=$LIBDEFLATELIBS
   ZLIBDEPS=$LIBDEFLATEDEPS
fi
AC_SUBST(ZLIBINCS)
AC_SUBST(ZLIBLIBS)
AC_SUBST(ZLIBDEPS)

dnl *
dnl * --enable-ebnet option.
dnl *
AC_ARG_ENABLE(ebnet,
AC_HELP_STRING([--enable-ebnet], [EBNET support [[yes]]]),
[ENABLE_EBNET="${enableval}"], [ENABLE_EBNET='yes'])
AC_SUBST(ENABLE_EBNET)
AM_CONDITIONAL(ENABLE_EBNET, test X$ENABLE_EBNET = Xyes)

dnl *
dnl * --enable-ipv6 option.
dnl *
AC_ARG_ENABLE(ipv6,
AC_HELP_STRING([--enable-ipv6],
    [IPv6 support for EBNET [[yes]] (if the system supports IPv6)]),
[ENABLE_IPV6="${enableval}"], [ENABLE_IPV6='auto'])

dnl *
dnl * --enable-winsock option.
dnl *
AC_ARG_ENABLE(winsock2,
AC_HELP_STRING([--enable-winsock2],
    [Use Winsock2 for EBNET [[auto]]]),
[ENABLE_WINSOCK2="${enableval}"], [ENABLE_WINSOCK2='auto'])

dnl * 
dnl * Compiler Characteristics.
dnl * 

dnl * 
dnl * Header Files.
dnl * 
AC_CHECK_HEADERS(direct.h langinfo.h mbstring.h pthread.h)

dnl * 
dnl * Structures.
dnl * 

dnl * 
dnl * Libraries.
dnl * 
AC_CHECK_LIB(resolv, res_query)
if test "$ac_cv_lib_resolv_res_query" = no; then
    AC_CHECK_LIB(bind, res_query)
fi
AC_CHECK_LIB(nsl, gethostname)
AC_CHECK_LIB(socket, socket)

dnl * 
dnl * Library Functions.
dnl * 
AC_CHECK_FUNCS(nl_langinfo _getdcwd atoll _atoi64)
AC_REPLACE_FUNCS(strcasecmp)

dnl * 
dnl * Typedefs.
dnl * 
AC_TYPE_MODE_T
AC_CHECK_TYPE(ssize_t, int)

dnl *
dnl * System services
dnl *
AC_EXEEXT
AC_OBJEXT
AM_CONDITIONAL(EXEEXT_EXE, test X$ac_exeext = Xyes)
if test X$ac_exeext = X.exe; then
    AC_DEFINE(EXEEXT_EXE, 1, [Define if command names have the suffix \`.exe'])
fi

dnl *
dnl * Check for Winsock2
dnl *
if test "$ENABLE_EBNET" = yes; then
   AC_CHECK_HEADERS(ws2tcpip.h sys/socket.h)
   if test "$ENABLE_WINSOCK2" = auto; then
      if test "$ac_cv_header_ws2tcpip_h" = yes; then
         if test "$ac_cv_header_sys_socket_h" = no; then
	    ENABLE_WINSOCK2=yes
         else
            ENABLE_WINSOCK2=no
         fi
      else 
      ENABLE_WINSOCK2=no
      fi
   fi

   if test "$ENABLE_WINSOCK2" = yes; then
      if test "$ac_cv_header_ws2tcpip_h" = no; then
         AC_MSG_ERROR(Winsock2 not available)
      fi
   AC_DEFINE(ENABLE_WINSOCK2, 1, [Define if build with Winsock2])
   fi
else
   ENABLE_WINSOCK2=no
fi
AC_MSG_CHECKING(for Winsock2 support)
AM_CONDITIONAL(ENABLE_WINSOCK2, test X$ENABLE_WINSOCK2 = Xyes)
AC_MSG_RESULT($ENABLE_WINSOCK2)

dnl *
dnl * Check for getopt_long
dnl *
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_FUNCS(getopt_long)
AM_CONDITIONAL(HAVE_GETOPT_LONG, test X$HAVE_GETOPT_LONG = Xyes)

dnl *
dnl * ebnet support
dnl *
if test "$ENABLE_EBNET" = yes; then
    AC_DEFINE(ENABLE_EBNET, 1, [Define if build with ebnet support])

    AC_TYPE_IN_PORT_T
    AC_TYPE_SA_FAMILY_T
    AC_TYPE_SOCKLEN_T
    AC_STRUCT_IN6_ADDR
    AC_STRUCT_SOCKADDR_IN6
    AC_STRUCT_SOCKADDR_STORAGE
    AC_DECL_IN6ADDR_ANY
    AC_DECL_IN6ADDR_LOOPBACK

    if test "$ENABLE_WINSOCK2" = yes; then
       AC_CHECK_LIB(ws2_32, getaddrinfo)
       AC_CHECK_FUNCS(getaddrinfo getnameinfo)
    else
       AC_CHECK_FUNCS(getaddrinfo getnameinfo gai_strerror)
    fi
    if test "$ENABLE_IPV6" = auto; then
       if test "$ac_cv_struct_in6_addr$ac_cv_func_getaddrinfo" = yesyes; then
       	  ENABLE_IPV6=yes
       else
          ENABLE_IPV6=no
       fi
    fi

    AC_MSG_CHECKING(for IPv6 support)
    if test "$ENABLE_IPV6" = yes; then
        if test "$ac_cv_struct_in6_addr" = no; then
            AC_MSG_ERROR(IPv6 not available)
        fi
        if test "${ac_cv_func_getaddrinfo}" = no; then
            AC_MSG_ERROR(IPv6 not available)
        fi
        AC_DEFINE(ENABLE_IPV6, 1, [Define if build with IPv6 support])
    fi
    AC_MSG_RESULT($ENABLE_IPV6)
fi

dnl *
dnl * DOS style path support
dnl * 
AC_CANONICAL_HOST
AC_MSG_CHECKING(for DOS style pathnames)

dos_file_path="no"
case ${host_os} in
  *djgpp | *mingw* | *emx*) dos_file_path="yes" ;;
esac

if test "$dos_file_path" = yes; then
    AC_DEFINE(DOS_FILE_PATH, 1, [Use DOS style pathnames])
fi
AC_MSG_RESULT($dos_file_path)

dnl *
dnl * Substitusion for eb.conf.
dnl *
AC_SUBST(ENABLE_LIBDEFLATE)
EBCONF_ZLIBINCS=$ZLIBINCS
EBCONF_ZLIBLIBS=$ZLIBLIBS
AC_SUBST(EBCONF_ZLIBINCS)
AC_SUBST(EBCONF_ZLIBLIBS)

AC_SUBST(ENABLE_PTHREAD)
AC_SUBST(PTHREAD_CPPFLAGS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LDFLAGS)

EBCONF_INTLINCS=$INTLINCS
EBCONF_INTLLIBS=$INTLLIBS
AC_SUBST(EBCONF_INTLINCS)
AC_SUBST(EBCONF_INTLLIBS)

EBCONF_EBINCS='-I$(includedir)'
if test "$ENABLE_WINSOCK2" = yes; then
   EBCONF_EBLIBS='-L$(libdir) -lebu -lws2_32'
else
   EBCONF_EBLIBS='-L$(libdir) -lebu'
fi
AC_SUBST(EBCONF_EBINCS)
AC_SUBST(EBCONF_EBLIBS)

dnl * 
dnl * Output Files.
dnl * 
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile ebu/Makefile libebutils/Makefile ebappendix/Makefile
    ebfont/Makefile ebinfo/Makefile ebrefile/Makefile ebstopcode/Makefile 
    ebzip/Makefile doc/Makefile po-eb/Makefile po-ebutils/Makefile 
    m4/Makefile samples/Makefile])
AC_OUTPUT
