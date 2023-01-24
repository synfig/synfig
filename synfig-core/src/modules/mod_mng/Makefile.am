MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if HAVE_LIBMNG

module_LTLIBRARIES = \
	libmod_mng.la

libmod_mng_la_SOURCES = \
	main.cpp \
	trgt_mng.cpp \
	trgt_mng.h

libmod_mng_la_LDFLAGS = \
	-module \
	@MNG_LIBS@ \
	-no-undefined \
	-avoid-version

libmod_mng_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_mng_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_mng.nsh \
	unmod_mng.nsh
