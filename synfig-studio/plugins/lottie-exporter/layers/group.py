# pylint: disable=line-too-long
"""
Store all functions corresponding to group layer in Synfig
"""

import sys
import settings
from sources.precomp import add_precomp_asset
from helpers.transform import gen_helpers_transform
from synfig.animation import print_animation, gen_dummy_waypoint
import synfig.group as group
sys.path.append("..")


def gen_layer_group(lottie, layer, idx):
    """
    Will generate a pre composition but has small differences than pre-comp layer used in
    layers/preComp.py
    """
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_PRECOMP_TYPE
    lottie["nm"] = settings.LAYER_PRECOMP_NAME + str(idx)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "canvas":
                canvas = chld
            elif chld.attrib["name"] == "transformation":
                transform = chld[0]
                for child in transform:
                    if child.tag == "scale":
                        scale = child
                    elif child.tag == "offset":
                        pos = child
                    elif child.tag == "angle":
                        angle = child
            elif chld.attrib["name"] == "origin":
                origin = chld

    origin = gen_dummy_waypoint(origin, "param", "vector")
    anchor = origin
    group.update_pos(anchor)

    angle = gen_dummy_waypoint(angle, "angle", "angle")
    angle[0].attrib["type"] = "rotate_layer_angle"

    pos = gen_dummy_waypoint(pos, "offset", "vector")
    #group.update_pos(pos)

    scale = gen_dummy_waypoint(scale, "scale", "vector")
    scale[0].attrib["type"] = "group_layer_scale"
    # Generate the transform properties here
    gen_helpers_transform(lottie["ks"], layer, pos[0], anchor[0], scale[0], angle[0])

    prev_state = settings.INSIDE_PRECOMP
    settings.INSIDE_PRECOMP = True

    settings.lottie_format["assets"].append({})
    asset = add_precomp_asset(settings.lottie_format["assets"][-1], canvas[0], len(canvas[0]))
    lottie["refId"] = asset

    lottie["w"] = settings.lottie_format["w"] + settings.ADDITIONAL_PRECOMP_WIDTH # Experimental increase in width and height of precomposition
    lottie["h"] = settings.lottie_format["h"] + settings.ADDITIONAL_PRECOMP_HEIGHT
    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    lottie["bm"] = settings.DEFAULT_BLEND    # This will change in group layer 

    # Return to previous state, when we go outside the group layer
    settings.INSIDE_PRECOMP = prev_state
