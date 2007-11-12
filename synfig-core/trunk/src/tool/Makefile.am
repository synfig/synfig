# $Id$

MAINTAINERCLEANFILES=Makefile.in
EXTRA_DIST=tool.nsh

INCLUDES=-I$(top_builddir) -I$(top_srcdir)/src

bin_PROGRAMS	=	synfig

synfig_SOURCES = main.cpp
synfig_LDADD = -L../synfig -lsynfig @SYNFIG_LIBS@ @OPENEXR_HALF_LIBS@
synfig_CXXFLAGS = @SYNFIG_CFLAGS@
#synfig_LDFLAGS=-export-dynamic  -dlopen self
#-dlopen ../modules/example/libexample.la

