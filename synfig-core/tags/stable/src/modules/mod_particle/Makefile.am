# $Header: /opt/voria/cvs/sinfg/src/modules/mod_particle/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_particle.la
libmod_particle_la_SOURCES = random.cpp random.h plant.cpp plant.h main.cpp 
libmod_particle_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_particle_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
libmod_particle_la_LDFLAGS = -module -no-undefined
