# $Header: /opt/voria/cvs/synfig/src/modules/mod_yuv420p/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_yuv420p.la
libmod_yuv420p_la_SOURCES = main.cpp trgt_yuv.cpp trgt_yuv.h
libmod_yuv420p_la_LDFLAGS = -module -no-undefined
libmod_yuv420p_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_yuv420p_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
