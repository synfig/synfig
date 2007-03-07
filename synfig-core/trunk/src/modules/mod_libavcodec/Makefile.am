# $Header: /opt/voria/cvs/synfig/src/modules/mod_libavcodec/Makefile.am,v 1.1.1.1 2005/01/04 01:23:11 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_LIBAVCODEC
module_LTLIBRARIES = libmod_libavcodec.la
libmod_libavcodec_la_SOURCES = main.cpp trgt_av.cpp trgt_av.h mptr.cpp mptr.h
libmod_libavcodec_la_LDFLAGS = -module -no-undefined
libmod_libavcodec_la_CXXFLAGS = @SYNFIG_CFLAGS@ @LIBAVCODEC_CFLAGS@ @LIBAVFORMAT_CFLAGS@
libmod_libavcodec_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@ @LIBAVCODEC_LIBS@ @LIBAVFORMAT_LIBS@
else
endif
EXTRA_DIST= mod_libavcodec.nsh unmod_libavcodec.nsh 
