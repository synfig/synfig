MAINTAINERCLEANFILES=Makefile.in
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_svg.la
libmod_svg_la_SOURCES = main.cpp layer_svg.h layer_svg.cpp svg_parser.h svg_parser.cpp
libmod_svg_la_LDFLAGS = -module -no-undefined -avoid-version
libmod_svg_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_svg_la_LIBADD = ../../synfig/libsynfig.la @SYNFIG_LIBS@
EXTRA_DIST= mod_svg.nsh unmod_svg.nsh

