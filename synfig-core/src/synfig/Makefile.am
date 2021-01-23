# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src \
	@INCLTDL@

EXTRA_DIST = \
	pch.h \
	synfig.nsh \
	synfig.h \
	nodebase.h


TARGETHEADERS = \
	target_multi.h \
	target_null.h \
	target_null_tile.h \
	target_scanline.h \
	target_cairo.h \
	target_tile.h \
	target_cairo_tile.h \
	targetparam.h

TARGETSOURCES = \
	target_multi.cpp \
	target_null.cpp \
	target_null_tile.cpp \
	target_scanline.cpp \
	target_cairo.cpp \
	target_tile.cpp \
	target_cairo_tile.cpp


IMPORTERHEADERS = \
	listimporter.h \
	cairolistimporter.h

IMPORTERSOURCES = \
	listimporter.cpp \
	cairolistimporter.cpp


VALUEHEADERS = \
	blinepoint.h \
	widthpoint.h \
	dashitem.h \
	gradient.h \
	weightedvalue.h \
	pair.h \
	type.h \
	base_types.h \
	value.h

VALUESOURCES = \
	blinepoint.cpp \
	widthpoint.cpp \
	dashitem.cpp \
	gradient.cpp \
	type.cpp \
	base_types.cpp \
	value.cpp


SYNFIGHEADERS = \
	angle.h \
	canvasbase.h \
	general.h \
	localization.h \
	progresscallback.h \
	guidset.h \
	interpolation.h \
	protocol.h \
	quick_rng.h \
	real.h \
	complex.h \
	releases.h \
	segment.h \
	smartfile.h \
	string.h \
	types.h \
	vector.h \
	transformation.h \
	version.h \
	boneweightpair.h \
	activepoint.h \
	blur.h \
	bone.h \
	cairo_operators.h \
	cairo_renddesc.h \
	canvas.h \
	color.h \
	context.h \
	curve.h \
	curve_helper.h \
	curveset.h \
	distance.h \
	exception.h \
	guid.h \
	importer.h \
	cairoimporter.h \
	keyframe.h \
	layer.h \
	loadcanvas.h \
	main.h \
	module.h \
	node.h \
	palette.h \
	paramdesc.h \
	polynomial_root.h \
	rect.h \
	renddesc.h \
	render.h \
	savecanvas.h \
	surface.h \
	target.h \
	time.h \
	timepointcollect.h \
	transform.h \
	uniqueid.h \
	valuenode.h \
	valuenode_registry.h \
	waypoint.h \
	matrix.h \
	filesystem.h \
	filesystemnative.h \
	filesystemgroup.h \
	filesystemtemporary.h \
	filecontainer.h \
	filecontainerzip.h \
	zstreambuf.h \
	valueoperations.h \
	valuetransformation.h \
	soundprocessor.h \
	polygon.h \
	canvasfilenaming.h \
	token.h \
	threadpool.h

SYNFIGSOURCES = \
	activepoint.cpp \
	bone.cpp \
	blur.cpp \
	cairo_operators.cpp \
	cairo_renddesc.cpp \
	canvas.cpp \
	context.cpp \
	curve.cpp \
	curve_helper.cpp \
	curveset.cpp \
	distance.cpp \
	exception.cpp \
	guid.cpp \
	importer.cpp \
	cairoimporter.cpp \
	keyframe.cpp \
	layer.cpp \
	loadcanvas.cpp \
	main.cpp \
	module.cpp \
	node.cpp \
	palette.cpp \
	paramdesc.cpp \
	polynomial_root.cpp \
	rect.cpp \
	renddesc.cpp \
	render.cpp \
	savecanvas.cpp \
	surface.cpp \
	target.cpp \
	time.cpp \
	timepointcollect.cpp \
	transform.cpp \
	uniqueid.cpp \
	valuenode.cpp \
	valuenode_registry.cpp \
	waypoint.cpp \
	matrix.cpp \
	filesystem.cpp \
	filesystemnative.cpp \
	filesystemgroup.cpp \
	filesystemtemporary.cpp \
	filecontainer.cpp \
	filecontainerzip.cpp \
	zstreambuf.cpp \
	valueoperations.cpp \
	soundprocessor.cpp \
	canvasfilenaming.cpp \
	token.cpp \
	threadpool.cpp


libsynfig_src = \
	$(VALUEHEADERS) \
	$(VALUESOURCES) \
	$(TARGETHEADERS) \
	$(TARGETSOURCES) \
	$(SYNFIGHEADERS) \
	$(SYNFIGSOURCES) \
	$(IMPORTERHEADERS) \
	$(IMPORTERSOURCES)

libsynfig_include_HH = \
	$(SYNFIGHEADERS) \
	$(TARGETHEADERS) \
	$(IMPORTERHEADERS) \
	$(VALUEHEADERS)

include color/Makefile_insert
include debug/Makefile_insert
include layers/Makefile_insert
include rendering/Makefile_insert
include valuenodes/Makefile_insert

lib_LTLIBRARIES = libsynfig.la

libsynfig_la_SOURCES = \
	$(libsynfig_src)

nobase_include_synfig_HEADERS = \
	$(libsynfig_include_HH)


libsynfig_la_LIBADD = \
	@LIBLTDL@ \
	@SYNFIG_LIBS@ \
	@LIBADD_DL@

libsynfig_la_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	-DLIBDIR="\"@libdir@\"" \
	-DSYSCONFDIR="\"@sysconfdir@\""

libsynfig_la_LDFLAGS = \
	-export-dynamic \
	-no-undefined \
	-version-info 0:0:0


include_synfigdir = @synfigincludedir@/synfig

