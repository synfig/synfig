# $Header: /opt/voria/cvs/synfig/src/modules/mod_filter/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_filter.la
libmod_filter_la_SOURCES = blur.cpp blur.h colorcorrect.cpp colorcorrect.h halftone2.cpp halftone2.h lumakey.cpp lumakey.h radialblur.cpp radialblur.h main.cpp halftone.cpp halftone.h halftone3.cpp halftone3.h
libmod_filter_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_filter_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
libmod_filter_la_LDFLAGS = -module -no-undefined
