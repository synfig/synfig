# $Id$

MAINTAINERCLEANFILES = Makefile.in

PLUGIN_NAME = add-skeleton-simple

EXTRA_FILES = stickman.sif

plugindir = ${datadir}/synfig/plugins/$(PLUGIN_NAME)
plugin_DATA = \
	$(PLUGIN_NAME).py \
	plugin.xml \
	$(EXTRA_FILES)

@INTLTOOL_XML_RULE@

EXTRA_DIST = $(plugin_DATA)
