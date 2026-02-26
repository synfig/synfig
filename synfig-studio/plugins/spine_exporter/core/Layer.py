#!/usr/bin/env python
"""
Layer.py
This module defines the Layer class for spine-exporter.
It wraps a Synfig layer XML element, extracts its parameters,
and makes available basic properties such as type, activity state,
and description. This is adapted from the lottie-exporter version.
"""

import sys
import settings
from core.Param import Param
sys.path.append("..")

class Layer:
    def init(self, layer, parent_canvas):

        self.parent_canvas = parent_canvas
        self.layer = layer
        self.params = {} # Dictionary to hold parameters keyed by name.
        self.extract_params()
        self.set_description()

    def extract_params(self):
        """
        Extracts direct child parameters (with the tag "param") from the layer XML element.
        Each parameter is wrapped as a Param instance and stored in self.params using its name attribute.
        """
        for child in self.layer:
            if child.tag == "param":
                key = child.attrib.get("name")
                if key:
                    self.params[key] = Param(child, self)
        # (Optionally, mark parameters from group layers if needed.)
        self.set_group_params()

    def set_group_params(self):
        """
        If this layer is a group (or precomp) layer, mark its parameters as being
        part of a group. The spine-exporter may not need this behavior immediately,
        but it is retained for future expansion.
        """
        layer_type = self.get_type()
        if layer_type in settings.GROUP_LAYER or layer_type in settings.PRE_COMP_LAYER:
            for param in self.params.values():
                param.is_group_child = True

    def get_param(self, *keys):
        """
        Returns the first parameter found among the given keys.
        If none is found, returns a dummy Param (or could throw an error).
        """
        for key in keys:
            if key in self.params:
                return self.params[key]
        return Param(None, None)  # Fallback; consider raising an exception if preferred.

    def get_type(self):
        """
        Returns the layer type from the XML attribute "type".
        If undefined, returns a default value from settings.
        """
        if "type" in self.layer.attrib:
            return self.layer.attrib["type"]
        return settings.UNKNOWN_LAYER

    def is_active(self):
        """
        Returns True if the layer is active.
        """
        return self.layer.attrib.get("active", "true") == "true"

    def to_render(self):
        """
        Returns True if the layer should be rendered.
        Checks for an attribute 'exclude_from_rendering'.
        """
        if self.layer.attrib.get("exclude_from_rendering", "false") == "true":
            return False
        return True

    def set_description(self):
        """
        Set a human-readable description for the layer.
        If there is no description, a default name is generated based on the layer type and a counter.
        """
        if "desc" in self.layer.attrib:
            self.description = self.layer.attrib["desc"]
        else:
            # Choose a default description based on the layer type.
            layer_type = self.get_type()
            if layer_type in settings.SHAPE_LAYER:
                desc = settings.LAYER_SHAPE_NAME
            elif layer_type in settings.SOLID_LAYER or layer_type in settings.SHAPE_SOLID_LAYER:
                desc = settings.LAYER_SOLID_NAME
            elif layer_type in settings.IMAGE_LAYER:
                desc = settings.LAYER_IMAGE_NAME
            elif layer_type in settings.PRE_COMP_LAYER or layer_type in settings.GROUP_LAYER:
                desc = settings.LAYER_PRECOMP_NAME
            else:
                desc = settings.UNKNOWN_LAYER
            # Append a unique counter for uniqueness.
            desc += str(settings.layer_count.inc())
            self.description = desc

    def get_description(self):
        """
        Returns the description of the layer.
        """
        return self.description

    def get_layer(self):
        """
        Returns the original XML element of the layer.
        """
        return self.layer

    def getparent(self):
        """
        Returns the parent canvas of this layer.
        """
        return self.parent_canvas

    # Optionally, if offsets are needed for export, a function like add_offset() could be defined.
    def add_offset(self):
        """
        Inserts necessary offsets into the positions of the layer's parameters
        if they reside inside another composition. For now, this functionality
        is a stub and can be developed further if needed.
        """
        