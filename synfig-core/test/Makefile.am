MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @ETL_CFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src
AM_LDFLAGS = \
	../src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS = \
	angle \
	benchmark \
	bezier \
	bline \
	bone \
	clock \
	keyframe \
	node \
	pen \
	string \
	surface_etl

angle_SOURCES=angle.cpp

benchmark_SOURCES=benchmark.cpp

bezier_SOURCES=hermite.cpp

bone_SOURCES=bone.cpp

bline_SOURCES=bline.cpp

clock_SOURCES=clock.cpp

keyframe_SOURCES=keyframe.cpp

node_SOURCES=node.cpp

pen_SOURCES=pen.cpp

string_SOURCES=string.cpp

surface_etl_SOURCES=surface_etl.cpp

