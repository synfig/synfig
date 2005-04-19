# $Header: /opt/voria/cvs/synfig/src/modules/mptr_mplayer/Makefile.am,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmptr_mplayer.la
libmptr_mplayer_la_SOURCES = main.cpp mptr_mplayer.cpp mptr_mplayer.h
libmptr_mplayer_la_LDFLAGS = -module -no-undefined
libmptr_mplayer_la_LIBADD = -L../../synfig -lsynfig @SYNFIG_LIBS@



