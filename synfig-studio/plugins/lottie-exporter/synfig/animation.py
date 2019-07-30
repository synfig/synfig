# pylint: disable=line-too-long
"""
Will store all the functions to play with and modify the animations and
waypoints in Synfig format
"""

import sys
import copy
from lxml import etree
import settings
from common.misc import approximate_equal, is_animated, get_frame
from common.Vector import Vector
from helpers.bezier import get_bezier_val
sys.path.append("..")


def gen_dummy_waypoint(non_animated, animated_tag, anim_type, animated_name="anything"):
    """
    Makes a non animated parameter to animated parameter by creating a new dummy
    waypoint with constant animation

    Args:
        non_animated (lxml.etree._Element): Holds the non-animated parameter in Synfig xml format
        animated_tag(str)                : Decides the tag of the animation
        anim_type    (str)                : Decides the animation type

    Returns:
        (lxml.etree._Element) : Updated non-animated parameter, which is now animated
    """
    is_animate = is_animated(non_animated[0])
    if is_animate == 2:
        # If already animated, no need to add waypoints
        # Forcibly set it's animation type to the given anim_type :needed in:->
        # properties/shapePropKeyframe.py #31
        non_animated[0].attrib["type"] = anim_type
        return non_animated
    elif is_animate == 0:
        st = '<{animated_tag} name="{animated_name}"><animated type="{anim_type}"><waypoint time="0s" before="constant" after="constant"></waypoint></animated></{animated_tag}>'
        st = st.format(anim_type=anim_type, animated_name=animated_name, animated_tag=animated_tag)
        root = etree.fromstring(st)
        root[0][0].append(copy.deepcopy(non_animated[0]))
        non_animated = root
    elif is_animate == 1:
        # Forcibly set it's animation type to the given anim_type
        non_animated[0].attrib["type"] = anim_type

        non_animated[0][0].attrib["before"] = non_animated[0][0].attrib["after"] = "constant"

    new_waypoint = copy.deepcopy(non_animated[0][0])
    frame = get_frame(non_animated[0][0])
    frame += 1
    time = frame / settings.lottie_format["fr"]
    time = str(time) + "s"
    new_waypoint.attrib["time"] = time
    non_animated[0].insert(1, new_waypoint)
    return non_animated


def insert_waypoint_at_frame(animated, orig_path, frame, animated_name):
    """
    This function will only insert a waypoint at 'frame' if no waypoint is
    present at that 'frame' already

    Args:
        animated      (lxml.etree._Element): Holds the animation in Synfig xml format
        orig_path     (dict)               : Holds the animation in Lottie format
        frame         (int)                : The frame at which the waypoint is to be inserted
        animated_name (str)                : The name/type of animation

    Returns:
        (None)
    """
    i = 0
    while i < len(animated):
        at_frame = get_frame(animated[i])
        if approximate_equal(frame, at_frame):
            return
        elif frame < at_frame:
            break
        i += 1
    pos = get_vector_at_frame(orig_path, frame)
    pos = to_Synfig_axis(pos, animated_name)

    if i == len(animated):
        new_waypoint = copy.deepcopy(animated[i-1])
    else:
        new_waypoint = copy.deepcopy(animated[i])
    if animated_name == "vector":
        new_waypoint[0][0].text = str(pos[0])
        new_waypoint[0][1].text = str(pos[1])
    else:
        new_waypoint[0].attrib["value"] = str(pos)

    new_waypoint.attrib["time"] = str(frame/settings.lottie_format["fr"]) + "s"
    if i == 0 or i == len(animated):
        # No need of tcb value copy as halt interpolation need to be copied here
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "constant"
    else:
        copy_tcb_average(new_waypoint, animated[i], animated[i-1])
        new_waypoint.attrib["before"] = animated[i-1].attrib["after"]
        new_waypoint.attrib["after"] = animated[i].attrib["before"]
        # If the interval was constant before, then the whole interval should
        # remain constant now also
        if new_waypoint.attrib["before"] == "constant" or new_waypoint.attrib["after"] == "constant":
            new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "constant"
    animated.insert(i, new_waypoint)


def print_animation(*argv):
    """
    Given any animation, b, It prints the animation in pretty way.
    Helpful in debugging

    Args:
        argv (tuple): Holds the animations to be printed
        arg will be of type lxml.etree._Element

    Returns:
        (None)
    """
    print("####################### START ########################")
    for arg in argv:
        a = copy.deepcopy(arg)
        print(etree.tostring(a, method='xml', encoding='utf8', pretty_print=True).decode())
    print("####################### END ##########################")


