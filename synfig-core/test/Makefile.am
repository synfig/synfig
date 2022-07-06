MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @ETL_CFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src
AM_LDFLAGS = \
	../src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS = \
	angle \
	benchmark \
	bline \
	bone \
	clock \
	node

angle_SOURCES=angle.cpp

benchmark_SOURCES=benchmark.cpp

bone_SOURCES=bone.cpp

bline_SOURCES=bline.cpp

clock_SOURCES=clock.cpp

node_SOURCES=node.cpp
