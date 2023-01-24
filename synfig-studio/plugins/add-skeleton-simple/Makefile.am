PLUGIN_NAME = add-skeleton-simple

EXTRA_FILES = stickman.sif

plugindir = $(synfig_datadir)/plugins/$(PLUGIN_NAME)
plugin_DATA = \
	$(PLUGIN_NAME).py \
	plugin.xml \
	$(EXTRA_FILES)

@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(PLUGIN_NAME).py \
	plugin.xml.in \
	$(EXTRA_FILES)
	
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = plugin.xml
