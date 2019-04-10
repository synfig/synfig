# $Id$

PLUGIN_NAME = lottie-exporter

EXTRA_FILES = canvas.py \
			  misc.py \
			  settings.py \
			  helpers/transform.py \
			  layers/shape.py \
			  properties/multiDimensionalKeyframed.py \
			  properties/offsetKeyframe.py \
			  properties/value.py \
			  shapes/fill.py \
			  shapes/star.py

plugindir = ${datadir}/synfig/plugins/$(PLUGIN_NAME)
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
