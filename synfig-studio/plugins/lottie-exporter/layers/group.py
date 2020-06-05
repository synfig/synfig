# pylint: disable=line-too-long
"""
Store all functions corresponding to group layer in Synfig
"""

import sys
import math
import settings
from common.Param import Param
from common.Canvas import Canvas
from common.Count import Count
from common.misc import get_frame, approximate_equal, get_time
from sources.precomp import add_precomp_asset
from helpers.transform import gen_helpers_transform
from helpers.blendMode import get_blend
from synfig.animation import insert_waypoint_at_frame, to_Synfig_axis
sys.path.append("..")


def gen_layer_group(lottie, layer, idx):
    """
    Will generate a pre composition but has small differences than pre-comp layer used in
    layers/preComp.py
    This function will be used for group layer as well as switch group layer

    Args:
        lottie (dict)               : Lottie format layer will be stored here
        layer (common.Layer.Layer) : Synfig format group/switch layer
        idx   (int)                 : Index of the layer

    Returns:
        (None)
    """
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_PRECOMP_TYPE
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled
    lottie["nm"] = layer.get_description()
    index = Count()

    # Extract parameters
    canvas = Canvas(layer.get_param("canvas"))
    origin = layer.get_param("origin")
    opacity = layer.get_param("amount")
    outline_grow = layer.get_param("outline_grow")
    time_offset = layer.get_param("time_offset")
    time_dilation = layer.get_param("time_dilation")
    transformation = layer.get_param("transformation")
    transform = transformation[0]
    try_par = Param(transform, Param(transformation.get(), layer))
    for child in transform:
        if child.tag == "scale":
            scale = Param(child, try_par)
        elif child.tag == "offset":
            pos = Param(child, try_par)
        elif child.tag == "angle":
            angle = Param(child, try_par)
        elif child.tag == "skew_angle":
            skew = Param(child, try_par)

    outline_grow.animate("real")

    origin.animate("vector")
    anchor = origin

    for layer in canvas.layers:
        if layer.get_type() in settings.TEXT_LAYER:
            settings.TEXT_LAYER_FLAG = True #To indicate text in a group layer

    if settings.TEXT_LAYER_FLAG:
        settings.TEXT_LAYER_FLAG = False
    else:
        anchor.add_offset()

    angle.animate("rotate_layer_angle")

    pos.animate("vector")
    if settings.INSIDE_PRECOMP:
        pos.add_offset()

    scale.animate("group_layer_scale")
    # Generating animation for skew
    skew.animate("rotate_layer_angle")
    # Animating opacity
    opacity.animate("opacity")

    # Reset the animations after adding offset
    anchor.animate("vector", True)
    pos.animate("vector", True)
    # Generate the transform properties here
    gen_helpers_transform(lottie["ks"], pos, anchor, scale, angle, opacity, skew)

    # Store previous states, to be recovered at the end of group layer
    prev_state = settings.INSIDE_PRECOMP
    settings.OUTLINE_GROW.append(outline_grow)    # Storing the outline grow in settings, will be used inside child outlines

    settings.INSIDE_PRECOMP = True

    settings.lottie_format["assets"].append({})
    asset = add_precomp_asset(settings.lottie_format["assets"][-1], canvas, canvas.get_num_layers())
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
    if layer.get_type() == "switch":
        change_opacity_switch(layer, lottie)
    # Change opacity of layers for group layers
    elif layer.get_type() == "group":
        change_opacity_group(layer, lottie)

    # Return to previous state, when we go outside the group layer
    settings.INSIDE_PRECOMP = prev_state
    settings.OUTLINE_GROW.pop()


