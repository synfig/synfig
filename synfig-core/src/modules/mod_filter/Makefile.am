MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmod_filter.la

libmod_filter_la_SOURCES = \
	blur.cpp \
	blur.h \
	colorcorrect.cpp \
	colorcorrect.h \
	halftone2.cpp \
	halftone2.h \
	lumakey.cpp \
	lumakey.h \
	radialblur.cpp \
	radialblur.h \
	main.cpp \
	halftone.cpp \
	halftone.h \
	halftone3.cpp \
	halftone3.h

libmod_filter_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_filter_la_LIBADD = \
	../../synfig/libsynfig.la @SYNFIG_LIBS@

libmod_filter_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version


EXTRA_DIST = \
	mod_filter.nsh \
	unmod_filter.nsh
