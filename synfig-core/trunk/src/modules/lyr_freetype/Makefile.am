# $Header: /opt/voria/cvs/sinfg/src/modules/lyr_freetype/Makefile.am,v 1.2 2005/01/16 19:54:09 darco Exp $

MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src
CXXFLAGS=@CXXFLAGS@ @FT2_CFLAGS@

moduledir=@MODULE_DIR@

if WITH_FREETYPE
module_LTLIBRARIES = liblyr_freetype.la
liblyr_freetype_la_SOURCES = main.cpp lyr_freetype.cpp lyr_freetype.h
liblyr_freetype_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
liblyr_freetype_la_LDFLAGS = -module @FT2_LIBS@ -no-undefined
liblyr_freetype_la_CXXFLAGS = @SINFG_CFLAGS@
else
endif


