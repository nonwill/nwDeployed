localedir = $(datadir)/locale

LIBEB = $(top_builddir)/ebu/libebu.la
LIBEBUTILS = $(top_builddir)/libebutils/libebutils.a

bin_PROGRAMS = eburefile

eburefile_SOURCES = ebrefile.c
eburefile_LDADD = $(LIBEBUTILS) $(LIBEB) $(ZLIBLIBS) $(INTLLIBS) $(ICONVLIBS)
eburefile_DEPENDENCIES = $(LIBEBUTILS) $(LIBEB) $(ZLIBDEPS) $(INTLDEPS) \
	$(ICONVDEPS)

INCLUDES = -I../libebutils -I$(top_srcdir)/libebutils -I$(top_srcdir) \
	-I../ebu -I$(top_srcdir)/ebu $(INTLINCS)
