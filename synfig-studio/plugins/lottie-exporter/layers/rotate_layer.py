"""
Will store all the functions needed to export the rotate layer
"""

import sys
import copy
import settings
from helpers.transform import gen_helpers_transform
import synfig.group as group
sys.path.append("..")


def gen_layer_rotate(lottie, layer):
    """
    Help generate transform properties of a rotate layer

    Args:
        lottie (dict) : Will store the transform properties in lottie format
        layer  (common.Layer.Layer) : Tranform properties in Synfig format

    Returns:
        (None)
    """
    scale = settings.DEFAULT_SCALE
    origin = layer.get_param("origin")
    origin.animate("vector")
    anchor = origin.get()
    pos = anchor

    amount = layer.get_param("amount")  # This is rotation amount
    amount.animate("rotate_layer_angle")

    anchor = copy.deepcopy(anchor)
    group.update_pos(anchor)
    if settings.INSIDE_PRECOMP:
        group.update_pos(pos)
    gen_helpers_transform(lottie, pos[0], anchor[0], scale, amount[0])
