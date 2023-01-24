MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = libmod_gradient.la

libmod_gradient_la_SOURCES = \
	curvegradient.cpp \
	curvegradient.h \
	lineargradient.cpp \
	lineargradient.h \
	conicalgradient.cpp \
	conicalgradient.h \
	spiralgradient.cpp \
	spiralgradient.h \
	radialgradient.cpp \
	radialgradient.h \
	main.cpp

libmod_gradient_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_gradient_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

libmod_gradient_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version


EXTRA_DIST= mod_gradient.nsh unmod_gradient.nsh
