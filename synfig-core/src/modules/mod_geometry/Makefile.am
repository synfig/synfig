MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src

# GCC on MacOS X does not take kindly to the -ffastmath optimization
# in this module for some reason.
AM_CXXFLAGS = \
	`echo @CXXFLAGS@ | sed s/-ffast-math//g`


moduledir = @MODULE_DIR@

module_LTLIBRARIES = \
	libmod_geometry.la

libmod_geometry_la_SOURCES = \
	rectangle.cpp \
	rectangle.h \
	star.h \
	star.cpp \
	checkerboard.cpp \
	checkerboard.h \
	circle.cpp \
	circle.h \
	region.h \
	region.cpp \
	outline.h \
	outline.cpp \
	advanced_outline.h \
	advanced_outline.cpp \
	main.cpp

libmod_geometry_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@

libmod_geometry_la_LIBADD = \
	../../synfig/libsynfig.la \
	@SYNFIG_LIBS@

libmod_geometry_la_LDFLAGS = \
	-module \
	-no-undefined \
	-avoid-version


EXTRA_DIST = \
	mod_geometry.nsh \
	unmod_geometry.nsh
