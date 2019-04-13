"""
Fill this
"""
import sys
sys.path.append("..")
import settings
from misc import parse_position, change_axis


def gen_properties_offset_keyframe(curve_list, animated, i):
    """
     Generates the dictionary corresponding to properties/offsetKeyFrame.json
    """
    lottie = curve_list[-1]

    waypoint, next_waypoint = animated[i], animated[i+1]
    cur_get_after, next_get_before = waypoint.attrib["after"], next_waypoint.attrib["before"]
    cur_get_before, next_get_after = waypoint.attrib["before"], next_waypoint.attrib["after"]
    # Calculate positions of waypoints
    cur_pos = parse_position(animated, i)
    prev_pos = cur_pos
    next_pos = parse_position(animated, i + 1)
    after_next_pos = next_pos
    if i + 2 <= len(animated) - 1:
        after_next_pos = parse_position(animated, i + 2)
    if i - 1 >= 0:
        prev_pos = parse_position(animated, i - 1)

    lottie["i"] = {}    # Time bezier curve, not used in synfig
    lottie["o"] = {}    # Time bezier curve, not used in synfig
    lottie["i"]["x"] = 0.5
    lottie["i"]["y"] = 0.5
    lottie["o"]["x"] = 0.5
    lottie["o"]["y"] = 0.5
    if cur_get_after == "halt": # For ease out
        lottie["o"]["x"] = 0.3
        lottie["o"]["y"] = 0
    if next_get_before == "halt": # For ease in
        lottie["i"]["x"] = 0.7
        lottie["i"]["y"] = 1
    lottie["t"] = float(waypoint.attrib["time"][:-1]) * settings.lottie_format["fr"]
    lottie["s"] = change_axis(cur_pos[0], cur_pos[1])
    lottie["e"] = change_axis(next_pos[0], next_pos[1])
    tens, bias, cont = 0, 0, 0   # default values
    tens1, bias1, cont1 = 0, 0, 0
    if "tension" in waypoint.keys():
        tens = float(waypoint.attrib["tension"])
    if "continuity" in waypoint.keys():
        cont = float(waypoint.attrib["continuity"])
    if "bias" in waypoint.keys():
        bias = float(waypoint.attrib["bias"])
    if "tension" in next_waypoint.keys():
        tens1 = float(next_waypoint.attrib["tension"])
    if "continuity" in next_waypoint.keys():
        cont1 = float(next_waypoint.attrib["continuity"])
    if "bias" in next_waypoint.keys():
        bias1 = float(next_waypoint.attrib["bias"])
    lottie["to"] = []
    lottie["ti"] = []
    for dim in range(len(cur_pos)):
        # iter           next
        # ANY/TCB ------ ANY/ANY
        if cur_get_after == "auto":
            if i >= 1:
                out_val = ((1 - tens) * (1 + bias) * (1 + cont) *\
                           (cur_pos[dim] - prev_pos[dim]))/2 +\
                           ((1 - tens) * (1 - bias) * (1 - cont) *\
                           (next_pos[dim] - cur_pos[dim]))/2
            else:
                out_val = next_pos[dim] - cur_pos[dim]      # t1 = p2 - p1

        # iter           next
        # ANY/LINEAR --- ANY/ANY
        # ANY/EASE   --- ANY/ANY
        if cur_get_after in {"linear", "halt"}:
            out_val = next_pos[dim] - cur_pos[dim]

        # iter          next
        # ANY/ANY ----- LINEAR/ANY
        # ANY/ANY ----- EASE/ANY
        if next_get_before in {"linear", "halt"}:
            in_val = next_pos[dim] - cur_pos[dim]

        # iter              next
        # ANY/CONSTANT ---- ANY/ANY
        # ANY/ANY      ---- CONSTANT/ANY
        if cur_get_after == "constant" or next_get_before == "constant":
            lottie["h"] = 1
            del lottie["to"], lottie["ti"]
            del lottie["i"], lottie["o"]
            # "e" is not needed, but is still not deleted as it is of use in the last iteration of animation
            # del lottie["e"]
            return

        # iter           next           after_next
        # ANY/ANY ------ TCB/ANY ------ ANY/ANY
        if next_get_before == "auto":
            if i + 2 <= len(animated) - 1:
                in_val = ((1 - tens1) * (1 + bias1) * (1 - cont1) *\
                          (next_pos[dim] - cur_pos[dim]))/2 +\
                          ((1 - tens1) * (1 - bias1) * (1 + cont1) *\
                          (after_next_pos[dim] - next_pos[dim]))/2
            else:
                in_val = next_pos[dim] - cur_pos[dim]       # t2 = p2 - p1
        lottie["to"].append(out_val)
        lottie["ti"].append(in_val)

    # TCB/!TCB and list is not empty
    if cur_get_before == "auto" and cur_get_after != "auto" and i > 0:
        curve_list[-2]["ti"] = lottie["to"]
        curve_list[-2]["ti"] = [-item/settings.TANGENT_IN_FACTOR for item in curve_list[-2]["ti"]]
        curve_list[-2]["ti"][1] = -curve_list[-2]["ti"][1]
        if cur_get_after == "halt":
            curve_list[-2]["i"]["x"] = 0.7
            curve_list[-2]["i"]["y"] = 1

    # Lottie and synfig use different tangents SEE DOCUMENTATION
    lottie["ti"] = [-item for item in lottie["ti"]]

    # Lottie tangent length is larger than synfig
    lottie["ti"] = [item/settings.TANGENT_IN_FACTOR for item in lottie["ti"]]
    lottie["to"] = [item/settings.TANGENT_OUT_FACTOR for item in lottie["to"]]

    # IMPORTANT to and ti have to be relative
    # The y-axis is different in lottie
    lottie["ti"][1] = -lottie["ti"][1]
    lottie["to"][1] = -lottie["to"][1]
