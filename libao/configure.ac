dnl Process this file with autoconf to produce a configure script.

AC_INIT([libao],[1.2.2],[monty@xiph.org])
AM_INIT_AUTOMAKE([gnu 1.6])

AM_MAINTAINER_MODE

dnl $$$ben: Is that neccessary ? And why ?
AM_DISABLE_STATIC

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Library versioning
LIB_CURRENT=5
LIB_REVISION=1
LIB_AGE=1
AC_SUBST(LIB_CURRENT)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_AGE)

dnl Plugin versioning.  We use an integer version number much like LIB_CURRENT.
PLUGIN_VERSION=4

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

plugindir=$libdir/ao/plugins-$PLUGIN_VERSION
AC_SUBST(plugindir)

dnl ====================================
dnl Check for programs
dnl ====================================

cflags_save="$CFLAGS"
AC_PROG_CC
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

dnl ====================================
dnl Check dlopen 
dnl ====================================

dnl Currently use this to disable plugin support dlfcn.h
AC_CHECK_HEADERS([dlfcn.h],
	[AC_SEARCH_LIBS([dlopen],[dl],
		[AC_DEFINE([HAVE_DLOPEN],[1],
			[support dynamic linking loader])])])

dnl ====================================
dnl Set some general compile options
dnl ====================================

if test -z "$GCC"; then
        case $host in
        *-*-irix*)
                if test -z "$CC"; then
                        CC=cc
                fi
                PLUGIN_LDFLAGS="-export-dynamic -avoid-version"
                DEBUG="-g -signed"
                CFLAGS="-O2 -w -signed"
                PROFILE="-p -g3 -O2 -signed" ;;
        sparc-sun-solaris*)
                PLUGIN_LDFLAGS="-export-dynamic -avoid-version"
                DEBUG="-v -g"
                CFLAGS="-xO4 -fast -w -fsimple -native -xcg92"
                PROFILE="-v -xpg -g -xO4 -fast -native -fsimple -xcg92 -Dsuncc" ;;
        *)
                PLUGIN_LDFLAGS="-export-dynamic -avoid-version"
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p" ;;
        esac
else

        case $host in
        *-*-linux*)
                PLUGIN_LDFLAGS="-export-dynamic -avoid-version"
                DEBUG="-g -Wall -D_REENTRANT -D__NO_MATH_INLINES -fsigned-char"
                CFLAGS="-O20 -ffast-math -D_REENTRANT -fsigned-char"
                PROFILE="-pg -g -O20 -ffast-math -D_REENTRANT -fsigned-char";;
        sparc-sun-*)
                PLUGIN_LDFLAGS="-export-dynamic -avoid-version"
                DEBUG="-g -Wall -D__NO_MATH_INLINES -fsigned-char"
                CFLAGS="-O20 -ffast-math -D__NO_MATH_INLINES -fsigned-char"
                PROFILE="-pg -g -O20 -D__NO_MATH_INLINES -fsigned-char" ;;
        *-darwin*)
                PLUGIN_LDFLAGS="-module -avoid-version"
               	DEBUG="-g -Wall -D__NO_MATH_INLINES -fsigned-char"
                CFLAGS="-D__NO_MATH_INLINES -fsigned-char"
                PROFILE="-g -pg -D__NO_MATH_INLINES -fsigned-char" ;;
     	*)
                PLUGIN_LDFLAGS="-export-dynamic -avoid-version"
                DEBUG="-g -Wall -D__NO_MATH_INLINES -fsigned-char"
                CFLAGS="-O20 -D__NO_MATH_INLINES -fsigned-char"
                PROFILE="-O20 -g -pg -D__NO_MATH_INLINES -fsigned-char" ;;
        esac
fi
CFLAGS="$CFLAGS $cflags_save -DAO_BUILDING_LIBAO"
DEBUG="$DEBUG $cflags_save -DAO_BUILDING_LIBAO"
PROFILE="$PROFILE $cflags_save -DAO_BUILDING_LIBAO"

AC_SUBST(DEBUG)
AC_SUBST(PROFILE)

dnl =========================================
dnl Need -no-undefined for building win32 dll
dnl =========================================
dnl Should we do that for cygwin too ???
dnl I guess so but it needs to be tested.
case $host in
    *-mingw*|*-cygwin*)
	LIBAO_LA_LDFLAGS="-no-undefined"
	;;
    *)	
	LIBAO_LA_LDFLAGS=""
	;;
esac
AC_SUBST(LIBAO_LA_LDFLAGS)

dnl ===================================
dnl Check for symbol versioning support
dnl ===================================

AC_MSG_CHECKING([if libraries can be versioned])
GLD=`$LD --help < /dev/null 2>/dev/null | grep version-script`
if test "$GLD"; then
    have_ld_version_script=yes
    AC_MSG_RESULT(yes)