def change_opacity_group(layer, lottie):
    """
    Will make the opacity of underlying layers 0 according to the layers lying
    inside z range(if it is active)[z-range is non-animatable]

    Args:
        layer (common.Layer.Layer) : Synfig format layer
        lottie (dict)      : Lottie format layer

    Returns:
        (None)
    """
    z_range = layer.get_param("z_range")
    z_range_pos = layer.get_param("z_range_position")
    z_range_depth = layer.get_param("z_range_depth")
    canvas = Canvas(layer.get_param("canvas"))

    for assets in settings.lottie_format["assets"]:
        if assets["id"] == lottie["refId"]:
            root = assets
            break

    # If z-range is non-active (static value)
    if z_range[0].attrib["value"] == "false":
        return

    z_range_pos.animate("real")

    z_range_depth.animate("real")

    z_st, z_en = float('-inf'), float('-inf')
    active_range = [] # Stores the time and change of layers in z-range
    fr = settings.lottie_format["ip"]
    while fr <= settings.lottie_format["op"]:
        pos_val = to_Synfig_axis(z_range_pos.get_value(fr), "real")
        depth_val = to_Synfig_axis(z_range_depth.get_value(fr), "real")
        st, en = math.ceil(pos_val), math.floor(pos_val + depth_val)
        if st > en or en < 0:
            if (fr == settings.lottie_format["ip"]) or (z_st != -1 and z_en != -1):
                z_st, z_en = -1, -1
                active_range.append([fr, z_st, z_en])
        elif (st != z_st) or (en != z_en):
            z_st, z_en = st, en
            active_range.append([fr, z_st, z_en])
        fr += 1

    z_value = 0
    for c_layer in reversed(canvas.get_layer_list()):
        if not c_layer.is_active() or not c_layer.to_render():
            continue
        active_time = set()
        itr = 0
        while itr < len(active_range):
            st, en = active_range[itr][1], active_range[itr][2]
            if z_value <= en and z_value >= st:
                now = active_range[itr][0] / settings.lottie_format["fr"]
                later = get_time_bound("op")
                if itr + 1 < len(active_range):
                    later = active_range[itr + 1][0] / settings.lottie_format["fr"]
                active_time.add((now, later))
            itr += 1
        active_time = sorted(active_time)
        inactive_time = sorted(flip_time(active_time))

        if c_layer.get_type() in set.union(settings.SHAPE_SOLID_LAYER, settings.SOLID_LAYER):
            anim_type = "effects_opacity"
            sw = 1
        elif c_layer.get_type() in set.union(settings.PRE_COMP_LAYER, settings.GROUP_LAYER, settings.IMAGE_LAYER):
            anim_type = "opacity"
            sw = 2
        elif c_layer.get_type() in settings.SHAPE_LAYER:
            anim_type = "opacity"
            sw = 3
            dic = root["layers"][z_value]["shapes"][1]["o"]

        animation = gen_hold_waypoints(inactive_time, c_layer, anim_type)
        animation.animate(anim_type)

        if sw == 1:
            animation.fill_path(root["layers"][z_value]["ef"][0]["ef"][-1], "v")
            # See effects/fill.py: Opacity is the last property, and hence we are using [-1].
            # We should actually search for "opacity", but due to multiple elements with same
            # "ty"(which should not happen), we are using [-1]. "ty" here means type which uniquely
            # identifies the effect in Lottie, but 'horizontal feather', 'vertical feather' and
            # 'opacity' in Lottie have the same type and hence we can not search for "opacity"
            # uniquely
        elif sw == 2:
            animation.fill_path(root["layers"][z_value]["ks"], "o")
        elif sw == 3:
            animation.fill_path(root["layers"][z_value]["shapes"][1], "o")

        z_value += 1



def change_opacity_switch(layer, lottie):
    """
    Will make the opacity of underlying layers 0 according to the active layer

    Args:
        layer (common.Layer.Layer) : Synfig format layer
        lottie (dict)      : Lottie format layer

    Returns:
        (None)
    """
    layer_name = layer.get_param("layer_name")
    canvas = Canvas(layer.get_param("canvas"))

    layer_name.animate_without_path("string")
    for assets in settings.lottie_format["assets"]:
        if assets["id"] == lottie["refId"]:
            root = assets
            break

    it = 0
    for c_layer in reversed(canvas.get_layer_list()):
        if not c_layer.is_active() or not c_layer.to_render():
            continue
        active_time = set()
        description = root["layers"][it]["nm"]

        waypoint_itr = 0
        while waypoint_itr < len(layer_name[0]):
            waypoint = layer_name[0][waypoint_itr]
            l_name = waypoint[0].text
            if (l_name == description) or (l_name is None and it == 0):
                update_time(active_time, layer_name[0], waypoint_itr)
            waypoint_itr += 1

        active_time = sorted(active_time)
        inactive_time = sorted(flip_time(active_time))

        sw = 0  # To decide which if condition to go to
        if c_layer.get_type() in set.union(settings.SHAPE_SOLID_LAYER, settings.SOLID_LAYER):
            anim_type = "effects_opacity"
            sw = 1
        elif c_layer.get_type() in set.union(settings.PRE_COMP_LAYER, settings.GROUP_LAYER, settings.IMAGE_LAYER):
            anim_type = "opacity"
            sw = 2
        elif c_layer.get_type() in settings.SHAPE_LAYER:
            anim_type = "opacity"
            sw = 3

        animation = gen_hold_waypoints(inactive_time, c_layer, anim_type)
        animation.animate(anim_type)

        if sw == 1:
            animation.fill_path(root["layers"][it]["ef"][0]["ef"][-1], "v")
            # See effects/fill.py: Opacity is the last property, and hence we are using [-1].
            # We should actually search for "opacity", but due to multiple elements with same
            # "ty"(which should not happen), we are using [-1]. "ty" here means type which uniquely
            # identifies the effect in Lottie, but 'horizontal feather', 'vertical feather' and
            # 'opacity' in Lottie have the same type and hence we can not search for "opacity"
            # uniquely
        elif sw == 2:
            animation.fill_path(root["layers"][it]["ks"], "o")
        elif sw == 3:
            animation.fill_path(root["layers"][it]["shapes"][1], "o")

        it += 1


