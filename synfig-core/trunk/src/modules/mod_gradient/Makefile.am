# $Header: /opt/voria/cvs/sinfg/src/modules/mod_gradient/Makefile.am,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_gradient.la
libmod_gradient_la_SOURCES = curvegradient.cpp curvegradient.h  lineargradient.cpp lineargradient.h conicalgradient.cpp conicalgradient.h spiralgradient.cpp spiralgradient.h radialgradient.cpp radialgradient.h main.cpp
libmod_gradient_la_CXXFLAGS = @SINFG_CFLAGS@
libmod_gradient_la_LIBADD = -L../../sinfg -lsinfg @SINFG_LIBS@
libmod_gradient_la_LDFLAGS = -module -no-undefined
