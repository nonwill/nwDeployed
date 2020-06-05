AUTOMAKE_OPTIONS = foreign

LIBEB = ../eb/libeb.la

noinst_PROGRAMS = initexit disctype subbook word text font appendix booklist

initexit_SOURCES = initexit.c
initexit_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
initexit_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

disctype_SOURCES = disctype.c
disctype_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
disctype_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

subbook_SOURCES = subbook.c
subbook_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
subbook_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

word_SOURCES = word.c
word_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
word_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

text_SOURCES = text.c
text_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
text_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

font_SOURCES = font.c
font_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
font_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

appendix_SOURCES = appendix.c
appendix_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
appendix_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

booklist_SOURCES = booklist.c
booklist_LDADD = $(LIBEB) $(ZLIBLIBS) $(INTLLIBS)
booklist_DEPENDENCIES = $(LIBEB) $(ZLIBDEPS) $(INTLDEPS)

EXTRA_DIST = README

INCLUDES = -I$(srcdir)/..