def to_Synfig_axis(pos, animated_name):
    """
    Converts a Lottie format vector or values into Synfig format vector or
    values

    Args:
        pos             (float | list) : Stores the position/value to be converted to the Synfig format
        animated_name   (str)          : Distinguishes between position and value

    Returns:
        (float) If pos is of type float
        (list)  Else
    """
    if animated_name == "vector":
        pos[0], pos[1] = pos[0] - settings.lottie_format["w"]/2, pos[1] - settings.lottie_format["h"]/2
        pos[1] = -pos[1]
        ret = [x/settings.PIX_PER_UNIT for x in pos]
    elif animated_name == "real":
        ret = pos / settings.PIX_PER_UNIT
    elif animated_name in ["amount", "opacity"]:
        ret = pos / settings.OPACITY_CONSTANT
    elif animated_name == "effects_opacity":
        ret = pos
    elif animated_name == "angle":
        s1 = int(pos / 360)
        t1 = pos % 360
        t1 = (90 - t1) % 360
        t1 = t1 + s1 * 360
        ret = t1
    return ret


def get_first_control_point(interval):
    """
    Returns the first control point of a bezier interval

    Args:
        interval (dict) : Holds one interval of the bezier curve that is two waypoints

    Returns:
        (common.Vector.Vector) If the interval holds position bezier
        (float)       Else : the interval holds value bezier
    """
    if len(interval["s"]) >= 2:
        st = Vector(interval["s"][0], interval["s"][1])
    else:
        st = interval["s"][0]
    return st


def get_last_control_point(interval):
    """
    Returns the last control point of a bezier interval

    Args:
        interval (dict) : Holds one interval of the bezier curve that is two waypoints

    Returns:
        (common.Vector.Vector) If the interval holds position bezier
        (float)       Else : the interval holds value bezier
    """
    if len(interval["e"]) >= 2:
        en = Vector(interval["e"][0], interval["e"][1])
    else:
        en = interval["e"][0]
    return en


def get_control_points(interval):
    """
    Returns all 4 control points of a bezier interval

    Args:
        interval (dict) : Holds one interval of the bezier curve that is two waypoints

    Returns:
        (common.Vector.Vector, common.Vector.Vector, common.Vector.Vector, common.Vector.Vector) If the interval holds position bezier
        (float, float, float, float)       Else : the interval holds value bezier
    """
    # If the interval is for position or vector
    if "to" in interval.keys():
        st = Vector(interval["s"][0], interval["s"][1])
        en = Vector(interval["e"][0], interval["e"][1])
        to = Vector(interval["synfig_to"][0], interval["synfig_to"][1])
        ti = Vector(interval["synfig_ti"][0], interval["synfig_ti"][1])

    # If the interval is for real values
    else:
        st = interval["s"][0]
        en = interval["e"][0]
        to = interval["synfig_o"][0]
        ti = interval["synfig_i"][0]
    return st, to, ti, en


def get_vector_at_frame(path, t):
    """
    Given 'path' in lottie format and t(in frames), this function returns the
    vector or real value at frame t depending on the type of path supplied to it

    Args:
        path (dict): Contains the bezier curve in Lottie JSON format
        t    (int) : Contains the frame at which the vector/value is requested

    Returns:
        (list)  If a positional bezier is queried
        (float) If a value bezier is queried
    """
    keyfr = path["k"]
    # Find the interval in which it lies
    i = 0
    while i < len(keyfr):
        t_key = keyfr[i]["t"]
        if t < t_key:
            break
        i += 1
    i -= 1
    if i < 0:
        pos = get_first_control_point(keyfr[0])
    elif i < len(keyfr) - 1:
        # If hold interpolation
        if 'h' in keyfr[i].keys():
            pos = get_first_control_point(keyfr[i])
        else:
            st, to, ti, en = get_control_points(keyfr[i])
            this_fr = keyfr[i]["t"]
            next_fr = keyfr[i+1]["t"]
            percent = (t - this_fr) / (next_fr - this_fr)
            pos = get_bezier_val(st, st + to, en - ti, en, percent)
    elif i >= len(keyfr) - 1:
        pos = get_last_control_point(keyfr[-2])

    if isinstance(pos, Vector):
        return pos.get_list()
    return pos


def get_bool_at_frame(anim, frame):
    """
    Calculates the boolean value at a given frame, given a boolean animation

    Args:
        anim  (lxml.etree._Element): Boolean animation
        frame (int)                : Frame at which the value is to be calculated

    Returns:
        (bool) : True if the value is "true" at that frame
               : False otherwise
    """
    i = 0
    while i < len(anim):
        cur_fr = get_frame(anim[i])
        if frame <= cur_fr:
            break
        i += 1
    if i == 0:
        val = anim[0][0].attrib["value"]
    elif i < len(anim):
        # frame lies between i-1 and i'th value of animation
        prev = anim[i-1][0].attrib["value"]
        cur = anim[i][0].attrib["value"]
        cur_frame = get_frame(anim[i])
        if frame == cur_frame:
            val = cur
        else:
            if prev == "true" and cur == "false":
                val = prev
            elif prev == "false" and cur == "true":
                val = cur
            elif prev == "true" and cur == "true":
                val = cur
            elif prev == "false" and cur == "false":
                val = cur
    elif i >= len(anim):
        val = anim[-1][0].attrib["value"]

    if val == "false":
        val = False
    else:
        val = True
    return val


