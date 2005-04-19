# $Header: /opt/voria/cvs/synfig/src/modules/mod_openexr/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_OPENEXR
module_LTLIBRARIES = libmod_openexr.la
libmod_openexr_la_SOURCES = main.cpp mptr_openexr.cpp mptr_openexr.h trgt_openexr.cpp trgt_openexr.h
libmod_openexr_la_LDFLAGS = -module @PNG_LIBS@ -no-undefined
libmod_openexr_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
libmod_openexr_la_CXXFLAGS = @SYNFIG_CFLAGS@
else
endif


