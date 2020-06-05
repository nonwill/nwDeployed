ACLOCAL_AMFLAGS = -I m4

if ENABLE_SAMPLES
SAMPLES_SUBDIR = samples
else
SAMPLES_SUBDIR =
endif

SUBDIRS = ebu libebutils ebappendix ebfont ebinfo ebrefile ebstopcode ebzip \
	doc po-eb po-ebutils m4 $(SAMPLES_SUBDIR)

EXTRA_DIST = ChangeLog.0 ChangeLog.1 ChangeLog.2 move-if-change \
   ebu.conf.in misc/ebfixlog misc/ebdump README.org
CLEANFILES = ebu.conf

appendixdir = $(pkgdatadir)/appendix
appendix_DATA =
sysconf_DATA = ebu.conf

ebu.conf: ebu.conf.in Makefile
	rm -f $@
	sed -e 's;\@VERSION\@;$(VERSION);' \
	   -e 's;\@EBCONF_EBINCS\@;$(EBCONF_EBINCS);' \
	   -e 's;\@EBCONF_EBLIBS\@;$(EBCONF_EBLIBS);' \
	   -e 's;\@EBCONF_ZLIBINCS\@;$(EBCONF_ZLIBINCS);' \
	   -e 's;\@EBCONF_ZLIBLIBS\@;$(EBCONF_ZLIBLIBS);' \
	   -e 's;\@ENABLE_PTHREAD\@;$(ENABLE_PTHREAD);' \
	   -e 's;\@PTHREAD_CPPFLAGS\@;$(PTHREAD_CPPFLAGS);' \
	   -e 's;\@PTHREAD_CFLAGS\@;$(PTHREAD_CFLAGS);' \
	   -e 's;\@PTHREAD_LDFLAGS\@;$(PTHREAD_LDFLAGS);' \
	   -e 's;\@ENABLE_NLS\@;$(ENABLE_NLS);' \
	   -e 's;\@EBCONF_INTLINCS\@;$(EBCONF_INTLINCS);' \
	   -e 's;\@EBCONF_INTLLIBS\@;$(EBCONF_INTLLIBS);' \
	   -e 's;\@ENABLE_EBNET\@;$(ENABLE_EBNET);' \
	   $(srcdir)/$@.in > $@.tmp
	cp $@.tmp $@
	chmod 644 $@
	rm -f $@.tmp

gttest.mo: gttest.po
	$(MSGFMT) -o gttest.mo gttest.po
