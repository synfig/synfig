MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src
AM_LDFLAGS = \
	../src/synfigapp/libsynfigapp.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS=app_layerduplicate smach

app_layerduplicate_SOURCES=app_layerduplicate.cpp

smach_SOURCES=smach.cpp

