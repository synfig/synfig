# $Header: /opt/voria/cvs/sinfg/src/modules/mod_ppm/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_ppm.la
libmod_ppm_la_SOURCES = main.cpp trgt_ppm.cpp trgt_ppm.h mptr_ppm.cpp mptr_ppm.h # trgt_mpg.cpp trgt_mpg.h
libmod_ppm_la_LDFLAGS = -module -no-undefined
libmod_ppm_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_ppm_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
