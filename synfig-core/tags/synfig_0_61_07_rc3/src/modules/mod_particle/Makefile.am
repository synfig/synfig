# $Id$


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_particle.la
libmod_particle_la_SOURCES = random.cpp random.h plant.cpp plant.h main.cpp 
libmod_particle_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_particle_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
libmod_particle_la_LDFLAGS = -module -no-undefined
EXTRA_DIST= mod_particle.nsh unmod_particle.nsh
