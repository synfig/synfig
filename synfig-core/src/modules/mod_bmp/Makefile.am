MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmod_bmp.la

libmod_bmp_la_SOURCES = \
	main.cpp \
	trgt_bmp.cpp \
	trgt_bmp.h \
	mptr_bmp.cpp \
	mptr_bmp.h

libmod_bmp_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_bmp_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_bmp_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@


EXTRA_DIST = \
	mod_bmp.nsh \
	unmod_bmp.nsh
