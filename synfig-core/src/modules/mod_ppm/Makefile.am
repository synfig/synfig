MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmod_ppm.la

libmod_ppm_la_SOURCES = \
	main.cpp \
	trgt_ppm.cpp \
	trgt_ppm.h \
	mptr_ppm.cpp \
	mptr_ppm.h

libmod_ppm_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version

libmod_ppm_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_ppm_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@


EXTRA_DIST= mod_ppm.nsh unmod_ppm.nsh
