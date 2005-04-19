# $Header: /opt/voria/cvs/synfig/src/modules/mod_noise/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_noise.la
libmod_noise_la_SOURCES = random.cpp random.h distort.cpp distort.h noise.cpp noise.h main.cpp 
libmod_noise_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_noise_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
libmod_noise_la_LDFLAGS = -module -no-undefined
