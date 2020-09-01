# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if HAVE_LIBMAGICKPP

module_LTLIBRARIES = libmod_magickpp.la

libmod_magickpp_la_SOURCES = \
	main.cpp \
	trgt_magickpp.cpp \
	trgt_magickpp.h

libmod_magickpp_la_LDFLAGS = \
	-module \
	@MAGICKPP_LIBS@ \
	-no-undefined \
	-avoid-version

libmod_magickpp_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	@MAGICKPP_CFLAGS@

libmod_magickpp_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_magickpp.nsh \
	unmod_magickpp.nsh
