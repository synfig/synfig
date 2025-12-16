MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/modules/mod_geometry

AM_CXXFLAGS = \
	@CXXFLAGS@ @ETL_CFLAGS@ @SYNFIG_CFLAGS@

check_PROGRAMS=$(TESTS)

TESTS = \
	testadvancedline

testadvancedline_SOURCES=advancedline.cpp

testadvancedline_LDADD = \
	../libmod_geometry.la \
	$(top_builddir)/src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

