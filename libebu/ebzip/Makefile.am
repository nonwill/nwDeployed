localedir = $(datadir)/locale

LIBEB = $(top_builddir)/ebu/libebu.la
LIBEBUTILS = $(top_builddir)/libebutils/libebutils.a

bin_PROGRAMS = ebuzip
noinst_HEADERS = ebzip.h

ebuzip_SOURCES = ebzip.c ebzip1.c copyfile.c unzipbook.c unzipfile.c \
	zipbook.c zipfile.c zipinfobook.c zipinfofile.c sebxa.c speedup.c \
	unlinkfile.c
ebuzip_LDADD = $(LIBEBUTILS) $(LIBEB) $(ZLIBLIBS) $(INTLLIBS) $(ICONVLIBS)
ebuzip_DEPENDENCIES = $(LIBEB) $(LIBEBUTILS) $(ZLIBDEPS) $(INTLDEPS) \
	$(ICONVDEPS)

INCLUDES = -I../libebutils -I$(top_srcdir)/libebutils -I$(top_srcdir) \
	-I../ebu -I$(top_srcdir)/ebu $(INTLINCS) $(ZLIBINCS)

install-exec-hook:
	rm -f $(DESTDIR)$(bindir)/ebuunzip$(EXEEXT)
	ln $(DESTDIR)$(bindir)/`echo ebuzip|sed '$(transform)'`$(EXEEXT) \
	   $(DESTDIR)$(bindir)/`echo ebuunzip|sed '$(transform)'`$(EXEEXT) \
	   || cp $(DESTDIR)$(bindir)/`echo ebuzip|sed '$(transform)'`$(EXEEXT) \
	      $(DESTDIR)$(bindir)/`echo ebuunzip|sed '$(transform)'`$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/ebuzipinfo$(EXEEXT)
	ln $(DESTDIR)$(bindir)/`echo ebuzip|sed '$(transform)'`$(EXEEXT) \
	   $(DESTDIR)$(bindir)/`echo ebuzipinfo|sed '$(transform)'`$(EXEEXT) \
	   || cp $(DESTDIR)$(bindir)/`echo ebuzip|sed '$(transform)'`$(EXEEXT) \
	      $(DESTDIR)$(bindir)/`echo ebuzipinfo|sed '$(transform)'`$(EXEEXT)

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/`echo ebuunzip|sed '$(transform)'`$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/`echo ebuzipinfo|sed '$(transform)'`$(EXEEXT)
