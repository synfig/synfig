# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

INCLUDES = \
	-I$(top_builddir) -I$(top_srcdir)/src @INCLTDL@

EXTRA_DIST = \
	surfacenew.cpp pch.h synfig.nsh

LAYERHEADERS = \
	layer_bitmap.h	 layer_composite.h   layer_duplicate.h	 layer_mime.h	layer_motionblur.h			\
	layer_pastecanvas.h   layer_polygon.h	layer_shape.h	layer_solidcolor.h
LAYERSOURCES = \
	layer_bitmap.cpp layer_composite.cpp layer_duplicate.cpp layer_mime.cpp layer_motionblur.cpp			\
	layer_pastecanvas.cpp layer_polygon.cpp layer_shape.cpp layer_solidcolor.cpp

TARGETHEADERS = \
	target_multi.h	 target_null.h	 target_null_tile.h   target_scanline.h	  target_tile.h
TARGETSOURCES = \
	target_multi.cpp target_null.cpp target_null_tile.cpp target_scanline.cpp target_tile.cpp

IMPORTERHEADERS = \
	listimporter.h
IMPORTERSOURCES = \
	listimporter.cpp

VALUENODEHEADERS = \
	valuenode_add.h	  valuenode_animated.h	 valuenode_atan2.h   valuenode_bline.h	 valuenode_blinecalctangent.h	\
	valuenode_blinecalcvertex.h   valuenode_blinereversetangent.h	valuenode_composite.h	valuenode_const.h	\
	valuenode_cos.h	  valuenode_duplicate.h	  valuenode_dynamiclist.h   valuenode_exp.h				\
	valuenode_gradientrotate.h   valuenode_integer.h   valuenode_linear.h valuenode_radialcomposite.h		\
	valuenode_range.h   valuenode_reciprocal.h   valuenode_reference.h   valuenode_repeat_gradient.h		\
	valuenode_scale.h   valuenode_segcalctangent.h	 valuenode_segcalcvertex.h   valuenode_sine.h			\
	valuenode_stripes.h   valuenode_subtract.h    valuenode_switch.h   valuenode_timedswap.h			\
	valuenode_timeloop.h   valuenode_twotone.h
VALUENODESOURCES = \
	valuenode_add.cpp valuenode_animated.cpp valuenode_atan2.cpp valuenode_bline.cpp valuenode_blinecalctangent.cpp	\
	valuenode_blinecalcvertex.cpp valuenode_blinereversetangent.cpp valuenode_composite.cpp valuenode_const.cpp	\
	valuenode_cos.cpp valuenode_duplicate.cpp valuenode_dynamiclist.cpp valuenode_exp.cpp				\
	valuenode_gradientrotate.cpp valuenode_integer.cpp valuenode_linear.cpp valuenode_radialcomposite.cpp		\
	valuenode_range.cpp valuenode_reciprocal.cpp valuenode_reference.cpp valuenode_repeat_gradient.cpp		\
	valuenode_scale.cpp valuenode_segcalctangent.cpp valuenode_segcalcvertex.cpp valuenode_sine.cpp			\
	valuenode_stripes.cpp valuenode_subtract.cpp valuenode_switch.cpp valuenode_timedswap.cpp			\
	valuenode_timeloop.cpp valuenode_twotone.cpp

VALUEHEADERS = \
	blinepoint.h   gradient.h   value.h
VALUESOURCES = \
	blinepoint.cpp gradient.cpp value.cpp

SYNFIGHEADERS = \
	angle.h canvasbase.h general.h guidset.h interpolation.h nodebase.h protocol.h quick_rng.h real.h releases.h	\
	segment.h smartfile.h string.h string_decl.h surfacenew.h synfig.h types.h vector.h version.h			\
	activepoint.h	blur.h	 canvas.h   color.h   context.h	  curve_helper.h   curveset.h	distance.h		\
	exception.h   gamma.h	guid.h	 importer.h   keyframe.h   layer.h   loadcanvas.h   main.h   module.h		\
	mutex.h	  node.h   palette.h   paramdesc.h   polynomial_root.h	 rect.h	  renddesc.h   render.h			\
	savecanvas.h   surface.h   target.h   time.h   timepointcollect.h   transform.h	  uniqueid.h   valuenode.h	\
	waypoint.h
SYNFIGSOURCES = \
	activepoint.cpp blur.cpp canvas.cpp color.cpp context.cpp curve_helper.cpp curveset.cpp distance.cpp		\
	exception.cpp gamma.cpp guid.cpp importer.cpp keyframe.cpp layer.cpp loadcanvas.cpp main.cpp module.cpp		\
	mutex.cpp node.cpp palette.cpp paramdesc.cpp polynomial_root.cpp rect.cpp renddesc.cpp render.cpp		\
	savecanvas.cpp surface.cpp target.cpp time.cpp timepointcollect.cpp transform.cpp uniqueid.cpp valuenode.cpp	\
	waypoint.cpp

lib_LTLIBRARIES = libsynfig.la
libsynfig_la_SOURCES = \
	$(VALUEHEADERS) $(VALUESOURCES) $(LAYERSOURCES) $(LAYERHEADERS) $(TARGETHEADERS) $(TARGETSOURCES)		\
	$(VALUENODEHEADERS) $(VALUENODESOURCES) $(SYNFIGHEADERS) $(SYNFIGSOURCES) $(IMPORTERHEADERS) $(IMPORTERSOURCES)
libsynfig_la_LIBADD = @LIBLTDL@ @SYNFIG_LIBS@ @LIBADD_DL@
libsynfig_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
libsynfig_la_LDFLAGS = -export-dynamic -no-undefined -version-info 0:0:0

include_synfigdir=@synfigincludedir@/synfig
include_synfig_HEADERS = \
	$(SYNFIGHEADERS) $(LAYERHEADERS) $(TARGETHEADERS) $(VALUENODEHEADERS) $(IMPORTERHEADERS) $(VALUEHEADERS)
