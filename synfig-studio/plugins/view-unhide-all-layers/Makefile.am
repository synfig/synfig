PLUGIN_NAME = view-unhide-all-layers

EXTRA_FILES = 

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
