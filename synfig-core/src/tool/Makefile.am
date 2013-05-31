# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	tool.nsh

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/src \
	@BOOST_CPPFLAGS@


bin_PROGRAMS = \
	synfig

synfig_SOURCES = \
	named_type.h \
	definitions.h \
	progress.h \
	renderprogress.h \
	job.h \
	synfigtoolexception.h \
	printing_functions.h \
	printing_functions.cpp \
	optionsprocessor.h \
	optionsprocessor.cpp \
	joblistprocessor.h \
	joblistprocessor.cpp \
	main.cpp

synfig_LDADD = \
	../synfig/libsynfig.la \
	@SYNFIG_LIBS@ \
	@OPENEXR_HALF_LIBS@ \
	@BOOST_LDFLAGS@ \
	@BOOST_PROGRAM_OPTIONS_LIB@

synfig_CXXFLAGS = \
	@SYNFIG_CFLAGS@
