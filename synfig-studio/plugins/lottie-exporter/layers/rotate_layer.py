"""
Will store all the functions needed to export the rotate layer
"""

import sys
import settings
from helpers.transform import gen_helpers_transform
from synfig.animation import print_animation, gen_dummy_waypoint
sys.path.append("..")


def gen_layer_rotate_layer(lottie, layer):
    """
    Help generate transform properties
    """
    pos = [settings.lottie_format["w"]/2, settings.lottie_format["h"]/2]
    anchor = [0, 0, 0]
    scale = [100, 100, 100]
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "origin":
                anchor = gen_dummy_waypoint(child, "param", "vector")[0]
            elif child.attrib["name"] == "amount":  # This is rotation
                rotation = gen_dummy_waypoint(child, "param", "angle")[0]
                rotation.attrib["type"] = "rotate_layer_angle"
                # Angle needs to made neg of what they are
                for waypoint in rotation:
                    waypoint[0].attrib["value"] = str(-float(waypoint[0].attrib["value"]))

    gen_helpers_transform(lottie, layer, anchor, anchor, scale, rotation)
