"""
Fill this
"""
import sys
import copy
sys.path.append("..")
import settings
from misc import parse_position, change_axis

"""
See this for details: https://github.com/synfig/synfig/blob/master/synfig-core/src/synfig/vector.h#L187
return value: 
    True if p1 < p2
    False if p1 >= p2
"""

"""
	To be completed. Waiting for answer of my query
"""
def clamped_tangent(p1, p2, p3, animated, i):
    # pw -> prev_waypoint, w -> waypoint, nw -> next_waypoint
    pw, w, nw = animated[i-1], animated[i], animated[i+1]
    t1 = float(pw.attrib["time"][:-1]) * settings.lottie_format["fr"]
    t2 = float(w.attrib["time"][:-1]) * settings.lottie_format["fr"]
    t3 = float(nw.attrib["time"][:-1]) * settings.lottie_format["fr"]
    bias = 0.0
    tangent = [0 for i in range(len(p1))]
    pm = []

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
    prev_pos = copy.deepcopy(cur_pos)
    next_pos = parse_position(animated, i + 1)
    after_next_pos = copy.deepcopy(next_pos)

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
    lottie["s"] = change_axis(cur_pos.x, cur_pos.y)
    lottie["e"] = change_axis(next_pos.x, next_pos.y)
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

    ######################### TESTING ########################################
    # iter           next
    # ANY/TCB ------ ANY/ANY
    if cur_get_after == "auto":
        if i >= 1:
            out_val = ((1 - tens) * (1 + bias) * (1 + cont) *\
                       (cur_pos - prev_pos))/2 +\
                       ((1 - tens) * (1 - bias) * (1 - cont) *\
                       (next_pos - cur_pos))/2
        else:
            out_val = next_pos - cur_pos      # t1 = p2 - p1

    # iter           next
    # ANY/LINEAR --- ANY/ANY
    # ANY/EASE   --- ANY/ANY
    if cur_get_after in {"linear", "halt"}:
        out_val = next_pos - cur_pos

    # iter          next
    # ANY/ANY ----- LINEAR/ANY
    # ANY/ANY ----- EASE/ANY
    if next_get_before in {"linear", "halt"}:
        in_val = next_pos - cur_pos

    # iter          next
    # ANY/CLAMPED - ANY/ANY
    if cur_get_after == "clamped":
        if i >= 1:
            out_val = clamped_tangent(prev_pos, cur_pos, next_pos, animated, i) 
        else:
            out_val = next_pos - cur_pos      # t1 = p2 - p1

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
                      (next_pos - cur_pos))/2 +\
                      ((1 - tens1) * (1 - bias1) * (1 + cont1) *\
                      (after_next_pos - next_pos))/2
        else:
            in_val = next_pos - cur_pos      # t2 = p2 - p1
    print(cur_get_after, next_get_before)
    lottie["to"] = out_val.get_list()
    lottie["ti"] = in_val.get_list()

    ##################################################################################################

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
