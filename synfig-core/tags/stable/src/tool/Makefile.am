# $Header: /opt/voria/cvs/sinfg/src/tool/Makefile.am,v 1.1.1.1 2005/01/04 01:23:15 darco Exp $

MAINTAINERCLEANFILES=Makefile.in

INCLUDES=-I$(top_srcdir)/src

bin_PROGRAMS	=	sinfg
sinfg_SOURCES = main.cpp
sinfg_LDADD = -L../sinfg -lsinfg @SINFG_LIBS@
sinfg_CXXFLAGS = -lsinfg @SINFG_CFLAGS@
#sinfg_LDFLAGS=-export-dynamic  -dlopen self
#-dlopen ../modules/example/libexample.la

