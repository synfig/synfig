MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if WITH_LIBDV

module_LTLIBRARIES = libmod_dv.la

libmod_dv_la_SOURCES = \
	main.cpp \
	trgt_dv.cpp \
	trgt_dv.h

libmod_dv_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_dv_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_dv_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_dv.nsh \
	unmod_dv.nsh
