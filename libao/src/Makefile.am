## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
SUBDIRS = plugins

AM_CPPFLAGS = -I$(top_builddir)/include/ao \
	      -I$(top_srcdir)/include \
	      -I$(top_srcdir)/include/ao \
	      -DAO_PLUGIN_PATH=\"@plugindir@\"

lib_LTLIBRARIES = libao.la

if HAVE_WMM
wmm=ao_wmm.c
else
wmm=
endif

libao_la_SOURCES = audio_out.c config.c ao_null.c ao_wav.c ao_au.c ao_raw.c ao_aixs.c $(wmm)
libao_la_LDFLAGS = @LIBAO_LA_LDFLAGS@ -version-info @LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@
if HAVE_LD_VERSION_SCRIPT
  libao_la_LDFLAGS += -Wl,--version-script=$(top_srcdir)/src/ao.vers
endif


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

EXTRA_DIST = ao_wmm.c ao.vers
