dnl *
dnl * Copyright (c) 2004-2006  Motoyuki Kasahara
dnl *
dnl * Redistribution and use in source and binary forms, with or without
dnl * modification, are permitted provided that the following conditions
dnl * are met:
dnl * 1. Redistributions of source code must retain the above copyright
dnl *    notice, this list of conditions and the following disclaimer.
dnl * 2. Redistributions in binary form must reproduce the above copyright
dnl *    notice, this list of conditions and the following disclaimer in the
dnl *    documentation and/or other materials provided with the distribution.
dnl * 3. Neither the name of the project nor the names of its contributors
dnl *    may be used to endorse or promote products derived from this software
dnl *    without specific prior written permission.
dnl * 
dnl * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
dnl * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
dnl * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
dnl * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
dnl * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
dnl * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
dnl * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
dnl * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
dnl * SUCH DAMAGE.
dnl *

AC_DEFUN([eb_GNU_GETTEXT], [dnl
  INTLINCS=
  INTLDEPS=
  INTLLIBS=

  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_LIBTOOL])

  AC_CHECK_HEADERS([locale.h nl_types.h])
  AC_CHECK_FUNCS([setlocale])

  AM_LC_MESSAGES

  dnl * 
  dnl * --enable-nls option
  dnl * 
  AC_ARG_ENABLE(nls,
    AC_HELP_STRING([--enable-nls], [Native Language Support [[yes]]]),
    ENABLE_NLS=$enableval, ENABLE_NLS=auto)

  dnl * 
  dnl * --with-gettext-includes option
  dnl * 
  AC_ARG_WITH(gettext-includes,
    AC_HELP_STRING([--with-gettext-includes=DIR],
      [gettext include files are in DIR]),
  [gettext_includes="-I${withval}"], [gettext_includes=''])

  dnl * 
  dnl * --with-gettext-libraries option
  dnl * 
  AC_ARG_WITH(gettext-libraries,
    AC_HELP_STRING([--with-gettext-libraries=DIR],
      [gettext library files are in DIR]),
  [gettext_libraries="-L${withval}"], [gettext_libraries=''])

  dnl * 
  dnl * --with-iconv-includes option
  dnl * 
  AC_ARG_WITH(iconv-includes,
    AC_HELP_STRING([--with-iconv-includes=DIR],
      [iconv include files are in DIR]),
  [iconv_includes="-I${withval}"], [iconv_includes=''])

  dnl * 
  dnl * --with-iconv-libraries option
  dnl * 
  AC_ARG_WITH(iconv-libraries,
    AC_HELP_STRING([--with-iconv-libraries=DIR],
      [iconv library files are in DIR]),
  [iconv_libraries="-L${withval}"], [iconv_libraries=''])

  dnl *
  dnl * Check iconv(), iconv.h and -liconv.
  dnl *
  ICONVINCS=
  ICONVLIBS=
  save_CPPFLAGS=$CPPFLAGS
  save_LIBS=$LIBS
  CPPFLAGS="$save_CPPFLAGS $iconv_includes"
  LIBS="$save_LIBS $iconv_libraries"
  AC_CHECK_LIB(iconv, iconv_open,
    [ICONVLIBS="$iconv_libraries -liconv"; LIBS="$LIBS -liconv"],
    AC_CHECK_LIB(iconv, libiconv_open,
    [ICONVLIBS="$iconv_libraries -liconv"; LIBS="$LIBS -liconv"]))
  AC_CHECK_FUNCS(iconv_open libiconv_open locale_charset)
  AC_CHECK_HEADERS(iconv.h libcharset.h)
  if test $ac_cv_func_iconv_open != no; then
    ICONVINCS="$iconv_includes"
  fi
  CPPFLAGS=$save_CPPFLAGS
  LIBS=$save_LIBS
  AC_SUBST(ICONVINCS)
  AC_SUBST(ICONVLIBS)

  dnl *
  dnl * Check gettext().
  dnl * 
  INTLINCS=
  INTLLIBS=
  try_nls=no

  AC_MSG_CHECKING([for NLS support])

  if test $ENABLE_NLS != no; then
    save_CPPFLAGS=$CPPFLAGS
    save_LIBS=$LIBS

    dnl *
    dnl * Test 1: Try to link both libintl and libiconv.
    dnl *
    CPPFLAGS="$save_CPPFLAGS $gettext_includes"
    LIBS="$save_LIBS $gettext_libraries -lintl $iconv_libraries -liconv"
    AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#ifdef ENABLE_NLS
