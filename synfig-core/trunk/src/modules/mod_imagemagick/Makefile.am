# $Id$


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

if WITH_IMAGEMAGICK
module_LTLIBRARIES = libmod_imagemagick.la
libmod_imagemagick_la_SOURCES = main.cpp mptr_imagemagick.cpp mptr_imagemagick.h trgt_imagemagick.cpp trgt_imagemagick.h
libmod_imagemagick_la_LDFLAGS = -module -no-undefined
libmod_imagemagick_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_imagemagick_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@
else
endif


EXTRA_DIST= mod_imagemagick.nsh unmod_imagemagick.nsh
