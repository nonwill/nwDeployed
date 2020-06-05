noinst_LIBRARIES = libebutils.a
libebutils_a_SOURCES = ebutils.c getopt.c getumask.c makedir.c puts_eucjp.c \
	samefile.c strlist.c yesno.c
libebutils_a_LIBADD = @LIBOBJS@

dist_noinst_HEADERS = getopt.h getumask.h makedir.h samefile.h strlist.h \
	yesno.h
nodist_noinst_HEADERS = ebutils.h

EXTRA_DIST = getopt_long.pl ebutils.h.in
CLEANFILES = stamp-ebutils-h ebutils.h
BUILT_SOURCES = ebutils.h

INCLUDES = -I$(top_srcdir) @INTLINCS@ @ICONVINCS@

ebutils.h: stamp-ebutils-h
stamp-ebutils-h: ebutils.h.in Makefile
	rm -f ebutils.h.tmp
	echo '/* automatically generated from ebutils.h.in. */' \
	   > ebutils.h.tmp
	sed -e 's;\@localedir\@;$(localedir);' \
	   $(srcdir)/ebutils.h.in >> ebutils.h.tmp
	$(top_srcdir)/move-if-change ebutils.h.tmp ebutils.h
	@echo timestamp > stamp-ebutils-h
