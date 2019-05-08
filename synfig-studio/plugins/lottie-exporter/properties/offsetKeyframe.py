"""
Fill this
"""
import sys
import copy
sys.path.append("..")
import settings
from misc import parse_position, change_axis, Vector

"""
A Function for testing approximate equality
"""
def isclose(a, b, rel_tol=1e-09, abs_tol=0.0):
    return abs(a-b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)

def clamped_tangent(p1, p2, p3, animated, i):
    # pw -> prev_waypoint, w -> waypoint, nw -> next_waypoint
    pw, w, nw = animated[i-1], animated[i], animated[i+1]
    t1 = float(pw.attrib["time"][:-1]) * settings.lottie_format["fr"]
    t2 = float(w.attrib["time"][:-1]) * settings.lottie_format["fr"]
    t3 = float(nw.attrib["time"][:-1]) * settings.lottie_format["fr"]
    bias = 0.0
    tangent = 0.0
    pm = p1 + (p3 - p1)*(t2 - t1)/(t3 - t1)
    if p3 > p1:
        if p2 >= p3 or p2 <= p1:
            tangent = tangent * 0.0
        else:
            if p2 > pm:
                bias = (pm - p2) / (p3 - pm)
            elif p2 < pm:
                bias = (pm - p2) / (pm - p1)
            else:
                bias = 0.0
            tangent = (p2 - p1) * (1.0 + bias) / 2.0 + (p3 - p2) * (1.0 - bias) / 2.0
    elif p1 > p2:
        if p2 >= p1 or p2 <= p3:
            tangent = tangent * 0.0
        else:
            if p2 > pm:
                bias = (pm - p2) / (pm - p1)
            elif p2 < pm:
                bias = (pm - p2) / (p3 - pm)
            else:
                bias = 0.0
            tangent = (p2 - p1) * (1.0 + bias) / 2.0 + (p3 - p2) * (1.0 - bias) / 2.0
    else:
        tangent = tangent * 0.0
    return tangent

def clamped_vector(p1, p2, p3, animated, i, lottie, ease):
    x_tan = clamped_tangent(p1.x, p2.x, p3.x, animated, i)
    y_tan = clamped_tangent(p1.y, p2.y, p3.y, animated, i)
    if isclose(x_tan, 0.0) or isclose(y_tan, 0.0):
        if ease == "in":
            ease_in(lottie)
        else:
            ease_out(lottie)
    return Vector(clamped_tangent(p1.x, p2.x, p3.x, animated, i), clamped_tangent(p1.y, p2.y, p3.y, animated, i))

def ease_out(lottie):
    lottie["o"]["x"] = settings.OUT_TANGENT_X 
    lottie["o"]["y"] = settings.OUT_TANGENT_Y

def ease_in(lottie):
    lottie["i"]["x"] = settings.IN_TANGENT_X
    lottie["i"]["y"] = settings.IN_TANGENT_Y

def calc_tangent(animated, lottie, i):

    waypoint, next_waypoint = animated[i], animated[i+1]
    cur_get_after, next_get_before = waypoint.attrib["after"], next_waypoint.attrib["before"]
    cur_get_before, next_get_after = waypoint.attrib["before"], next_waypoint.attrib["after"]

    if animated.attrib["type"] == "angle":
        if cur_get_after == "auto":
            cur_get_after = "linear"
        if cur_get_before == "auto":
            cur_get_before = "linear"
        if next_get_before == "auto":
            next_get_before = "linear"
        if next_get_after == "auto":
            next_get_after = "linear"

    # Calculate positions of waypoints
    cur_pos = parse_position(animated, i)
    prev_pos = copy.deepcopy(cur_pos)
    next_pos = parse_position(animated, i + 1)
    after_next_pos = copy.deepcopy(next_pos)

    if i + 2 <= len(animated) - 1:
        after_next_pos = parse_position(animated, i + 2)
    if i - 1 >= 0:
        prev_pos = parse_position(animated, i - 1)

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
            ease = "out"
            out_val = clamped_vector(prev_pos, cur_pos, next_pos, animated, i, lottie, ease)
        else:
            out_val = next_pos - cur_pos      # t1 = p2 - p1

    # iter          next             after_next
    # ANY/ANY ----- CLAMPED/ANY ---- ANY/ANY
    if next_get_before == "clamped":
        if i + 2 <= len(animated) - 1:
            ease = "in"
            in_val = clamped_vector(cur_pos, next_pos, after_next_pos, animated, i + 1, lottie, ease)
        else:
            in_val = next_pos - cur_pos     # t2 = p2 - p1

    # iter              next
    # ANY/CONSTANT ---- ANY/ANY
    # ANY/ANY      ---- CONSTANT/ANY
    if cur_get_after == "constant" or next_get_before == "constant":
        lottie["h"] = 1
        if animated.attrib["type"] == "vector":
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
    return out_val, in_val

