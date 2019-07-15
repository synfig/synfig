# pylint: disable=line-too-long
"""
Store all functions corresponding to group layer in Synfig
"""

import sys
import copy
from lxml import etree
import settings
from misc import get_frame, approximate_equal, get_time,Count, set_layer_desc
from sources.precomp import add_precomp_asset
from helpers.transform import gen_helpers_transform
from helpers.blendMode import get_blend
from synfig.animation import insert_waypoint_at_frame, to_Synfig_axis, print_animation, gen_dummy_waypoint, get_vector_at_frame
import synfig.group as group
from properties.shapePropKeyframe.helper import append_path
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")


def gen_layer_group(lottie, layer, idx):
    """
    Will generate a pre composition but has small differences than pre-comp layer used in
    layers/preComp.py
    This function will be used for group layer as well as switch group layer
    """
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_PRECOMP_TYPE
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled
    set_layer_desc(layer, settings.LAYER_PRECOMP_NAME + str(idx), lottie)
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
            elif chld.attrib["name"] == "time_dilation":
                time_dilation = chld

    outline_grow = gen_dummy_waypoint(outline_grow, "param", "real")
    append_path(outline_grow[0], outline_grow, "outline_grow_path")

    origin = gen_dummy_waypoint(origin, "param", "vector")
    anchor = origin
    group.update_pos(anchor)

    angle = gen_dummy_waypoint(angle, "angle", "rotate_layer_angle")

    pos = gen_dummy_waypoint(pos, "offset", "vector")
    if settings.INSIDE_PRECOMP:
        group.update_pos(pos)

    scale = gen_dummy_waypoint(scale, "scale", "group_layer_scale")
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

    # Time offset and speed
    lottie["tm"] = {}
    gen_time_remap(lottie["tm"], time_offset, time_dilation, index.inc())

    # Change opacity of layers for switch-group layers
    if layer.attrib["type"] == "switch":
        change_opacity(layer, lottie)

    # Return to previous state, when we go outside the group layer
    settings.INSIDE_PRECOMP = prev_state
    settings.OUTLINE_GROW.pop()


def change_opacity(layer, lottie):
    """
    Will make the opacity of underlying layers 0 according to the active layer
    """
    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "layer_name":
                layer_name = chld 
            elif chld.attrib["name"] == "canvas":
                canvas = chld

    layer_name = gen_dummy_waypoint(layer_name, "param", "string", "layer_name")
    for assets in settings.lottie_format["assets"]:
        if assets["id"] == lottie["refId"]:
            root = assets
            break

    it = 0
    for c_layer in reversed(canvas[0]):
        active_time = set()
        description = root["layers"][it]["nm"]

        waypoint_itr = 0
        while waypoint_itr < len(layer_name[0]):
            waypoint = layer_name[0][waypoint_itr]
            l_name = waypoint[0].text
            if (l_name == description) or (l_name == None and it == 0):
                update_time(active_time, layer_name[0], waypoint_itr)
            waypoint_itr += 1

        active_time = sorted(active_time)
        deactive_time = sorted(flip_time(active_time))

        if c_layer.attrib["type"] in set.union(settings.SHAPE_SOLID_LAYER, settings.SOLID_LAYER):
            anim_type = "effects_opacity"
            dic = root["layers"][it]["ef"][0]["ef"][-1]["v"]
        elif c_layer.attrib["type"] in set.union(settings.PRE_COMP_LAYER, settings.GROUP_LAYER, settings.IMAGE_LAYER):
            anim_type = "opacity"
            dic = root["layers"][it]["ks"]["o"]
        elif c_layer.attrib["type"] in settings.SHAPE_LAYER:
            anim_type = "opacity"
            dic = root["layers"][it]["shapes"][1]["o"]

        animation = gen_hold_waypoints(deactive_time, c_layer, anim_type)
        gen_value_Keyframed(dic, animation[0], 0)

        it += 1


