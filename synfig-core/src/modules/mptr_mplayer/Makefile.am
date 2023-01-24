MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmptr_mplayer.la

libmptr_mplayer_la_SOURCES = \
	main.cpp \
	mptr_mplayer.cpp \
	mptr_mplayer.h

libmptr_mplayer_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmptr_mplayer_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@



