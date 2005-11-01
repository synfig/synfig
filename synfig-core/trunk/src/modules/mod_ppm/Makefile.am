# $Header: /opt/voria/cvs/synfig/src/modules/mod_ppm/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_ppm.la
libmod_ppm_la_SOURCES = main.cpp trgt_ppm.cpp trgt_ppm.h mptr_ppm.cpp mptr_ppm.h # trgt_mpg.cpp trgt_mpg.h
libmod_ppm_la_LDFLAGS = -module -no-undefined
libmod_ppm_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_ppm_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
EXTRA_DIST= mod_ppm.nsh unmod_ppm.nsh
