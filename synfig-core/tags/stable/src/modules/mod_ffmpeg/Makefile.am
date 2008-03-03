# $Id$


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_builddir) -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_FFMPEG
module_LTLIBRARIES = libmod_ffmpeg.la
libmod_ffmpeg_la_SOURCES = main.cpp mptr_ffmpeg.cpp mptr_ffmpeg.h trgt_ffmpeg.cpp trgt_ffmpeg.h
libmod_ffmpeg_la_LDFLAGS = -module -no-undefined -avoid-version
libmod_ffmpeg_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_ffmpeg_la_LIBADD = ../../synfig/libsynfig.la @SYNFIG_LIBS@
else
endif


EXTRA_DIST= mod_ffmpeg.nsh unmod_ffmpeg.nsh
