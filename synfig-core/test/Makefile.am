MAINTAINERCLEANFILES=Makefile.in
AM_CXXFLAGS=@CXXFLAGS@ @SYNFIG_CFLAGS@ -I$(top_builddir) -I$(top_srcdir)/src -I$(top_srcdir)/test
AM_LDFLAGS = \
	$(top_builddir)/src/synfig/libsynfig.la \
	@SYNFIG_LIBS@

check_PROGRAMS=$(TESTS)

TESTS = \
	test_synfig_angle \
	test_synfig_benchmark \
	test_synfig_bezier \
	test_synfig_bline \
	test_synfig_bone \
	test_synfig_clock \
	test_synfig_filesystem_path \
	test_synfig_handle \
	test_synfig_keyframe \
	test_synfig_node \
	test_synfig_pen \
	test_synfig_reference_counter \
	test_synfig_string \
	test_synfig_surface_etl \
	test_synfig_valuenode_maprange

test_synfig_angle_SOURCES=angle.cpp

test_synfig_benchmark_SOURCES=benchmark.cpp

test_synfig_bezier_SOURCES=hermite.cpp

test_synfig_bone_SOURCES=bone.cpp

test_synfig_bline_SOURCES=bline.cpp

test_synfig_clock_SOURCES=clock.cpp

test_synfig_filesystem_path_SOURCES=filesystem_path.cpp

test_synfig_handle_SOURCES=handle.cpp

test_synfig_keyframe_SOURCES=keyframe.cpp

test_synfig_node_SOURCES=node.cpp

test_synfig_pen_SOURCES=pen.cpp

test_synfig_reference_counter_SOURCES=reference_counter.cpp

test_synfig_string_SOURCES=string.cpp

test_synfig_surface_etl_SOURCES=surface_etl.cpp

test_synfig_valuenode_maprange_SOURCES=valuenode_maprange.cpp

EXTRA_DIST = test_base.h