#undef ENABLE_NLS
#endif
#define ENABLE_NLS 1
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>

int
main()
{
#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif
  bindtextdomain("gttest", ".locale");
  textdomain("gttest");
  gettext("foo");
  return 0;
}
])], 
    try_nls=yes, try_nls=no)

    if test "$try_nls" = yes; then
      INTLINCS="$gettext_includes"
      INTLLIBS="$gettext_libraries -lintl $iconv_libraries -liconv"
    fi

    dnl *
    dnl * Test 2: Try to link libintl.
    dnl * 
    if test "$try_nls" = no; then
      CPPFLAGS="$save_CPPFLAGS $gettext_includes"
      LIBS="$save_LIBS $gettext_libraries -lintl"
      AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#ifdef ENABLE_NLS
#undef ENABLE_NLS
#endif
#define ENABLE_NLS 1
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>

int
main()
{
#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif
  bindtextdomain("gttest", ".locale");
  textdomain("gttest");
  gettext("foo");
  return 0;
}
])], 
      try_nls=yes, try_nls=no)

      if test "$try_nls" = yes; then
        INTLINCS="$gettext_includes"
        INTLLIBS="$gettext_libraries -lintl"
      fi
    fi

    dnl *
    dnl * Test 3: Try to link libiconv.
    dnl * 
    if test "$try_nls" = no; then
      CPPFLAGS="$save_CPPFLAGS"
      LIBS="$save_LIBS $iconv_libraries -liconv"
      AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#ifdef ENABLE_NLS
#undef ENABLE_NLS
#endif
#define ENABLE_NLS 1
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>

int
main()
{
#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif
  bindtextdomain("gttest", ".locale");
  textdomain("gttest");
  gettext("foo");
  return 0;
}
])], 
      try_nls=yes, try_nls=no)

      if test "$try_nls" = yes; then
        INTLINCS=
        INTLLIBS="$iconv_libraries -liconv"
      fi
    fi

    dnl *
    dnl * Test 4: Try to link libc only.
    dnl * 
    if test "$try_nls" = no; then
      CPPFLAGS="$save_CPPFLAGS"
      LIBS="$save_LIBS"
      AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#ifdef ENABLE_NLS
#undef ENABLE_NLS
#endif
#define ENABLE_NLS 1
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <libintl.h>

int
main()
{
#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif
  bindtextdomain("gttest", ".locale");
  textdomain("gttest");
  gettext("foo");
  return 0;
}
])], 
      try_nls=yes, try_nls=no)

      if test "$try_nls" = yes; then
        INTLINCS=
        INTLLIBS=
      fi
    fi

    CPPFLAGS=$save_CPPFLAGS
    LIBS=$save_LIBS
  fi

  if test $ENABLE_NLS = auto; then
    ENABLE_NLS=$try_nls
  fi

  AC_MSG_RESULT($try_nls)

  if test $ENABLE_NLS = yes; then
    if test $try_nls = no; then
      AC_MSG_ERROR(gettext not available)
    fi
  fi

  AC_SUBST(ENABLE_NLS)
  AC_SUBST(INTLINCS)
  AC_SUBST(INTLLIBS)
  localedir='$(datadir)/locale'
  AC_SUBST(localedir)
  if test $ENABLE_NLS = yes; then
    AC_DEFINE(ENABLE_NLS, 1, [Define if NLS is requested])
  fi

  dnl *
  dnl * Check msgfmt and xgettext commands.
  dnl * 
  AC_PATH_PROGS(MSGFMT, gmsgfmt msgfmt, :)
  AC_PATH_PROGS(XGETTEXT, gxgettext xgettext, :)
  MSGMERGE=msgmerge
  AC_SUBST(MSGMERGE)
])
