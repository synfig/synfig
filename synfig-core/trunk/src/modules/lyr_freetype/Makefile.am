# $Header: /opt/voria/cvs/synfig/src/modules/lyr_freetype/Makefile.am,v 1.3 2005/03/24 21:07:24 darco Exp $

MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src
CXXFLAGS=@CXXFLAGS@ @FREETYPE_CFLAGS@

moduledir=@MODULE_DIR@

if WITH_FREETYPE
module_LTLIBRARIES = liblyr_freetype.la
liblyr_freetype_la_SOURCES = main.cpp lyr_freetype.cpp lyr_freetype.h
liblyr_freetype_la_LIBADD = -L../../synfig -lsynfig @FREETYPE_LIBS@ @SYNFIG_LIBS@
liblyr_freetype_la_LDFLAGS = -module -no-undefined
liblyr_freetype_la_CXXFLAGS = @SYNFIG_CFLAGS@ @FREETYPE_CFLAGS@
else
endif


