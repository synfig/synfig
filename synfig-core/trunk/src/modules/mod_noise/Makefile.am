# $Header: /opt/voria/cvs/sinfg/src/modules/mod_noise/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_noise.la
libmod_noise_la_SOURCES = random.cpp random.h distort.cpp distort.h noise.cpp noise.h main.cpp 
libmod_noise_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_noise_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
libmod_noise_la_LDFLAGS = -module -no-undefined
