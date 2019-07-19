"""
Layer.py
Will store the Layer class for Synfig layers
"""

import sys
import settings
sys.path.append("..")


class Layer:
    """
    Class to keep Synfig Layers
    """
    def __init__(self, layer):
        """
        """
        self.layer = layer
        self.params = {}
        self.extract_params(self.params)

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

    def extract_params(self, params):
        """
        Will extract the parameters from the layer and store in params
        """
        for child in self.layer:
            if child.tag == "param":
                key = child.attrib["name"]
                params[key] = child

    def get_param(self, *keys):
        """
        Given a key, this will return the address of the child corresponding to
        it
        If multiple keys are given, it will return the key found first
        """
        for key in keys:
            if key in self.params.keys():
                return self.params[key]
        return None
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
