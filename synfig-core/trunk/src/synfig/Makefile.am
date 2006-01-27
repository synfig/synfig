# $Header: /opt/voria/cvs/synfig/src/synfig/Makefile.am,v 1.3 2005/01/21 19:29:10 darco Exp $

# SUBDIRS=proto

MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/libltdl

EXTRA_DIST=proto/Makefile proto/nodebase.h proto/nodebase.px proto/proto.m4 synfig.nsh

LAYERHEADERS=layer_motionblur.h layer_pastecanvas.h layer_solidcolor.h layer_polygon.h layer_composite.h layer_bitmap.h layer_mime.h layer_shape.h
LAYERSOURCES=layer_motionblur.cpp layer_pastecanvas.cpp layer_solidcolor.cpp layer_polygon.cpp layer_composite.cpp layer_bitmap.cpp layer_mime.cpp layer_shape.cpp

TARGETHEADERS=target_scanline.h target_tile.h target_multi.h target_null.h target_null_tile.h
TARGETSOURCES=target_scanline.cpp target_tile.cpp target_multi.cpp target_null.cpp target_null_tile.cpp

IMPORTERHEADERS=listimporter.h
IMPORTERSOURCES=listimporter.cpp

VALUENODEHEADERS=valuenode_sine.h valuenode_radialcomposite.h valuenode_bline.h valuenode_segcalcvertex.h valuenode_segcalctangent.h valuenode_twotone.h valuenode_stripes.h valuenode_subtract.h valuenode_const.h valuenode_reference.h valuenode_linear.h valuenode_composite.h valuenode_dynamiclist.h valuenode_animated.h valuenode_scale.h valuenode_timedswap.h valuenode_gradientrotate.h
VALUENODESOURCES=valuenode_sine.cpp valuenode_radialcomposite.cpp valuenode_bline.cpp valuenode_segcalcvertex.cpp valuenode_segcalctangent.cpp valuenode_twotone.cpp valuenode_stripes.cpp valuenode_subtract.cpp valuenode_const.cpp valuenode_reference.cpp valuenode_linear.cpp valuenode_composite.cpp valuenode_dynamiclist.cpp valuenode_animated.cpp valuenode_scale.cpp valuenode_timedswap.cpp valuenode_gradientrotate.cpp

VALUEHEADERS=blinepoint.h gradient.h value.h
VALUESOURCES=blinepoint.cpp gradient.cpp value.cpp

SYNFIGHEADERS=protocol.h surfacenew.h mutex.h timepointcollect.h interpolation.h guidset.h guid.h rect.h node.h smartfile.h distance.h palette.h main.h waypoint.h activepoint.h gamma.h uniqueid.h canvasbase.h context.h real.h paramdesc.h string_decl.h angle.h keyframe.h synfig.h renddesc.h general.h importer.h surface.h module.h layer.h vector.h color.h canvas.h render.h target.h loadcanvas.h savecanvas.h valuenode.h version.h segment.h types.h exception.h string.h time.h blur.h transform.h curve_helper.h polynomial_root.h curveset.h
SYNFIGSOURCES=mutex.cpp timepointcollect.cpp rect.cpp node.cpp guid.cpp loadcanvas.cpp distance.cpp palette.cpp paramdesc.cpp waypoint.cpp activepoint.cpp gamma.cpp uniqueid.cpp context.cpp renddesc.cpp time.cpp exception.cpp keyframe.cpp main.cpp surface.cpp module.cpp importer.cpp layer.cpp color.cpp canvas.cpp render.cpp target.cpp savecanvas.cpp valuenode.cpp blur.cpp curve_helper.cpp polynomial_root.cpp transform.cpp curveset.cpp

lib_LTLIBRARIES = libsynfig.la
libsynfig_la_SOURCES = $(VALUEHEADERS) $(VALUESOURCES) $(LAYERSOURCES) $(LAYERHEADERS) $(TARGETHEADERS) $(TARGETSOURCES) $(VALUENODEHEADERS) $(VALUENODESOURCES) $(SYNFIGHEADERS) $(SYNFIGSOURCES) $(IMPORTERHEADERS) $(IMPORTERSOURCES)
libsynfig_la_LIBADD = @LIBLTDL@ @SYNFIG_LIBS@ @LIBADD_DL@
libsynfig_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
libsynfig_la_LDFLAGS = -export-dynamic -no-undefined -version-info 0:0:0

include_synfigdir=@synfigincludedir@/synfig
include_synfig_HEADERS = $(SYNFIGHEADERS) $(LAYERHEADERS) $(TARGETHEADERS) $(VALUENODEHEADERS) $(IMPORTERHEADERS) $(VALUEHEADERS)
