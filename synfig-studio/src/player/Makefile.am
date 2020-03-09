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

bin_PROGRAMS = synfigplayer

synfigplayer_SOURCES = \
	$(VISUALIZATION_HH) \
	$(VISUALIZATION_CC)

synfigplayer_LDADD = \
	@SYNFIG_LIBS@ \
	@GTKMM_LIBS@

synfigplayer_LDFLAGS = \
	-dlopen self

synfigplayer_CXXFLAGS = \
	@SYNFIG_CFLAGS@ \
	@GTKMM_CFLAGS@ \
	-DPLUGIN_DIR=\"$(plugindir)\" \
	-DLOCALEDIR=\"${LOCALEDIR}\"
