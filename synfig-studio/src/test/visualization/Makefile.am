# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

VISUALIZATION_HH = \
	visualizationwindow.h

VISUALIZATION_CC = \
	visualizationwindow.cpp \
	main.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src

bin_PROGRAMS = vizualization

vizualization_SOURCES = \
	$(VISUALIZATION_HH) \
	$(VISUALIZATION_CC)

vizualization_LDADD = \
	@SYNFIG_LIBS@ \
	@GTKMM_LIBS@

vizualization_LDFLAGS = \
	-dlopen self

vizualization_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	@GTKMM_CFLAGS@ \
	-DPLUGIN_DIR=\"$(plugindir)\" \
	-DLOCALEDIR=\"${LOCALEDIR}\"
