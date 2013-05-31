# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CXXFLAGS = \
	@CXXFLAGS@ \
	-I$(top_srcdir) \
	-I$(top_builddir)/ETL

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir)/ETL

TESTS = \
	fixed \
	clock \
	handle \
	angle \
	random \
	hermite \
	stringf \
	pen \
	surface \
	smart_ptr \
	benchmark \
	spline \
	value

check_PROGRAMS = \
	fixed \
	handle \
	clock \
	angle \
	random \
	hermite \
	stringf \
	pen \
	surface \
	smart_ptr \
	benchmark \
	smach \
	spline \
	value

benchmark_SOURCES=benchmark.cpp
smart_ptr_SOURCES=smart_ptr.cpp
surface_SOURCES=surface.cpp
pen_SOURCES=pen.cpp
handle_SOURCES=handle.cpp
angle_SOURCES=angle.cpp
random_SOURCES=random.cpp
fixed_SOURCES=fixed.cpp
smach_SOURCES=smach.cpp
clock_SOURCES=clock.cpp
hermite_SOURCES=hermite.cpp
spline_SOURCES=spline.cpp
stringf_SOURCES=stringf.cpp
value_SOURCES=value.cpp

# FIXME: fix compiler errors for the spline and value tests.
EXTRA_DIST = \
	spline.cpp \
	value.cpp
