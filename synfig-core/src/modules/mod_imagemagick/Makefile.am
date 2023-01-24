MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if WITH_IMAGEMAGICK

module_LTLIBRARIES = libmod_imagemagick.la

libmod_imagemagick_la_SOURCES = \
	main.cpp \
	mptr_imagemagick.cpp \
	mptr_imagemagick.h \
	trgt_imagemagick.cpp \
	trgt_imagemagick.h

libmod_imagemagick_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_imagemagick_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_imagemagick_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_imagemagick.nsh \
	unmod_imagemagick.nsh
