# $Id$

MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @ETL_CFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src
AM_LDFLAGS = \
	../src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS= \
    bone \
    bline \
    fixed \
    clock \
    handle \
    angle \
    hermite \
    stringf \
    pen \
    surface \
    smart_ptr \
    benchmark


angle_SOURCES=angle.cpp
benchmark_SOURCES=benchmark.cpp
bline_SOURCES=bline.cpp
bone_SOURCES=bone.cpp
clock_SOURCES=clock.cpp
fixed_SOURCES=fixed.cpp
handle_SOURCES=handle.cpp
hermite_SOURCES=hermite.cpp
pen_SOURCES=pen.cpp
smart_ptr_SOURCES=smart_ptr.cpp
stringf_SOURCES=stringf.cpp
surface_SOURCES=surface.cpp


