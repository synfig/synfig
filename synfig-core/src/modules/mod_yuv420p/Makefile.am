MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = \
	libmod_yuv420p.la

libmod_yuv420p_la_SOURCES = \
        main.cpp \
	trgt_yuv.cpp \
	trgt_yuv.h

libmod_yuv420p_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_yuv420p_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_yuv420p_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@


EXTRA_DIST= mod_yuv420p.nsh unmod_yuv420p.nsh