else
    have_ld_version_script=no
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, test "$have_ld_version_script" = "yes")

dnl ==============================
dnl Check for libraries
dnl ==============================

# we link to libpthread just in case one of our plugins does
# in which case this is required to avoid problems on dlclose()
AC_CHECK_LIB(pthread, pthread_kill)

dnl ==============================
dnl Checks for header files
dnl ==============================

dnl ==============================
dnl Select proper plugin options
dnl ==============================
case $host in
    *hpux*)
	DLOPEN_FLAG='(RTLD_LAZY)'
	SHARED_LIB_EXT='.sl'
	;;    
    *openbsd* | *netbsd* | *solaris2.7 | *darwin*)
	DLOPEN_FLAG='(RTLD_LAZY)'
	SHARED_LIB_EXT='.so'
	;;
    *)
	DLOPEN_FLAG='(RTLD_NOW | RTLD_GLOBAL)'
	SHARED_LIB_EXT='.so'
	;;
esac
AC_DEFINE_UNQUOTED(DLOPEN_FLAG, $DLOPEN_FLAG)
AC_DEFINE_UNQUOTED(SHARED_LIB_EXT, "$SHARED_LIB_EXT")

dnl ==============================
dnl Checks for types
dnl ==============================

AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)

case 2 in
        $ac_cv_sizeof_short) SIZE16="short";;
        $ac_cv_sizeof_int) SIZE16="int";;
esac

case 4 in
        $ac_cv_sizeof_short) SIZE32="short";;
        $ac_cv_sizeof_int) SIZE32="int";;
        $ac_cv_sizeof_long) SIZE32="long";;
esac

if test -z "$SIZE16"; then
        AC_MSG_ERROR(No 16 bit type found on this platform!)
fi
if test -z "$SIZE32"; then
        AC_MSG_ERROR(No 32 bit type found on this platform!)
fi

AC_SUBST(SIZE16)
AC_SUBST(SIZE32)

dnl ======================================
dnl Disable default use of SLP in roar 
dnl until such time as SLP doesn't block
dnl indefinitely
dnl ======================================

use_slp="no"
AC_ARG_ENABLE([roar-default-slp],
    [AS_HELP_STRING(
	[--enable-roar-default-slp],
	[allow Roar driver to use SLP in default search])],
	[use_slp="$enableval"],[use_slp="no"]
)
if test "x$use_slp" != "xyes"; then
   SLP_DEF="-DROAR_LIBROAR_CONFIG_WAS_NO_SLP"
fi
AC_SUBST([SLP_DEF])

dnl ======================================
dnl Detect possible output devices 
dnl ======================================

dnl Check for WMM

