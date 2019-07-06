"""
Will store all the functions needed to export the scale layer
"""

import sys
import copy
import settings
from helpers.transform import gen_helpers_transform
from synfig.animation import gen_dummy_waypoint
import synfig.group as group
sys.path.append("..")


def gen_layer_scale(lottie, layer):
    """
    Help generate transform properties of a scale layer

    Args:
        lottie (dict) : Store transform properties in lottie format
        layer  (lxml.etree._Element) Transform properties in Synfig format

    Returns:
        (None)
    """
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "center":
                anchor = gen_dummy_waypoint(child, "param", "vector")
                pos = anchor
            elif child.attrib["name"] == "amount":  # This is rotation
                scale = gen_dummy_waypoint(child, "param", "amount")
                scale[0].attrib["type"] = "scale_layer_zoom"

    anchor = copy.deepcopy(anchor)
    group.update_pos(anchor)
    if settings.INSIDE_PRECOMP:
        group.update_pos(pos)
    gen_helpers_transform(lottie, layer, pos[0], anchor[0], scale[0])
