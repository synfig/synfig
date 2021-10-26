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
	@BOOST_LDFLAGS@

synfig_CXXFLAGS = \
	@SYNFIG_CFLAGS@