def waypoint_at_frame(anim, frame):
    """
    Returns true if a waypoint is present at 'frame'
    """
    for waypoint in anim:
        fr = get_frame(waypoint)
        if fr == frame:
            return True
    return False

def modify_bool_animation(anim):
    """
    Inserts waypoints at such frames so that the animation is similar to that in
    lottie
    """
    i = 0
    while i < len(anim):
        cur_fr = get_frame(anim[i])
        val_now = get_bool_at_frame(anim, cur_fr)
        # Check at one frame less and one frame more: only cases
        if cur_fr - 1 >= settings.lottie_format["ip"] and not waypoint_at_frame(anim, cur_fr-1):
            val_before = get_bool_at_frame(anim, cur_fr - 1)
            if val_now != val_before:
                new = copy.deepcopy(anim[i])
                new.attrib["time"] = str((cur_fr - 1)/settings.lottie_format["fr"]) + "s"
                if val_before:
                    new[0].attrib["value"] = "true"
                else:
                    new[0].attrib["value"] = "false"
                anim.insert(i, new)
                i += 1
        if cur_fr + 1 <= settings.lottie_format["op"] and not waypoint_at_frame(anim, cur_fr + 1):
            val_after = get_bool_at_frame(anim, cur_fr + 1)
            if val_after != val_now:
                new = copy.deepcopy(anim[i])
                new.attrib["time"] = str((cur_fr + 1)/settings.lottie_format["fr"]) + "s"
                if val_after:
                    new[0].attrib["value"] = "true"
                else:
                    new[0].attrib["value"] = "false"
                anim.insert(i+1, new)
                i += 1

        i += 1

    # Make the animation constant
    for waypoint in anim:
        waypoint.attrib["before"] = waypoint.attrib["after"] = "constant"




def get_animated_time_list(child, time_list):
    """
    Appends all the frames corresponding to the waypoints in the
    animated(child[0]) list, in time_list

    Args:
        child     (lxml.etree._Element) : Parent Element of animation
        time_list (set)                 : Will store all the frames at which waypoints are present

    Returns:
        (None)
    """
    animated = child[0]
    is_animate = is_animated(animated)
    if is_animate in {0, 1}:
        return
    for waypoint in animated:
        frame = get_frame(waypoint)
        time_list.add(frame)


def copy_tcb_average(new_waypoint, waypoint, next_waypoint):
    """
    Copies the average of TCB values of waypoint and next_waypoint to
    'new_waypoint'
    This is just a way around to determine TCB values at in-between intervals,
    where new waypoints are introduced. Technically we will first calculate the
    tangents at those new waypoints and then calculate any random TCB values
    from those tangents: IMPROVEMENT

    Args:
        new_waypoint  (lxml.etree._Element) : Waypoint at which the values will be averaged
        waypoint      (lxml.etree._Element) : Waypoint contributing in average
        next_waypoint (lxml.etree._Element) : Waypoint contributing in average

    Returns:
        (None)
    """
    tens, bias, cont = 0, 0, 0      # default values
    tens1, bias1, cont1 = 0, 0, 0   # default values
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
    f_tens, f_bias, f_cont = (tens + tens1) / 2, (bias + bias1) / 2, (cont + cont1) / 2
    new_waypoint.attrib["tension"] = str(f_tens)
    new_waypoint.attrib["continuity"] = str(f_cont)
    new_waypoint.attrib["bias"] = str(f_bias)


def copy_tcb(new_waypoint, waypoint):
    """
    Copies the tension, bias and continuity values from 'waypoint' to
    'new_waypoint'

    Args:
        new_waypoint (lxml.etree._Element) : Waypoint at which the values will be copied
        waypoint     (lxml.etree._Element) : Waypoint to be copied from

    Returns:
        (None)
    """
    tens, bias, cont = 0, 0, 0      # default values
    if "tension" in waypoint.keys():
        tens = float(waypoint.attrib["tension"])
    if "continuity" in waypoint.keys():
        cont = float(waypoint.attrib["continuity"])
    if "bias" in waypoint.keys():
        bias = float(waypoint.attrib["bias"])
    new_waypoint.attrib["tension"] = str(tens)
    new_waypoint.attrib["continuity"] = str(cont)
    new_waypoint.attrib["bias"] = str(bias)
