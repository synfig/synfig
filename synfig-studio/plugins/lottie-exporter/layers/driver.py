# pylint: disable=line-too-long
"""
Main driver functions to generate layers
"""

import sys
import logging
import settings
from layers.shape import gen_layer_shape
from layers.solid import gen_layer_solid
from layers.image import gen_layer_image
from layers.shape_solid import gen_layer_shape_solid
from layers.preComp import gen_layer_precomp
sys.path.append("..")


def gen_layers(lottie, root, layer_itr):
    """
    This function will be called for each canvas/composition. Main function to
    generate all the layers

    Args:
        lottie (dict) : Layers in Lottie format
        root   (lxml.etree._Element) : All layers of a canvas in Synfig format
        layer_itr (int) : position of layer in canvas

    Returns:
        (None)
    """
    itr = layer_itr
    shape_layer = {"simple_circle"}
    solid_layer = {"SolidColor"}
    shape_solid_layer = {"region", "polygon", "outline", "circle", "rectangle", "filled_rectangle", "star"}
    image_layer = {"import"}
    pre_comp_layer = {"rotate", "zoom", "translate", "group"}
    supported_layers = shape_layer.union(solid_layer)
    supported_layers = supported_layers.union(shape_solid_layer)
    supported_layers = supported_layers.union(image_layer)
    supported_layers = supported_layers.union(pre_comp_layer)
    while itr >= 0:
        child = root[itr]
        if child.tag == "layer":
            if child.attrib["type"] not in supported_layers:  # Only supported layers
                logging.warning(settings.NOT_SUPPORTED_TEXT, child.attrib["type"])
                itr -= 1
                continue
            elif child.attrib["active"] == "false":   # Only render the active layers
                logging.info(settings.NOT_ACTIVE_TEXT, child.attrib["type"])
                itr -= 1
                continue
            elif "exclude_from_rendering" in child.keys() and child.attrib["exclude_from_rendering"] == "true":
                logging.info(settings.EXCLUDE_FROM_RENDERING, child.attrib["type"])
                itr -= 1
                continue

            lottie.append({})
            if child.attrib["type"] in shape_layer:           # Goto shape layer
                gen_layer_shape(lottie[-1],
                                child,
                                itr)
            elif child.attrib["type"] in solid_layer:         # Goto solid layer
                gen_layer_solid(lottie[-1],
                                child,
                                itr)
            elif child.attrib["type"] in shape_solid_layer:   # Goto shape_solid layer
                gen_layer_shape_solid(lottie[-1],
                                      child,
                                      itr)
            elif child.attrib["type"] in image_layer:   # Goto image layer
                gen_layer_image(lottie[-1],
                                child,
                                itr)
            elif child.attrib["type"] in pre_comp_layer:      # Goto precomp layer
                gen_layer_precomp(lottie[-1],
                                  child,
                                  itr)
                return  # other layers will be generated inside the precomp
        itr -= 1
