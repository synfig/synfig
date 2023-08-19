MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @ETL_CFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src -I$(top_srcdir)/test
AM_LDFLAGS = \
	$(top_builddir)/src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS = \
	angle \
	benchmark \
	bezier \
	bline \
	bone \
	clock \
	filesystem_path \
	keyframe \
	node \
	pen \
	reference_counter \
	string \
	surface_etl

angle_SOURCES=angle.cpp

benchmark_SOURCES=benchmark.cpp

bezier_SOURCES=hermite.cpp

bone_SOURCES=bone.cpp

bline_SOURCES=bline.cpp

clock_SOURCES=clock.cpp

filesystem_path_SOURCES=filesystem_path.cpp

keyframe_SOURCES=keyframe.cpp

node_SOURCES=node.cpp

pen_SOURCES=pen.cpp

reference_counter_SOURCES=reference_counter.cpp

string_SOURCES=string.cpp

surface_etl_SOURCES=surface_etl.cpp

EXTRA_DIST = test_base.h
