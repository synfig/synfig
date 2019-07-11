# pylint: disable=line-too-long
"""
Store all functions corresponding to group layer in Synfig
"""

import sys
import copy
import settings
from misc import Count
from sources.precomp import add_precomp_asset
from helpers.transform import gen_helpers_transform
from helpers.blendMode import get_blend
from synfig.animation import print_animation, gen_dummy_waypoint, get_vector_at_frame
import synfig.group as group
from properties.shapePropKeyframe.helper import append_path
from properties.valueKeyframed import gen_value_Keyframed
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
    index = Count()

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
            elif chld.attrib["name"] == "amount":
                opacity = chld
            elif chld.attrib["name"] == "outline_grow":
                outline_grow = chld
            elif chld.attrib["name"] == "time_offset":
                time_offset = chld

    outline_grow = gen_dummy_waypoint(outline_grow, "param", "real")
    append_path(outline_grow[0], outline_grow, "outline_grow_path")

    origin = gen_dummy_waypoint(origin, "param", "vector")
    anchor = origin
    group.update_pos(anchor)

    angle = gen_dummy_waypoint(angle, "angle", "angle")
    angle[0].attrib["type"] = "rotate_layer_angle"

    pos = gen_dummy_waypoint(pos, "offset", "vector")
    if settings.INSIDE_PRECOMP:
        group.update_pos(pos)

    scale = gen_dummy_waypoint(scale, "scale", "vector")
    scale[0].attrib["type"] = "group_layer_scale"
    # Generate the transform properties here
    gen_helpers_transform(lottie["ks"], layer, pos[0], anchor[0], scale[0], angle[0], opacity[0])

    # Store previous states, to be recovered at the end of group layer
    prev_state = settings.INSIDE_PRECOMP

    settings.OUTLINE_GROW.append(outline_grow)
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
    get_blend(lottie, layer)

    # Time offset
    lottie["tm"] = {}
    gen_time_remap(lottie["tm"], time_offset, index.inc())

    # Return to previous state, when we go outside the group layer
    settings.INSIDE_PRECOMP = prev_state
    settings.OUTLINE_GROW.pop()

def gen_time_remap(lottie, time_offset, idx):
    """
    Time offset will be converted to time remap here
    Currently time remapping will be done for each frame, but this function can
    be intelligently written only for some particular frames,hence reducing the
    space
    """
    temp = {}
    time_offset = gen_dummy_waypoint(time_offset, "param", "time")
    time_offset[0].attrib["type"] = "time"

    gen_value_Keyframed(temp, time_offset[0], 0)

    fr, lst = settings.lottie_format["ip"], settings.lottie_format["op"]
    lottie["a"] = 1 # Animated
    lottie["ix"] = idx
    lottie["k"] = []

    while fr <= lst:
        lottie["k"].append({}) 
        gen_dict(lottie["k"][-1], temp, fr)
        fr += 1


def gen_dict(lottie, temp, fr):
    """
    Generates the constant values for each frame
    """
    lottie["i"], lottie["o"] = {}, {}
    lottie["i"]["x"], lottie["i"]["y"] = [], []
    lottie["o"]["x"], lottie["o"]["y"] = [], []
    lottie["i"]["x"].append(0.5)
    lottie["i"]["y"].append(0.5)
    lottie["o"]["x"].append(0.5)
    lottie["o"]["y"].append(0.5)
    lottie["t"] = fr

    first = get_vector_at_frame(temp, fr) + fr/settings.lottie_format["fr"]
    second = get_vector_at_frame(temp, fr + 1) + (fr + 1)/settings.lottie_format["fr"]
    first = min(max(get_time_bound("ip"), first), get_time_bound("op"))
    second = min(max(get_time_bound("ip"), second), get_time_bound("op"))

    lottie["s"], lottie["e"] = [first], [second]

def get_time_bound(st):
    ret = settings.lottie_format[st]
    ret /= settings.lottie_format["fr"]
    return ret
