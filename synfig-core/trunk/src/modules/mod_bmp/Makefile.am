# $Header: /opt/voria/cvs/sinfg/src/modules/mod_bmp/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_bmp.la
libmod_bmp_la_SOURCES = main.cpp trgt_bmp.cpp trgt_bmp.h mptr_bmp.cpp mptr_bmp.h
libmod_bmp_la_LDFLAGS = -module -no-undefined
libmod_bmp_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_bmp_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
