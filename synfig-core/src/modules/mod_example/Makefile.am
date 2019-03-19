# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmod_example.la

libmod_example_la_SOURCES = \
	main.cpp \
	simplecircle.cpp \
	simplecircle.h \
	metaballs.h \
	metaballs.cpp

libmod_example_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

libmod_example_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_example_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

EXTRA_DIST = \
	mod_example.nsh \
	unmod_example.nsh
