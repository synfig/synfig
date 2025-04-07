#!/usr/bin/env python
"""
Canvas.py
This module defines the Canvas class for spine-exporter.
It loads a Synfig XML canvas (.sif), extracts definitions, layers, and bones,
and provides helper methods to access these elements.
"""

import settings
from core.Param import Param
from core.Layer import Layer

class Canvas:
    def __init__(self, canvas, is_root_canvas=False):
        if is_root_canvas:
            settings.ROOT_CANVAS = self
        # If 'canvas' is a Param instance then extract its underlying XML element.
        if hasattr(canvas, 'get'):
            self.canvas_elem = canvas.get()[0]
            self.parent_param = canvas
        else:
            self.canvas_elem = canvas
            self.parent_param = None

        self.defs = {}    # dictionary of definition nodes keyed by id.
        self.layers = []  # list of layer objects.
        self.bones = {}   # dictionary of bones keyed by guid.

        self.extract_defs()
        self.extract_layers()
        self.extract_bones()
        self.set_name()

    def set_name(self):
        """
        Set the canvas name based on the <name> tag if available.
        Fallback to a default name from settings.
        """
        found = False
        for child in self.canvas_elem:
            if child.tag == "name":
                self.name = child.text
                found = True
                break
        if not found:
            # settings.CANVAS_NAME and settings.canvas_count should be defined in settings.py
            self.name = settings.CANVAS_NAME + str(settings.canvas_count.inc())

    def extract_defs(self):
        """
        Extract definitions contained in the <defs> tag.
        """
        for child in self.canvas_elem:
            if child.tag == "defs":
                for def_child in child:
                    key = def_child.attrib.get("id")
                    if key is not None:
                        self.defs[key] = def_child

    def extract_layers(self):
        """
        Extract all layers from the canvas.
        Each layer is wrapped into a Layer instance (defined in core/Layer.py).
        """
        for child in self.canvas_elem:
            if child.tag == "layer":
                self.layers.append(Layer(child, self))

    def extract_bones(self):
        """
        Extract all bones defined under the <bones> tag.
        Each bone element is wrapped as a Param instance.
        """
        for child in self.canvas_elem:
            if child.tag == "bones":
                # Iterate over each bone element inside the bones block.
                for bone in child:
                    guid = bone.attrib.get("guid")
                    if guid:
                        self.bones[guid] = Param(bone, self)

    def get_bone(self, guid):
        """
        Retrieve a bone by its guid.
        guid - the unique identifier for the bone.
        Returns the Param-wrapped bone or None if not found.
        """
        return self.bones.get(guid)

    def get_canvas(self):
        """
        Returns the underlying XML element of the canvas.
        """
        return self.canvas_elem

    def getparent_param(self):
        """
        Returns the parent parameter if the canvas was built from a Param.
        """
        return self.parent_param

    def __getitem__(self, index):
        """
        Allow layer list access via index.
        """
        return self.layers[index]

    def get_num_layers(self):
        """
        Return the number of layers in the canvas.
        """
        return len(self.layers)

    def get_layer_list(self):
        """
        Return the full list of layers.
        """
        return self.layers