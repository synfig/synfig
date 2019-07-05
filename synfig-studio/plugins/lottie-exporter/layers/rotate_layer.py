"""
Will store all the functions needed to export the rotate layer
"""

import sys
import copy
import settings
from helpers.transform import gen_helpers_transform
from synfig.animation import print_animation, gen_dummy_waypoint
from synfig.group import update_precomp
sys.path.append("..")


def gen_layer_rotate_layer(lottie, layer):
    """
    Help generate transform properties
    """
    scale = [100, 100, 100]
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "origin":
                anchor = gen_dummy_waypoint(child, "param", "vector")[0]
                anchor.attrib["name"] = "origin"
                pos = anchor
            elif child.attrib["name"] == "amount":  # This is rotation
                rotation = gen_dummy_waypoint(child, "param", "angle")[0]
                rotation.attrib["type"] = "rotate_layer_angle"
                # Angle needs to made neg of what they are
                for waypoint in rotation:
                    waypoint[0].attrib["value"] = str(-float(waypoint[0].attrib["value"]))

    anchor = copy.deepcopy(anchor)
    update_precomp(anchor)
    if settings.INSIDE_PRECOMP:
        update_precomp(pos)
    print_animation(anchor)
    gen_helpers_transform(lottie, layer, pos, anchor, scale, rotation)
