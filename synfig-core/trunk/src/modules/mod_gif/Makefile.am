# $Header: /opt/voria/cvs/sinfg/src/modules/mod_gif/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_gif.la
libmod_gif_la_SOURCES = main.cpp trgt_gif.cpp trgt_gif.h
libmod_gif_la_LDFLAGS = -module -no-undefined
libmod_gif_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_gif_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
