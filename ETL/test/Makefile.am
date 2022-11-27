MAINTAINERCLEANFILES = \
	Makefile.in

AM_CXXFLAGS = \
	@CXXFLAGS@ \
	-I$(top_srcdir) \
	-I$(top_builddir)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir)

TESTS = \
	handle \
	stringf

check_PROGRAMS = \
	handle \
	stringf

handle_SOURCES=handle.cpp
stringf_SOURCES=stringf.cpp

stringf_CXXFLAGS = @GLIBMM_CFLAGS@
stringf_LDADD = @GLIBMM_LIBS@
