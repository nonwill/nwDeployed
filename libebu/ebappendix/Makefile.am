bin_SCRIPTS = ebuappendix

EXTRA_DIST = ebappendix.in
CLEANFILES = ebuappendix ebappendix.tmp

ebuappendix: ebappendix.in $(top_srcdir)/libebutils/getopt_long.pl Makefile
	rm -f ebuappendix
	sed -e 's;\@PERL\@;$(PERL);' \
	    $(srcdir)/ebappendix.in | sed '/^$$/q' > ebappendix.tmp
	cat $(top_srcdir)/libebutils/getopt_long.pl >> ebappendix.tmp
	sed -e 's;\@VERSION\@;$(VERSION);' \
	   -e 's;\@MAILING_ADDRESS\@;$(MAILING_ADDRESS);' \
	   $(srcdir)/ebappendix.in | sed '1,/^$$/d' >> ebappendix.tmp
	cp ebappendix.tmp ebuappendix
	chmod +x ebuappendix
	rm -f ebappendix.tmp
