"""
Fill this
"""
import sys
import settings
sys.path.append("../")
from misc import parse_position
from properties.offsetKeyframe import ease_in, ease_out, calc_tangent

def normalize_tangents(cur_pos, next_pos, t_in, t_out):
    time_scale = next_pos.y - cur_pos.y    # time_scale means converting time(on x-axis) to 0-1 range
    value_scale = next_pos.x - cur_pos.x   # value_scale -> converting value(on y-axis to 0-1 range)

    time_diff = cur_pos.y / time_scale
    value_diff = cur_pos.x / value_scale

    t_out["x"][0] += cur_pos.y     # The tangents were relative to cur_pos, now relative to 0
    t_out["y"][0] += cur_pos.x

    t_in["x"][0] = next_pos.y - t_in["x"][0]     # -ve sign because lottie and synfig use opposite in tangents
    t_in["y"][0] = next_pos.x - t_in["y"][0]

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

    cur_pos = parse_position(animated, i)
    next_pos = parse_position(animated, i + 1)

    lottie["t"] = float(waypoint.attrib["time"][:-1]) * settings.lottie_format["fr"]
    lottie["s"] = [cur_pos.x]
    lottie["e"] = [next_pos.x]

    lottie["i"] = {}    
    lottie["o"] = {}    

    try:
        out_val, in_val = calc_tangent(animated, lottie, i)
    except:
        # That means halt/constant interval
        return
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

    normalize_tangents(cur_pos, next_pos, lottie["i"], lottie["o"])

    """
    Need to confirm whether to put a -ve sign for in tangent(relative)
    Ans:
    YES, this negative sign is put in the normalize tangents part
    """

    if cur_get_after == "halt": # For ease out
        lottie["o"]["x"][0] = settings.OUT_TANGENT_X 
        lottie["o"]["y"][0] = settings.OUT_TANGENT_Y
    if next_get_before == "halt": # For ease in
        lottie["i"]["x"][0] = settings.IN_TANGENT_X
        lottie["i"]["y"][0] = settings.IN_TANGENT_Y

"""
NOT SURE IF NEEDED, Need to check with konstantin
    # TCB/!TCB and list is not empty
    if cur_get_before == "auto" and cur_get_after != "auto" and i > 0:
"""
