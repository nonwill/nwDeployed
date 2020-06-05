## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign 1.6 dist-zip

SUBDIRS = src include doc

m4datadir = $(datadir)/aclocal
m4data_DATA = ao.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ao.pc

ckportdir = $(libdir)/ckport/db
ckport_DATA = libao.ckport

man_MANS = libao.conf.5

EXTRA_DIST = README AUTHORS CHANGES COPYING libao.spec ao.m4 acinclude.m4 as-ac-expand.m4 ao.pc.in $(ckport_DATA) $(man_MANS)

debug:
	$(MAKE) clean
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) clean
	$(MAKE) all CFLAGS="@PROFILE@"