def flip_time(time):
    """
    Time will be in a set();
    Example: input: ((2, 3), (4, 5))
             output: ((0, 2), (3, 4), (5, frame_last_time))

    Args:
        time (set) : Range of time is stored in this

    Returns:
        (set) : Flipped/opposite of `time` is returned
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


def gen_hold_waypoints(inactive_time, layer, anim_type):
    """
    Will only be used to modify opacity waypoints, and set zero values where the
    layer is inactive

    Args:
        inactive_time (set) : Range of time when the layer will be inactive
        layer (common.Layer.Layer) : Synfig format layer
        anim_type (str) : Specifies whether it is effects_opacity or opacity (it
                          will effect a factor of 100)

    Returns:
        (common.Param.Param) : Modified opacity animation is returned
    """
    opacity = layer.get_param("amount")

    opacity.animate(anim_type)
    opacity_dict = {}
    opacity_dict["o"] = {}
    opacity.fill_path(opacity_dict, "o")
    opacity_dict = opacity_dict["o"]

    for it in inactive_time:
        # First add waypoints at both points, make it constant interval
        # then remove any in-between waypoints
        first = round(it[0] * settings.lottie_format["fr"])
        second = round(it[1] * settings.lottie_format["fr"])
        insert_waypoint_at_frame(opacity[0], opacity_dict, first, anim_type)
        insert_waypoint_at_frame(opacity[0], opacity_dict, second, anim_type)

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
    Depending on the waypoints, the active time set is updated accordingly

    Args:
        active_time (set) : Stores the active time ranges of the layer till now
        animated (lxml.etree._Element) : Animation in Synfig format
        itr (int) : Position of the waypoint in the animation

    Returns:
        (None)
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

    Args:
        lottie (dict) : Time remapping in Lottie format
        time_offset (common.Param.Param) : Offset for time in Synfig format
        time_dilation (common.Param.Param) : Speed/dilation for time in Synfig format
        idx (itr) : Index of this property in the layer

    Returns:
        (None)
    """
    time_offset.animate("time")

    time_dilation.animate("real")

    fr, lst = settings.lottie_format["ip"], settings.lottie_format["op"]
    lottie["a"] = 1 # Animated
    lottie["ix"] = idx
    lottie["k"] = []

    while fr <= lst:
        lottie["k"].append({})
        gen_dict(lottie["k"][-1], time_offset, time_dilation, fr)
        fr += 1


def gen_dict(lottie, time_offset, time_dilation, fr):
    """
    Generates the constant values for each frame

    Args:
        lottie (dict) : Bezier values will be stored in here
        offset_dict (dict) : Animation of offset in lottie format
        dilation_dict (dict) : Animation of dilation/speed in lottie format
        fr (int) : frame number

    Returns:
        (None)
    """
    lottie["i"], lottie["o"] = {}, {}
    lottie["i"]["x"], lottie["i"]["y"] = [], []
    lottie["o"]["x"], lottie["o"]["y"] = [], []
    lottie["i"]["x"].append(0.5)
    lottie["i"]["y"].append(0.5)
    lottie["o"]["x"].append(0.5)
    lottie["o"]["y"].append(0.5)
    lottie["t"] = fr

    speed_f = to_Synfig_axis(time_dilation.get_value(fr), "real")
    speed_s = to_Synfig_axis(time_dilation.get_value(fr+1), "real")
    first = time_offset.get_value(fr) + (fr/settings.lottie_format["fr"])*speed_f
    second = time_offset.get_value(fr + 1) + ((fr + 1)/settings.lottie_format["fr"])*speed_s
    first = min(max(get_time_bound("ip"), first), get_time_bound("op"))
    second = min(max(get_time_bound("ip"), second), get_time_bound("op"))

    lottie["s"], lottie["e"] = [first], [second]


def get_time_bound(st):
    """
    Returns the extreme values of time in seconds

    Args:
        st (str) : Specifies in-time or out-time

    Returns:
        (float) : Extreme value of time in seconds
    """
    ret = settings.lottie_format[st]
    if st == "op":
        ret -= 1
    ret /= settings.lottie_format["fr"]
    return ret
