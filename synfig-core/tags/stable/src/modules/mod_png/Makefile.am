# $Header: /opt/voria/cvs/sinfg/src/modules/mod_png/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if HAVE_LIBPNG
module_LTLIBRARIES = libmod_png.la
libmod_png_la_SOURCES = main.cpp trgt_png.cpp trgt_png.h mptr_png.cpp mptr_png.h
libmod_png_la_LDFLAGS = -module @PNG_LIBS@ -no-undefined
libmod_png_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_png_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
else
endif
