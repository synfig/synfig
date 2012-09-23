# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	tool.nsh

INCLUDES = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src \
	@BOOST_CPPFLAGS@


bin_PROGRAMS = \
	synfig

synfig_SOURCES = \
	definitions.h \
	progress.h \
	renderprogress.h \
	job.h \
	printing_functions.h \
	printing_functions.cpp \
	main.cpp

synfig_LDADD = \
	../synfig/libsynfig.la \
	@SYNFIG_LIBS@ \
	@OPENEXR_HALF_LIBS@ \
	@BOOST_LDFLAGS@ \
	@BOOST_PROGRAM_OPTIONS_LIB@

synfig_CXXFLAGS = \
	@SYNFIG_CFLAGS@
