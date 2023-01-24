MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if WITH_LIBAVCODEC

module_LTLIBRARIES = libmod_libavcodec.la

libmod_libavcodec_la_SOURCES = \
	main.cpp \
	trgt_av.cpp \
	trgt_av.h \
	mptr.cpp \
	mptr.h

libmod_libavcodec_la_LDFLAGS = \
	-module -no-undefined -avoid-version

libmod_libavcodec_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	@LIBAVCODEC_CFLAGS@ \
	-D__STDC_CONSTANT_MACROS

libmod_libavcodec_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@ \
	@LIBAVCODEC_LIBS@

endif


EXTRA_DIST = \
	mod_libavcodec.nsh \
	unmod_libavcodec.nsh
