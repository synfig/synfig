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


LAYERHEADERS = \
	layer_bitmap.h \
	layer_composite.h \
	layer_duplicate.h \
	layer_group.h \
	layer_meshtransform.h \
	layer_mime.h \
	layer_motionblur.h \
	layer_pastecanvas.h \
	layer_polygon.h \
	layer_shape.h \
	layer_solidcolor.h \
	layer_sound.h \
	layer_skeleton.h \
	layer_skeletondeformation.h \
	layer_switch.h

LAYERSOURCES = \
	layer_bitmap.cpp \
	layer_composite.cpp \
	layer_duplicate.cpp \
	layer_group.cpp \
	layer_meshtransform.cpp \
	layer_mime.cpp \
	layer_motionblur.cpp \
	layer_pastecanvas.cpp \
	layer_polygon.cpp \
	layer_shape.cpp \
	layer_solidcolor.cpp \
	layer_sound.cpp \
	layer_skeleton.cpp \
	layer_skeletondeformation.cpp \
	layer_switch.cpp


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
	guidset.h \
	interpolation.h \
	protocol.h \
	quick_rng.h \
	real.h \
	releases.h \
	segment.h \
	smartfile.h \
	string.h \
	string_decl.h \
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
	curve_helper.h \
	curveset.h \
	distance.h \
	exception.h \
	gamma.h \
	guid.h \
	importer.h \
	cairoimporter.h \
	keyframe.h \
	layer.h \
	loadcanvas.h \
	main.h \
	module.h \
	mutex.h \
	node.h \
	palette.h \
	paramdesc.h \
	polynomial_root.h \
	rect.h \
	renddesc.h \
	render.h \
	rendermethod.h \
	savecanvas.h \
	surface.h \
	target.h \
	time.h \
	timepointcollect.h \
	transform.h \
	uniqueid.h \
	valuenode.h \
	waypoint.h \
	matrix.h \
	filesystem.h \
	filesystemnative.h \
	filesystemgroup.h \
	filecontainer.h \
	filecontainerzip.h \
	filecontainertemporary.h \
	zstreambuf.h \
	valueoperations.h \
	valuetransformation.h \
	mesh.h \
	renderer.h \
	renderersoftware.h \
	soundprocessor.h \
	polygon.h

SYNFIGSOURCES = \
	activepoint.cpp \
	bone.cpp \
	blur.cpp \
	cairo_operators.cpp \
	cairo_renddesc.cpp \
	canvas.cpp \
	color.cpp \
	context.cpp \
	curve_helper.cpp \
	curveset.cpp \
	distance.cpp \
	exception.cpp \
	gamma.cpp \
	guid.cpp \
	importer.cpp \
	cairoimporter.cpp \
	keyframe.cpp \
	layer.cpp \
	loadcanvas.cpp \
	main.cpp \
	module.cpp \
	mutex.cpp \
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
	waypoint.cpp \
	matrix.cpp \
	filesystem.cpp \
	filesystemnative.cpp \
	filesystemgroup.cpp \
	filecontainer.cpp \
	filecontainerzip.cpp \
	filecontainertemporary.cpp \
	zstreambuf.cpp \
	valueoperations.cpp \
	mesh.cpp \
	renderer.cpp \
	renderersoftware.cpp \
	soundprocessor.cpp


libsynfig_src = \
	$(VALUEHEADERS) \
	$(VALUESOURCES) \
	$(LAYERSOURCES) \
	$(LAYERHEADERS) \
	$(TARGETHEADERS) \
	$(TARGETSOURCES) \
	$(SYNFIGHEADERS) \
	$(SYNFIGSOURCES) \
	$(IMPORTERHEADERS) \
	$(IMPORTERSOURCES)

libsynfig_include_HH = \
	$(SYNFIGHEADERS) \
	$(LAYERHEADERS) \
	$(TARGETHEADERS) \
	$(IMPORTERHEADERS) \
	$(VALUEHEADERS)

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

