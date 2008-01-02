# $Id$


MAINTAINERCLEANFILES=Makefile.in
INCLUDES = -I$(top_builddir) -I$(top_srcdir)/src

moduledir=@MODULE_DIR@

module_LTLIBRARIES = libmod_noise.la
libmod_noise_la_SOURCES = random.cpp random.h distort.cpp distort.h noise.cpp noise.h valuenode_random.cpp valuenode_random.h main.cpp
libmod_noise_la_CXXFLAGS = @SYNFIG_CFLAGS@
libmod_noise_la_LIBADD = ../../synfig/libsynfig.la @SYNFIG_LIBS@
libmod_noise_la_LDFLAGS = -module -no-undefined
EXTRA_DIST= mod_noise.nsh unmod_noise.nsh