def gen_properties_offset_keyframe(curve_list, animated, i):
    """
     Generates the dictionary corresponding to properties/offsetKeyFrame.json
    """
    lottie = curve_list[-1]

    waypoint, next_waypoint = animated[i], animated[i+1]
    cur_get_after, next_get_before = waypoint.attrib["after"], next_waypoint.attrib["before"]
    cur_get_before, next_get_after = waypoint.attrib["before"], next_waypoint.attrib["after"]

    if animated.attrib["type"] == "angle":
        if cur_get_after == "auto":
            cur_get_after = "linear"
        if cur_get_before == "auto":
            cur_get_before = "linear"
        if next_get_before == "auto":
            next_get_before = "linear"
        if next_get_after == "auto":
            next_get_after = "linear"

    # Calculate positions of waypoints
    cur_pos = parse_position(animated, i)
    next_pos = parse_position(animated, i + 1)

    lottie["i"] = {}    # Time bezier curve, not used in synfig
    lottie["o"] = {}    # Time bezier curve, not used in synfig
    lottie["i"]["x"] = 0.5
    lottie["i"]["y"] = 0.5
    lottie["o"]["x"] = 0.5
    lottie["o"]["y"] = 0.5
    if cur_get_after == "halt": # For ease out
        ease_out(lottie)
    if next_get_before == "halt": # For ease in
        ease_in(lottie)
    lottie["t"] = float(waypoint.attrib["time"][:-1]) * settings.lottie_format["fr"]
    lottie["s"] = change_axis(cur_pos.x, cur_pos.y)
    lottie["e"] = change_axis(next_pos.x, next_pos.y)
    lottie["to"] = []
    lottie["ti"] = []

    # Calculating the unchanged tangent
    out_val, in_val = calc_tangent(animated, lottie, i)

    lottie["to"] = out_val.get_list()
    lottie["ti"] = in_val.get_list()

    # TCB/!TCB and list is not empty
    if cur_get_before == "auto" and cur_get_after != "auto" and i > 0:
        curve_list[-2]["ti"] = copy.deepcopy(lottie["to"])
        curve_list[-2]["ti"] = [-item/settings.TANGENT_FACTOR for item in curve_list[-2]["ti"]]
        curve_list[-2]["ti"][1] = -curve_list[-2]["ti"][1]
        if cur_get_after == "halt":
            curve_list[-2]["i"]["x"] = settings.IN_TANGENT_X
            curve_list[-2]["i"]["y"] = settings.IN_TANGENT_Y

    # Lottie and synfig use different tangents SEE DOCUMENTATION
    lottie["ti"] = [-item for item in lottie["ti"]]

    # Lottie tangent length is larger than synfig
    lottie["ti"] = [item/settings.TANGENT_FACTOR for item in lottie["ti"]]
    lottie["to"] = [item/settings.TANGENT_FACTOR for item in lottie["to"]]

    # IMPORTANT to and ti have to be relative
    # The y-axis is different in lottie
    lottie["ti"][1] = -lottie["ti"][1]
    lottie["to"][1] = -lottie["to"][1]
