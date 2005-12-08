# $Header: /opt/voria/cvs/synfig/src/modules/mod_ffmpeg/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_FFMPEG
module_LTLIBRARIES = libmod_ffmpeg.la
libmod_ffmpeg_la_SOURCES = main.cpp mptr_ffmpeg.cpp mptr_ffmpeg.h trgt_ffmpeg.cpp trgt_ffmpeg.h
libmod_ffmpeg_la_LDFLAGS = -module -no-undefined
libmod_ffmpeg_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_ffmpeg_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
else
endif


EXTRA_DIST= mod_ffmpeg.nsh unmod_ffmpeg.nsh
