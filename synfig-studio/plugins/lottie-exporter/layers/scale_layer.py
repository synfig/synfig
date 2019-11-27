"""
Will store all the functions needed to export the scale layer
"""

import sys
import copy
import settings
from helpers.transform import gen_helpers_transform
sys.path.append("..")


def gen_layer_scale(lottie, layer, layer_name="scale_layer"):
    """
    Help generate transform properties of a scale layer

    Args:
        lottie (dict) : Store transform properties in lottie format
        layer  (common.Layer.Layer) Transform properties in Synfig format

    Returns:
        (None)
    """
    animation_type = "scale_layer_zoom"
    if layer_name == "stretch_layer":
        animation_type = "stretch_layer_scale"

    center = layer.get_param("center")
    center.animate("vector")
    anchor = copy.deepcopy(center)
    # deep copy changes address of parent layer also
    anchor.parent = center.parent
    pos = center

    scale = layer.get_param("amount")  # This is scale amount
    scale.animate(animation_type)

    anchor.add_offset()
    if settings.INSIDE_PRECOMP:
        pos.add_offset()
    anchor.animate("vector", True)
    pos.animate("vector", True)
    gen_helpers_transform(lottie, pos, anchor, scale)
