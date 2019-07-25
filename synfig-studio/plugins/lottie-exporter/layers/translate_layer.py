"""
Will store all the functions needed to export the translate layer
"""

import sys
import copy
import settings
from helpers.transform import gen_helpers_transform
import synfig.group as group
sys.path.append("..")


def gen_layer_translate(lottie, layer):
    """
    Help generate transform properties of translate layer

    Args:
        lottie (dict) : Transform properties in lottie format
        layer  (common.Layer.Layer) : Transform properties in Synfig format

    Returns:
        (None)
    """

    origin = layer.get_param("origin")
    origin.animate("vector")
    anchor = copy.deepcopy(origin)
    pos = origin

    for waypoint in anchor[0]:
        waypoint[0][0].text = str(0)
        waypoint[0][1].text = str(0)
    anchor.add_offset()

    if settings.INSIDE_PRECOMP:
        pos.add_offset()

    gen_helpers_transform(lottie, pos, anchor)
