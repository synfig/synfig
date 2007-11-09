# $Id$


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_builddir) -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_OPENEXR
module_LTLIBRARIES = libmod_openexr.la
libmod_openexr_la_SOURCES = main.cpp mptr_openexr.cpp mptr_openexr.h trgt_openexr.cpp trgt_openexr.h
libmod_openexr_la_LDFLAGS = -module @PNG_LIBS@ -no-undefined
libmod_openexr_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@ @OPENEXR_LIBS@
libmod_openexr_la_CXXFLAGS = @SYNFIG_CFLAGS@ @OPENEXR_CFLAGS@
else
endif


EXTRA_DIST= mod_openexr.nsh unmod_openexr.nsh