[have_wmm="no"]
AC_ARG_ENABLE([wmm],
    [AS_HELP_STRING(
	[--enable-wmm],
	[include WMM output plugin @<:@default=check@:>@])],
	[],[enable_wmm="check"]
)
AS_IF([test "x$enable_wmm" != "xno"],
[AC_CHECK_HEADERS([mmsystem.h],
   [
   AC_MSG_CHECKING([waveOut family functions])
   waveout_old_LIBS="$LIBS"; LIBS="$LIBS -lwinmm"
dnl Can't use AC_SEARCH_LIBS because symbols are decorated
   AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <windows.h>
#include <mmsystem.h>
int main(int na, char ** a) {
    return waveOutOpen(0,0,0,0,0,0);
}
])],[
have_wmm="yes";
AC_MSG_RESULT([yes])
AC_DEFINE([HAVE_WMM],[1],[Support Windows MultiMedia])
],[
AC_MSG_RESULT([no])
LIBS="$waveout_old_LIBS"
])
],[],[#include <windows.h>])])
AM_CONDITIONAL([HAVE_WMM],[test "x$have_wmm" = "xyes"])
AS_IF([test "x${have_wmm}" = "xyes"],[WMM_LIBS="-lwinmm"],[WMM_LIBS=""])
AC_SUBST([WMM_LIBS])

dnl Check for ESD

have_esd="no";
AC_ARG_ENABLE(esd, [  --enable-esd            include ESD output plugin ],
[ BUILD_ESD="$enableval" ],[ BUILD_ESD="yes" ])

if test "$BUILD_ESD" = "yes"; then
  AM_PATH_ESD(0.2.8, have_esd=yes)
fi
AM_CONDITIONAL(HAVE_ESD,test "x$have_esd" = xyes)


dnl Check for OSS

have_oss="no";
AC_CHECK_HEADERS(sys/soundcard.h, have_oss=yes)
AC_CHECK_HEADERS(machine/soundcard.h, have_oss=yes)
AM_CONDITIONAL(HAVE_OSS,test "${have_oss}" = "yes")


dnl Check for ALSA 0.9/1.0
have_alsa="no";
AC_ARG_ENABLE(alsa, [  --enable-alsa         include alsa 0.9/1.0 output plugin ],
[ BUILD_ALSA="$enableval" ],[ BUILD_ALSA="yes" ])
AC_ARG_ENABLE(alsa-mmap,
   [  --enable-alsa-mmap          use mmio with alsa ],
   [ BUILD_ALSAMMIO="$enableval" ],[ BUILD_ALSAMMIO="no" ])

if test "$BUILD_ALSA" = "yes"; then
   AC_CHECK_LIB(asound, snd_pcm_open, have_alsa=yes, have_alsa=no)
   AC_CHECK_HEADER(alsa/asoundlib.h, , have_alsa=no)
   if test "$BUILD_ALSAMMIO" = "yes" ; then
      AC_CHECK_HEADER(sys/mman.h, have_alsammio=yes, have_alsammio=no)
   fi

fi

if test "x$have_alsa" = xyes; then
	ALSA_LIBS="-lasound"
	if test "x$have_alsammio" = xyes; then
		AC_DEFINE(USE_ALSA_MMIO)
	fi
else
	ALSA_LIBS=""
fi
AM_CONDITIONAL(HAVE_ALSA,test "x$have_alsa" = xyes)
AC_SUBST(ALSA_LIBS)

dnl Decide whether we need to enable the workaround for broken OSS APIs
dnl such as the OSS emulation in ALSA.

AC_ARG_ENABLE(broken-oss, [  --enable-broken-oss           workaround for some OSS drivers (see README for details)],,
if test "x$have_alsa" = "xyes" -o "x$have_alsa" = "xyes"; then
   enable_broken_oss="yes"
fi)

if test "x$enable_broken_oss" = "xyes"; then
   AC_DEFINE(BROKEN_OSS)
   AC_MSG_WARN(Broken OSS API workaround enabled.  See README for details.)
fi
   
dnl Check for Sun audio

have_sun="no";
AC_CHECK_HEADERS(sys/audioio.h, have_sun=yes)
AM_CONDITIONAL(HAVE_SUN_AUDIO,test "${have_sun}" = yes)

dnl Check for libsndio audio

have_sndio="no";
AC_CHECK_HEADERS(sndio.h, have_sndio=yes)
AM_CONDITIONAL(HAVE_SNDIO_AUDIO,test "${have_sndio}" = yes)

dnl Check for roaraudio

have_roar="no";
AC_CHECK_HEADERS(libroar/config.h, have_roar=yes)
if test "x$have_roar" = "xyes"; then
   AC_MSG_CHECKING([if SLP lookup can be disabled in libroar])
   AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <libroar/config.h>
int main(int a, char **b) {
#ifdef ROAR_LIBROAR_CONFIG_WAS_NO_SLP
       return 0;
#else
	ROAR_LIBROAR_CONFIG_WAS_NO_SLP required
#endif
}
     ],
     [AC_MSG_RESULT([yes])],
     [
	AC_MSG_RESULT([no])
   	AC_MSG_RESULT([RoarAudio 0.4beta2 or later required to build Roar support])
   	have_roar="no"])])
fi
AM_CONDITIONAL(HAVE_ROAR_AUDIO,test "${have_roar}" = yes)

dnl Check for AIX audio

have_aix="no";
case $host in
    *-aix*)
	AC_CHECK_HEADERS(sys/audio.h, have_aix=yes)
	;;
esac
AM_CONDITIONAL(HAVE_AIX_AUDIO,test "x$have_aix" = xyes)

dnl Check for aRts

AC_ARG_ENABLE(arts, [  --enable-arts           include aRts output plugin ],
[ BUILD_ARTS="$enableval" ],[ BUILD_ARTS="maybe" ])

dnl aRts support is whacked on OS X, so don't build it by default
if test "$BUILD_ARTS" = "maybe"; then
  case $host in
         *-darwin*)
                 BUILD_ARTS=no;;
         *)
                 BUILD_ARTS=yes;;
  esac
fi

have_arts="no";
if test "$BUILD_ARTS" = "yes"; then
  AC_PATH_PROG(ARTSC_CONFIG, artsc-config)

  if test "x$ac_cv_path_ARTSC_CONFIG" != x
  then
	PKG_CHECK_MODULES(ARTSGLIB, glib-2.0 gthread-2.0)
	ARTS_CFLAGS="`$ac_cv_path_ARTSC_CONFIG --cflags` $ARTSGLIB_CFLAGS"
	ARTS_LIBS="`$ac_cv_path_ARTSC_CONFIG --libs` $ARTSGLIB_LIBS"
	SAVELIBS=$LIBS
	LIBS="$LIBS $ARTS_LIBS"
	AC_CHECK_FUNCS(arts_suspended)
	LIBS=$SAVELIBS
	have_arts=yes
  fi
fi
AM_CONDITIONAL(HAVE_ARTS,test "x${have_arts}" = xyes)

