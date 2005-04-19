# $Header: /opt/voria/cvs/synfig/src/modules/example/Makefile.am,v 1.1.1.1 2005/01/04 01:23:09 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libexample.la
libexample_la_SOURCES = main.cpp simplecircle.cpp simplecircle.h filledrect.h filledrect.cpp metaballs.h metaballs.cpp
libexample_la_LIBADD = ../../synfig/libsynfig.la @SYNFIG_LIBS@
libexample_la_LDFLAGS = -module -no-undefined
libexample_la_CXXFLAGS = @SYNFIG_CFLAGS@
