"""
Will store all the functions needed to export the translate layer
"""

import sys
import copy
import settings
from helpers.transform import gen_helpers_transform
from synfig.animation import print_animation, gen_dummy_waypoint
import synfig.group as group
sys.path.append("..")


def gen_layer_translate(lottie, layer):
    """
    Help generate transform properties
    """
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "origin":
                anchor = gen_dummy_waypoint(child, "param", "vector")
                pos = anchor

    anchor = copy.deepcopy(anchor)
    for waypoint in anchor[0]:
        waypoint[0][0].text = str(0)
        waypoint[0][1].text = str(0)
    group.update_pos(anchor)
    if settings.INSIDE_PRECOMP:
        group.update_pos(pos)
    gen_helpers_transform(lottie, layer, pos[0], anchor[0])