def flip_time(time):
    """
    time  will be set();
    Example: input: ((2, 3), (4, 5))
             output: ((0, 2), (3, 4), (5, frame_last_time))
    """
    ret = set()
    last = settings.lottie_format["op"]/settings.lottie_format["fr"]
    z = 0
    for it in time:
        if (not approximate_equal(z, it[0])) and (not approximate_equal(it[0], it[1])):
            ret.add((z, it[0]))
        z = it[1]
    if not approximate_equal(z, last):
        ret.add((z, last))
    return ret


def gen_hold_waypoints(deactive_time, layer, anim_type):
    """
    Will only be used to modify opacity waypoints
    """
    for chld in layer:
        if chld.tag == "param" and chld.attrib["name"] == "amount":
            opacity = chld

    opacity = gen_dummy_waypoint(copy.deepcopy(opacity), "param", anim_type, "amount")
    opacity_dict = {}
    gen_value_Keyframed(opacity_dict, opacity[0], 0)

    for it in deactive_time:
        # First add waypoints at both points, make it constant interval
        # then remove any in-between waypoints
        first = round(it[0] * settings.lottie_format["fr"])
        second = round(it[1] * settings.lottie_format["fr"])
        insert_waypoint_at_frame(opacity[0], opacity_dict, first, "effects_opacity")
        insert_waypoint_at_frame(opacity[0], opacity_dict, second, "effects_opacity")

        # Making it a constant interval
        for waypoint in opacity[0]:
            if approximate_equal(get_frame(waypoint), first):
                st_waypoint = waypoint
                break
        st_waypoint.attrib["after"] = "constant"
        st_waypoint[0].attrib["value"] = str(0)

        # removing the in between waypoints
        for waypoint in opacity[0]:
            this_frame = get_frame(waypoint)
            if (not approximate_equal(this_frame, first)) and \
               (not approximate_equal(this_frame, second)) and \
               (this_frame > first and this_frame < second):
                waypoint.getparent().remove(waypoint)

    return opacity


def update_time(active_time, animated, itr):
    """
    """
    # Tuples will be added to the set
    first = get_time(animated[itr])
    if itr == 0:
        first = 0   # can use settings.lf["ip"]
    second = settings.lottie_format["op"]/settings.lottie_format["fr"]
    if itr + 1 <= len(animated) - 1:
        second = get_time(animated[itr+1])
    active_time.add((first, second))
    

def gen_time_remap(lottie, time_offset, time_dilation, idx):
    """
    Time offset will be converted to time remap here
    Currently time remapping will be done for each frame, but this function can
    be intelligently written only for some particular frames,hence reducing the
    space
    """
    offset_dict = {}
    time_offset = gen_dummy_waypoint(time_offset, "param", "time")
    gen_value_Keyframed(offset_dict, time_offset[0], 0)

    dilation_dict = {}
    time_dilation = gen_dummy_waypoint(time_dilation, "param", "real")
    gen_value_Keyframed(dilation_dict, time_dilation[0], 0)

    fr, lst = settings.lottie_format["ip"], settings.lottie_format["op"]
    lottie["a"] = 1 # Animated
    lottie["ix"] = idx
    lottie["k"] = []

    while fr <= lst:
        lottie["k"].append({}) 
        gen_dict(lottie["k"][-1], offset_dict, dilation_dict, fr)
        fr += 1


def gen_dict(lottie, offset_dict, dilation_dict, fr):
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

    speed_f = to_Synfig_axis(get_vector_at_frame(dilation_dict, fr), "real")
    speed_s = to_Synfig_axis(get_vector_at_frame(dilation_dict, fr + 1), "real")
    first = get_vector_at_frame(offset_dict, fr) + (fr/settings.lottie_format["fr"])*speed_f
    second = get_vector_at_frame(offset_dict, fr + 1) + ((fr + 1)/settings.lottie_format["fr"])*speed_s
    first = min(max(get_time_bound("ip"), first), get_time_bound("op"))
    second = min(max(get_time_bound("ip"), second), get_time_bound("op"))

    lottie["s"], lottie["e"] = [first], [second]


def get_time_bound(st):
    ret = settings.lottie_format[st]
    ret /= settings.lottie_format["fr"]
    return ret
