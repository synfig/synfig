MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @ETL_CFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src
AM_LDFLAGS = \
	../src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS=bone bline

bone_SOURCES=bone.cpp

bline_SOURCES=bline.cpp

