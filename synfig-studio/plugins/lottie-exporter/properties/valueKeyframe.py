"""
Implements function required for generating tangent between two value key frames
"""

import sys
import random
import settings
from misc import parse_position
from properties.offsetKeyframe import calc_tangent
sys.path.append("../")


def normalize_tangents(cur_pos, next_pos, t_in, t_out):
    """
    Converts the tangents from arbitrary range to the range of 0-1
    """
    # time_scale means converting time(on x-axis) to 0-1 range
    time_scale = next_pos.val2 - cur_pos.val2

    # value_scale -> converting value(on y-axis to 0-1 range)
    value_scale = next_pos.val1 - cur_pos.val1

    # If ever the value scale equals to absolute zero, randomly add or subtract
    # 1e-9 to it, in order to avoid division by zero
    # This difference in value is not caught by bare human eyes, so should not effect
    if value_scale == 0.0:
        bias = 1 if random.random() < 0.5 else -1
        bias *= 1e-9
        value_scale += bias

    time_diff = cur_pos.val2 / time_scale
    value_diff = cur_pos.val1 / value_scale

    t_out["x"][0] += cur_pos.val2     # The tangents were relative to cur_pos, now relative to 0
    t_out["y"][0] += cur_pos.val1

    # -ve sign because lottie and synfig use opposite "in" tangents
    t_in["x"][0] = next_pos.val2 - t_in["x"][0]
    t_in["y"][0] = next_pos.val1 - t_in["y"][0]

    # abs is put in order to deal with decreasing value
    t_out["x"][0] = abs(t_out["x"][0] / time_scale - time_diff)
    t_out["y"][0] = abs(t_out["y"][0] / value_scale - value_diff)

    t_in["x"][0] = abs(t_in["x"][0] / time_scale - time_diff)
    t_in["y"][0] = abs(t_in["y"][0] / value_scale - value_diff)


def gen_value_Keyframe(curve_list, animated, i):
    """
    Generates the dictionary corresponding to properties/valueKeyframe.json in lottie
    documentation
    """
    lottie = curve_list[-1]
    waypoint, next_waypoint = animated[i], animated[i+1]
    cur_get_after, next_get_before = waypoint.attrib["after"], next_waypoint.attrib["before"]
    cur_get_before, next_get_after = waypoint.attrib["before"], next_waypoint.attrib["after"]
    # Calculate positions of waypoints
    if animated.attrib["type"] == "angle":
        if cur_get_after == "auto":
            cur_get_after = "linear"
        if cur_get_before == "auto":
            cur_get_before = "linear"
        if next_get_before == "auto":
            next_get_before = "linear"
        if next_get_after == "auto":
            next_get_after = "linear"

    # Synfig only supports constant interpolations for points
    if animated.attrib["type"] == "points":
        cur_get_after = "constant"
        cur_get_before = "constant"
        next_get_after = "constant"
        next_get_before = "constant"

    # After effects only supports linear,ease-in,ease-out and constant interpolations for color
    ##### No support for TCB and clamped interpolations in color is there yet #####
    if animated.attrib["type"] == "color":
        if cur_get_after in {"auto", "clamped"}:
            cur_get_after = "linear"
        if cur_get_before in {"auto", "clamped"}:
            cur_get_before = "linear"
        if next_get_before in {"auto", "clamped"}:
            next_get_before = "linear"
        if next_get_after in {"auto", "clamped"}:
            next_get_after = "linear"

    cur_pos = parse_position(animated, i)
    next_pos = parse_position(animated, i + 1)

    lottie["t"] = float(waypoint.attrib["time"][:-1]) * settings.lottie_format["fr"]
    lottie["s"] = cur_pos.get_val()
    lottie["e"] = next_pos.get_val()

    lottie["i"] = {}
    lottie["o"] = {}

    try:
        out_val, in_val = calc_tangent(animated, lottie, i)
    except Exception as excep:
        # That means halt/constant interval
        return excep

    set_tangents(out_val, in_val, cur_pos, next_pos, lottie, animated)

    if cur_get_after == "halt": # For ease out
        lottie["o"]["x"][0] = settings.OUT_TANGENT_X
        lottie["o"]["y"][0] = settings.OUT_TANGENT_Y
    if next_get_before == "halt": # For ease in
        lottie["i"]["x"][0] = settings.IN_TANGENT_X
        lottie["i"]["y"][0] = settings.IN_TANGENT_Y

    # TCB/!TCB and list is not empty
    if cur_get_before == "auto" and cur_get_after != "auto" and i > 0:

        # need value for previous tangents
        # It may be helpful to store them somewhere
        prev_ov, prev_iv = calc_tangent(animated, curve_list[-2], i - 1)
        prev_iv = out_val
        set_tangents(prev_ov, prev_iv, parse_position(animated, i-1), cur_pos, curve_list[-2], animated)
        if cur_get_after == "halt":
            curve_list[-2]["i"]["x"][0] = settings.IN_TANGENT_X
            curve_list[-2]["i"]["y"][0] = settings.IN_TANGENT_Y


def set_tangents(out_val, in_val, cur_pos, next_pos, lottie, animated):
    """
    To set the tangents as required by the lottie format for value waypoints
    """
    out_lst = out_val.get_list()
    in_lst = in_val.get_list()

    lottie["i"]["x"] = [in_lst[1]]    # x-axis represents time
    lottie["i"]["y"] = [in_lst[0]]    # y-axis represents animation progress
    lottie["o"]["x"] = [out_lst[1]]
    lottie["o"]["y"] = [out_lst[0]]

    # Lottie tangent length is larger than synfig
    lottie["i"]["x"][0] /= settings.TANGENT_FACTOR
    lottie["i"]["y"][0] /= settings.TANGENT_FACTOR
    lottie["o"]["x"][0] /= settings.TANGENT_FACTOR
    lottie["o"]["y"][0] /= settings.TANGENT_FACTOR

    # Keeping original Synfig format tangents in dictionary
    lottie["synfig_i"] = [lottie["i"]["y"][0]]
    lottie["synfig_o"] = [lottie["o"]["y"][0]]

    # If type is color, the tangents are already normalized
    if animated.attrib["type"] != "color":
        normalize_tangents(cur_pos, next_pos, lottie["i"], lottie["o"])
