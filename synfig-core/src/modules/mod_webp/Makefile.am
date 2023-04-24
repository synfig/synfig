MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

if WITH_WEBP

module_LTLIBRARIES = libmod_webp.la

libmod_webp_la_SOURCES =  \
	main.cpp          \
	mptr_webp.cpp     \
	mptr_webp.h       \
	trgt_webp.cpp     \
	trgt_webp.h

libmod_webp_la_LDFLAGS =  \
	-module           \
	-no-undefined     \
	-avoid-version

libmod_webp_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_webp_la_LIBADD =           \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

endif


EXTRA_DIST = \
	mod_webp.nsh \
	unmod_webp.nsh
