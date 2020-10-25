MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if HAVE_LIBPNG

module_LTLIBRARIES = libmod_png.la

libmod_png_la_SOURCES = \
	main.cpp \
	trgt_png.cpp \
	trgt_png.h \
	trgt_png_spritesheet.cpp \
	trgt_png_spritesheet.h \
	mptr_png.cpp \
	mptr_png.h

libmod_png_la_LDFLAGS = \
	-module \
	@PNG_LIBS@ \
	-no-undefined \
	-avoid-version

libmod_png_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_png_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_png.nsh \
	unmod_png.nsh
