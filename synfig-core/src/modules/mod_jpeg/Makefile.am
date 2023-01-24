MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if HAVE_LIBJPEG

module_LTLIBRARIES = libmod_jpeg.la

libmod_jpeg_la_SOURCES = \
	main.cpp \
	trgt_jpeg.cpp \
	trgt_jpeg.h \
	mptr_jpeg.cpp \
	mptr_jpeg.h

libmod_jpeg_la_LDFLAGS = \
	-module \
	@JPEG_LIBS@ \
	-no-undefined \
	-avoid-version

libmod_jpeg_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_jpeg_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_jpeg.nsh \
	unmod_jpeg.nsh
