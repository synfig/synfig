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
from layers.group import gen_layer_group
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
    shape_layer = settings.SHAPE_LAYER
    solid_layer = settings.SOLID_LAYER
    shape_solid_layer = settings.SHAPE_SOLID_LAYER
    image_layer = settings.IMAGE_LAYER
    pre_comp_layer = settings.PRE_COMP_LAYER
    group_layer = settings.GROUP_LAYER
    supported_layers = set.union(shape_layer, solid_layer, shape_solid_layer, image_layer, pre_comp_layer, group_layer)
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
            elif child.attrib["type"] in group_layer:       # Goto group layer
                gen_layer_group(lottie[-1],
                                child,
                                itr)
                # No return statement here
        itr -= 1