AC_SUBST(ARTS_CFLAGS)
AC_SUBST(ARTS_LIBS)


dnl Check for IRIX

have_irix="no";
case $host in
        *-*-irix*)
                AC_CHECK_LIB(audio, ALwritesamps, have_irix=yes, have_irix=no)
        ;;
esac
AM_CONDITIONAL(HAVE_IRIX,test "x$have_irix" = xyes)


dnl Check for MacOS X
case $host in
       *-darwin*)
               have_macosx=yes;;
       *)
               have_macosx=no;;
esac
AM_CONDITIONAL(HAVE_MACOSX,test "x$have_macosx" = xyes)


dnl Check for NAS

AC_ARG_ENABLE(nas, [  --enable-nas            include NAS output plugin ],
[ BUILD_NAS="$enableval" ],[ BUILD_NAS="yes" ])

have_nas="no"
if test "$BUILD_NAS" = "yes"; then
   AC_PATH_XTRA
   AC_CHECK_LIB(Xau, XauFileName, have_nas=yes, have_nas=no, $X_LIBS)
   AC_CHECK_LIB(audio, AuOpenServer, dummy="no-op", have_nas=no, -lXau $X_LIBS)
   
   ac_save_CPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS $X_CFLAGS"
   AC_CHECK_HEADER(audio/audiolib.h, dummy="no-op", have_nas=no)
   CPPFLAGS="$ac_save_CPPFLAGS"
fi

AM_CONDITIONAL(HAVE_NAS,test "x$have_nas" = xyes)

if test "x$have_nas" = xyes; then
	NAS_CFLAGS="$X_CFLAGS"
	NAS_LIBS="-laudio -lXau $X_LIBS"
else
	NAS_CFLAGS=""
	NAS_LIBS=""
fi
AC_SUBST(NAS_CFLAGS)
AC_SUBST(NAS_LIBS)

dnl Check for pulse

AC_ARG_ENABLE(pulse, [  --enable-pulse       include PulseAudio output plugin ],
[ BUILD_PULSE="$enableval" ],[ BUILD_PULSE="yes" ])

have_pulse="no";
if test "x$BUILD_PULSE" = "xyes" ; then
    PKG_CHECK_MODULES(PULSE, [ libpulse-simple >= 0.9 ],
	[have_pulse=yes],[have_pulse=no])
fi
AC_SUBST(PULSE_LIBS)

AM_CONDITIONAL(HAVE_PULSE,test "x$have_pulse" = xyes)

dnl Orphaned driver.  We'll probably dump it soon.
AM_CONDITIONAL(HAVE_SOLARIS,test "x$have_solaris" = xyes)

dnl Plugins get special LDFLAGS
AC_SUBST(PLUGIN_LDFLAGS)


AC_OUTPUT([Makefile src/Makefile doc/Makefile include/Makefile include/ao/Makefile include/ao/os_types.h src/plugins/Makefile src/plugins/esd/Makefile src/plugins/oss/Makefile src/plugins/alsa/Makefile src/plugins/sun/Makefile src/plugins/irix/Makefile src/plugins/arts/Makefile src/plugins/macosx/Makefile src/plugins/nas/Makefile src/plugins/pulse/Makefile src/plugins/sndio/Makefile src/plugins/roar/Makefile ao.pc])

AS_AC_EXPAND(LIBDIR, ${libdir})
AS_AC_EXPAND(INCLUDEDIR, ${includedir})
AS_AC_EXPAND(BINDIR, ${bindir})
AS_AC_EXPAND(DOCDIR, ${docdir})

AC_MSG_RESULT([
------------------------------------------------------------------------
  $PACKAGE $VERSION:  Automatic configuration OK.

  Backends to be built:

    AU file output: .............. yes
    RAW file output: ............. yes
    WAV file output: ............. yes

    AIX live output: ............. ${have_aix}
    ALSA live output: ............ ${have_alsa}
    ARTS live output: ............ ${have_arts}
    ESD live output: ............. ${have_esd}
    IRIX live output: ............ ${have_irix}
    MACOSX live output: .......... ${have_macosx}
    NAS live output: ............. ${have_nas}
    NULL live output: ............ yes
    OSS live output: ............. ${have_oss}
    PULSE live output: ........... ${have_pulse}
    ROAR live output: ............ ${have_roar}
    SNDIO live output: ........... ${have_sndio}
    SUN live output: ............. ${have_sun}
    Windows WMM live output: ..... ${have_wmm}

  Installation paths:

    libao: ....................... ${LIBDIR}
    C header files: .............. ${INCLUDEDIR}/ao
    Documentation: ............... ${DOCDIR}

  Building:

    Type 'make' to compile $PACKAGE.

    Type 'make install' to install $PACKAGE.

------------------------------------------------------------------------
])
