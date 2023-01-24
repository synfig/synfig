MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmod_gif.la

libmod_gif_la_SOURCES = \
	main.cpp \
	trgt_gif.cpp \
	trgt_gif.h

libmod_gif_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_gif_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_gif_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@


EXTRA_DIST = \
	mod_gif.nsh \
	unmod_gif.nsh
