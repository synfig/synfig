# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src


moduledir = @MODULE_DIR@

module_LTLIBRARIES = liblyr_std.la

liblyr_std_la_SOURCES = \
	main.cpp \
	timeloop.cpp \
	timeloop.h \
	perspective.cpp \
	perspective.h \
	xorpattern.cpp \
	booleancurve.h \
	booleancurve.cpp \
	bevel.cpp \
	bevel.h \
	shade.cpp \
	shade.h \
	twirl.cpp \
	twirl.h \
	stretch.cpp \
	stretch.h \
	xorpattern.h \
	clamp.cpp \
	clamp.h \
	supersample.cpp \
	supersample.h \
	insideout.cpp \
	insideout.h \
	julia.cpp \
	julia.h \
	rotate.cpp \
	rotate.h \
	mandelbrot.cpp \
	mandelbrot.h \
	zoom.h \
	zoom.cpp \
	import.cpp \
	import.h \
	translate.h \
	translate.cpp \
	sphere_distort.h \
	sphere_distort.cpp \
	curvewarp.cpp \
	curvewarp.h \
	stroboscope.cpp \
	stroboscope.h \
	freetime.cpp \
	freetime.h

liblyr_std_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

liblyr_std_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

liblyr_std_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version


EXTRA_DIST = \
	lyr_std.nsh \
	unlyr_std.nsh
