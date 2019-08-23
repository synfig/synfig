"""
Layer.py
Will store the Layer class for Synfig layers
"""

import sys
import settings
from common.Param import Param
sys.path.append("..")


class Layer:
    """
    Class to keep Synfig Layers
    """
    def __init__(self, layer, parent_canvas):
        """
        """
        self.parent_canvas = parent_canvas
        self.layer = layer
        self.params = {}
        self.extract_params(self.params)
        self.set_description()


    def set_lottie_layer(self, lottie):
        """
        Stores the path to lottie format of layer
        """
        self.lottie_layer = lottie

    def get_lottie_layer(self):
        """
        Returns the path to lottie format of layer
        """
        return self.lottie_layer

    def get_layer(self):
        """
        Returns the original address of the layer
        This function might not be needed once class `Layer` is fully embedded
        in the converter
        """
        return self.layer

    def get_type(self):
        """
        Returns the type of layer
        """
        if "type" in self.layer.keys():
            return self.layer.attrib["type"]
        return settings.UNKNOWN_LAYER

    def add_param(self, key, param):
        """
        param is of type __something__
        Manually adding some parameter
        """
        self.params[key] = Param(param, self)
        return self.params[key]

    def set_group_params(self):
        """
        If the current layer is group, it sets the immediate children parameters
        as group_children=1. This is needed in convert methods like
        radial_composite and bone_link
        """
        typ = self.get_type()
        if typ in settings.GROUP_LAYER or typ in settings.PRE_COMP_LAYER:
            for param in self.params.values():
                param.is_group_child = True

    def extract_params(self, params):
        """
        Will extract the parameters from the layer and store in params
        """
        for child in self.layer:
            if child.tag == "param":
                key = child.attrib["name"]
                params[key] = Param(child, self)
        self.set_group_params()

    def get_param(self, *keys):
        """
        Given a key, this will return the address of the child corresponding to
        it
        If multiple keys are given, it will return the key found first
        """
        for key in keys:
            if key in self.params.keys():
                return self.params[key]
        return Param(None, None)
        #raise KeyError("No parameter %s in Layer %s", key, self.get_type())

    def is_active(self):
        """
        Returns true if a layer is active
        """
        if self.layer.attrib["active"] == "true":
            return True
        return False

    def to_render(self):
        """
        Returns true if we have to render the layer
        """
        key = "exclude_from_rendering"
        if key in self.layer.keys() and self.layer.attrib[key] == "true":
            return False
        return True

    def set_description(self):
        """
        Sets the layer description if given, otherwise the default description
        is set
        """
        if "desc" in self.layer.keys():
            self.description = self.layer.attrib["desc"]
        else:
            # Set the default name
            if self.get_type() in settings.SHAPE_LAYER:
                desc = settings.LAYER_SHAPE_NAME
            elif self.get_type() in set.union(settings.SOLID_LAYER, settings.SHAPE_SOLID_LAYER):
                desc = settings.LAYER_SOLID_NAME
            elif self.get_type() in settings.IMAGE_LAYER:
                desc = settings.LAYER_IMAGE_NAME
            elif self.get_type() in set.union(settings.PRE_COMP_LAYER, settings.GROUP_LAYER):
                desc = settings.LAYER_PRECOMP_NAME
            else:
                desc = settings.UNKNOWN_LAYER
            desc += str(settings.layer_count.inc())
            self.description = desc

    def get_description(self):
        """
        Getter for description of the layer
        """
        return self.description

    def getparent(self):
        """
        Returns the parent of this Layer
        return type: common.Canvas.Canvas
        """
        return self.parent_canvas

    def add_offset(self):
        """
        Inserts necassary offset in the positions of the layers if they lie inside
        another composition of Lottie
        """
        # This if condition is not applicable for group, rotate, precomp... layers
        if not settings.INSIDE_PRECOMP:
            return

        update_dict = []
        compare = {"center", "origin", "point1", "point2", "tl", "br"}
        for key in compare:
            param = self.get_param(key)
            if param.get() is not None:
                update_dict.append(param)

        for param in update_dict:
            param.add_offset()
