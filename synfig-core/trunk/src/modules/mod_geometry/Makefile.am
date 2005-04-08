# $Header: /opt/voria/cvs/sinfg/src/modules/mod_geometry/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

# GCC on MacOS X does not take kindly to the -ffastmath optimization
# in this module for some reason.
CXXFLAGS=`echo @CXXFLAGS@ | sed s/-ffast-math//g`

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_geometry.la
libmod_geometry_la_SOURCES = rectangle.cpp rectangle.h star.h star.cpp checkerboard.cpp checkerboard.h circle.cpp circle.h region.h region.cpp outline.h outline.cpp main.cpp 
libmod_geometry_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_geometry_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
libmod_geometry_la_LDFLAGS = -module -no-undefined
