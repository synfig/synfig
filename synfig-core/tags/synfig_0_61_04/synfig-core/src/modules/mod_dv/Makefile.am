# $Header: /opt/voria/cvs/synfig/src/modules/mod_dv/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_LIBDV
module_LTLIBRARIES = libmod_dv.la
libmod_dv_la_SOURCES = main.cpp trgt_dv.cpp trgt_dv.h
libmod_dv_la_LDFLAGS = -module -no-undefined
libmod_dv_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_dv_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
else
endif
EXTRA_DIST= mod_dv.nsh unmod_dv.nsh
