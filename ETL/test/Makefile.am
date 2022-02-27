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
	clock \
	handle \
	angle \
	hermite \
	stringf \
	pen \
	surface \
	smart_ptr \
	benchmark \
	spline

check_PROGRAMS = \
	handle \
	clock \
	angle \
	hermite \
	stringf \
	pen \
	surface \
	smart_ptr \
	benchmark \
	smach \
	spline

benchmark_SOURCES=benchmark.cpp
smart_ptr_SOURCES=smart_ptr.cpp
surface_SOURCES=surface.cpp
pen_SOURCES=pen.cpp
handle_SOURCES=handle.cpp
angle_SOURCES=angle.cpp
smach_SOURCES=smach.cpp
clock_SOURCES=clock.cpp
hermite_SOURCES=hermite.cpp
spline_SOURCES=spline.cpp
stringf_SOURCES=stringf.cpp

stringf_CXXFLAGS = @GLIBMM_CFLAGS@
stringf_LDADD = @GLIBMM_LIBS@
