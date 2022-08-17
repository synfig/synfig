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
	handle \
	hermite \
	stringf \
	pen \
	surface

check_PROGRAMS = \
	handle \
	hermite \
	stringf \
	pen \
	surface

surface_SOURCES=surface.cpp
pen_SOURCES=pen.cpp
handle_SOURCES=handle.cpp
stringf_SOURCES=stringf.cpp

stringf_CXXFLAGS = @GLIBMM_CFLAGS@
stringf_LDADD = @GLIBMM_LIBS@
