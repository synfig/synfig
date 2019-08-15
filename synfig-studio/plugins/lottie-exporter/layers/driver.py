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


def gen_layers(lottie, canvas, layer_itr):
    """
    This function will be called for each canvas/composition. Main function to
    generate all the layers

    Args:
        lottie (dict) : Layers in Lottie format
        canvas (common.Canvas.Canvas) : Synfig format canvas
        layer_itr (int) : position of layer in canvas

    Returns:
        (None)
    """
    itr = layer_itr
    shape = settings.SHAPE_LAYER
    solid = settings.SOLID_LAYER
    shape_solid = settings.SHAPE_SOLID_LAYER
    image = settings.IMAGE_LAYER
    pre_comp = settings.PRE_COMP_LAYER
    group = settings.GROUP_LAYER
    skeleton = settings.SKELETON_LAYER
    supported_layers = set.union(shape, solid, shape_solid, image, pre_comp, group, skeleton)
    while itr >= 0:
        layer = canvas[itr]
        if layer.get_type() not in supported_layers:  # Only supported layers
            logging.warning(settings.NOT_SUPPORTED_TEXT, layer.get_type())
            itr -= 1
            continue
        elif not layer.is_active():   # Only render the active layers
            logging.info(settings.NOT_ACTIVE_TEXT, layer.get_type())
            itr -= 1
            continue
        elif not layer.to_render():   # If we don't have to render the layer
            logging.info(settings.EXCLUDE_FROM_RENDERING, layer.get_type())
            itr -= 1
            continue

        lottie.append({})
        layer.set_lottie_layer(lottie[-1])

        if layer.get_type() in shape:           # Goto shape layer
            gen_layer_shape(lottie[-1],
                            layer,
                            itr)
        elif layer.get_type() in solid:         # Goto solid layer
            gen_layer_solid(lottie[-1],
                            layer,
                            itr)
        elif layer.get_type() in shape_solid:   # Goto shape_solid layer
            gen_layer_shape_solid(lottie[-1],
                                  layer,
                                  itr)
        elif layer.get_type() in image:   # Goto image layer
            gen_layer_image(lottie[-1],
                            layer,
                            itr)
        elif layer.get_type() in pre_comp:      # Goto precomp layer
            gen_layer_precomp(lottie[-1],
                              layer,
                              itr)
            return  # other layers will be generated inside the precomp
        elif layer.get_type() in group:       # Goto group layer
            gen_layer_group(lottie[-1],
                            layer,
                            itr)
            # No return statement here
        elif layer.get_type() in skeleton:
            pass
            # skeletons are just for linking purposes which is served by bones
        itr -= 1
