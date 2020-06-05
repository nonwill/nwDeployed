pkgdoc_DATA = eb.html ebappendix.html ebfont.html ebinfo.html ebrefile.html \
	ebstopcode.html ebzip.html 

EXTRA_DIST = eb.html.in ebappendix.html.in ebfont.html.in ebinfo.html.in \
	ebrefile.html.in ebstopcode.html.in ebzip.html.in eb.css ebutils.css \
	c2html html-include html-index html-split html-toc

CLEANFILES = *.html

install-data-hook:
	for i in $(pkgdoc_DATA); do \
	    base="`echo $$i | sed -e 's/\.html//'`"; \
	    for j in $$base-[0-9][0-9].html; do \
		test -r $$j || continue; \
		echo " $(INSTALL_DATA) '$$j' '$(DESTDIR)$(pkgdocdir)/$$j'"; \
		$(INSTALL_DATA) "$$j" "$(DESTDIR)$(pkgdocdir)/$$j"; \
	    done; \
	done

uninstall-hook:
	for i in $(pkgdoc_DATA); do \
	    base="`echo $$i | sed -e 's/\.html//'`"; \
	    rm -f "$(DESTDIR)$(pkgdocdir)"/$$base-*.html; \
	done

version.html:
	rm -f $@
	echo $(VERSION) > version.html

splitted: eb-splitted ebappendix-splitted ebfont-splitted ebinfo-splitted \
	ebrefile-splitted ebstopcode-splitted ebzip-splitted

eb.html: eb-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p eb -w 2 eb-cat.html

ebappendix.html: ebappendix-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p ebappendix -w 2 ebappendix-cat.html

ebfont.html: ebfont-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p ebfont -w 2 ebfont-cat.html

ebinfo.html: ebinfo-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p ebinfo -w 2 ebinfo-cat.html

ebrefile.html: ebrefile-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p ebrefile -w 2 ebrefile-cat.html

ebstopcode.html: ebstopcode-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p ebstopcode -w 2 ebstopcode-cat.html

ebzip.html: ebzip-cat.html
	rm -f $@
	$(PERL) $(srcdir)/html-split -Z -p ebzip -w 2 ebzip-cat.html


eb-cat.html: eb.html.in version.html eb-toc.html eb-sample-appendix.html \
	eb-sample-booklist.html eb-sample-disctype.html eb-sample-font.html \
	eb-sample-initexit.html eb-sample-subbook.html eb-sample-text.html \
	eb-sample-word.html eb-index-type.html eb-index-func.html \
	eb-index-const.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/eb.html.in

ebappendix-cat.html: $(srcdir)/ebappendix.html.in version.html \
ebappendix-toc.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/ebappendix.html.in

ebfont-cat.html: $(srcdir)/ebfont.html.in version.html ebfont-toc.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/ebfont.html.in

ebinfo-cat.html: $(srcdir)/ebinfo.html.in version.html ebinfo-toc.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/ebinfo.html.in

ebrefile-cat.html: $(srcdir)/ebrefile.html.in version.html ebrefile-toc.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/ebrefile.html.in

ebstopcode-cat.html: $(srcdir)/ebstopcode.html.in version.html \
ebstopcode-toc.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/ebstopcode.html.in

ebzip-cat.html: $(srcdir)/ebzip.html.in version.html ebzip-toc.html
	rm -f $@
	$(PERL) $(srcdir)/html-include -o $@ $(srcdir)/ebzip.html.in


eb-toc.html: $(srcdir)/eb.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/eb.html.in > $@

ebappendix-toc.html: $(srcdir)/ebappendix.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/ebappendix.html.in > $@

ebfont-toc.html: $(srcdir)/ebfont.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/ebfont.html.in > $@

ebinfo-toc.html: $(srcdir)/ebinfo.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/ebinfo.html.in > $@

ebrefile-toc.html: $(srcdir)/ebrefile.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/ebrefile.html.in > $@

ebstopcode-toc.html: $(srcdir)/ebstopcode.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/ebstopcode.html.in > $@

ebzip-toc.html: $(srcdir)/ebzip.html.in
	rm -f $@
	$(PERL) $(srcdir)/html-toc -m h2 -M h3 $(srcdir)/ebzip.html.in > $@

eb-sample-appendix.html: $(top_srcdir)/samples/appendix.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/appendix.c > $@

eb-sample-booklist.html: $(top_srcdir)/samples/booklist.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/booklist.c > $@

eb-sample-disctype.html: $(top_srcdir)/samples/disctype.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/disctype.c > $@

eb-sample-font.html: $(top_srcdir)/samples/font.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/font.c > $@

eb-sample-initexit.html: $(top_srcdir)/samples/initexit.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/initexit.c > $@

eb-sample-subbook.html: $(top_srcdir)/samples/subbook.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/subbook.c > $@

eb-sample-text.html: $(top_srcdir)/samples/text.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/text.c > $@

eb-sample-word.html: $(top_srcdir)/samples/word.c
	rm -f $@
	$(PERL) $(srcdir)/c2html $(top_srcdir)/samples/word.c > $@


eb-index-type.html eb-index-func.html eb-index-const.html: \
$(srcdir)/eb.html.in
	$(PERL) $(srcdir)/html-index -h -p eb-index $(srcdir)/eb.html.in
