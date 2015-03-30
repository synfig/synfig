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
	renderprogress.h   \
	renderprogress.cpp \
	job.h \
	synfigtoolexception.h \
	printing_functions.h \
	printing_functions.cpp \
	optionsprocessor.h \
	optionsprocessor.cpp \
	joblistprocessor.h \
	joblistprocessor.cpp \
	definitions.cpp \
	main.cpp

synfig_LDADD = \
	../synfig/libsynfig.la \
	@SYNFIG_LIBS@ \
	@OPENEXR_HALF_LIBS@ \
	@BOOST_LDFLAGS@ \
	@BOOST_PROGRAM_OPTIONS_LIB@ \
	@BOOST_SYSTEM_LIB@ \
	@BOOST_FILESYSTEM_LIB@ \
	@BOOST_CHRONO_LIB@

synfig_CXXFLAGS = \
	@SYNFIG_CFLAGS@
