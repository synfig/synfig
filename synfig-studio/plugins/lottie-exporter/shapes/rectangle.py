"""
Will store all functions needed to generate the rectangle layer in lottie
"""

import sys
import copy
from lxml import etree
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis, Vector, get_time, get_frame, get_vector, set_vector
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from helpers.bezier import get_bezier_time, get_bezier_val
sys.path.append("..")


def get_child_value(is_animate, child, what_type):
    """
    Depending upon the is_animate type, value of the position or other
    parameters are extracted by this function
    """
    if what_type == "position":
        if is_animate == 0:
            x_val, y_val = float(child[0][0].text), float(child[0][1].text)
        elif is_animate == 1:
            x_val, y_val = float(child[0][0][0][0].text), float(child[0][0][0][1].text)
        x_val *= settings.PIX_PER_UNIT
        y_val *= settings.PIX_PER_UNIT
        return x_val, y_val
    elif what_type == "value":
        if is_animate == 0:
            val = float(child[0].attrib["value"])
        elif is_animate == 1:
            val = float(child[0][0][0].attrib["value"])
        return val


def gen_shapes_rectangle(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/rect.json
    """
    index = Count()
    lottie["ty"] = "rc"     # Type: rectangle
    lottie["p"] = {}        # Position of rectangle
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of rectangle
    lottie["ix"] = idx      # setting the index
    lottie["r"] = {}        # Rounded corners of rectangle
    points = {}

    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "point1":
                points["1"] = child # Store address of child here

            elif child.attrib["name"] == "point2":
                points["2"] = child # Store address of child here

            elif child.attrib["name"] == "expand":
                expand_animate = is_animated(child[0])
                param_expand = child

            elif child.attrib["name"] == "bevel":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_value_Keyframed(lottie["r"], child[0], index.inc())
                else:
                    bevel = get_child_value(is_animate, child, "value")
                    bevel *= settings.PIX_PER_UNIT
                    gen_properties_value(lottie["r"],
                                         bevel,
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
    p1_animate = is_animated(points["1"][0])
    p2_animate = is_animated(points["2"][0])

    # If expand parameter is not animated
    if expand_animate in {0, 1}:
        param_expand = gen_dummy_waypoint(param_expand, expand_animate, "real")

    # p1 not animated and p2 not animated
    if p1_animate in {0, 1} and p2_animate in {0, 1}:
        points["1"] = gen_dummy_waypoint(points["1"], p1_animate, "vector")
        points["2"] = gen_dummy_waypoint(points["2"], p2_animate, "vector")

    # p1 is animated and p2 is not animated
    elif p1_animate == 2 and p2_animate in {0, 1}:
        points["2"] = gen_dummy_waypoint(points["2"], p2_animate, "vector")

    # p1 is not animated and p2 is animated
    elif p1_animate in {0, 1} and p2_animate == 2:
        points["1"] = gen_dummy_waypoint(points["1"], p1_animate, "vector")

    print_animation(param_expand[0])
    print_animation(points["1"][0])
    print_animation(points["2"][0])
    both_points_animated(points["1"], points["2"], param_expand, lottie, index)


def gen_dummy_waypoint(non_animated, is_animate, anim_type):
    """
    Makes a non animated parameter to animated parameter by creating a new dummy
    waypoint with constant animation
    """
    if is_animate == 0:
        st = '<param name="anything"><animated type="{anim_type}"><waypoint time="0s" before="constant" after="constant"></waypoint></animated></param>'
        st = st.format(anim_type=anim_type)
        root = etree.fromstring(st)
        root[0][0].append(copy.deepcopy(non_animated[0]))
        non_animated = root
    elif is_animate == 1:
        non_animated[0][0].attrib["before"] = non_animated[0][0].attrib["after"] = "constant"

    new_waypoint = copy.deepcopy(non_animated[0][0])
    frame = get_frame(non_animated[0][0])
    frame += 1
    time = frame / settings.lottie_format["fr"]
    time = str(time) + "s"
    new_waypoint.attrib["time"] = time
    non_animated[0].insert(1, new_waypoint)
    return non_animated


def both_points_animated(animated_1, animated_2, param_expand, lottie, index):
    """
    This function generates the lottie dictionary for position and size property
    of lottie(point1 and point2 are used from Synfig), when both point1 and
    point2 are animated
    """
    animated_1, animated_2 = animated_1[0], animated_2[0]
    orig_path_1, orig_path_2 = {}, {}
    expand_path = {}
    gen_value_Keyframed(expand_path, param_expand[0], 0)
    gen_properties_multi_dimensional_keyframed(orig_path_1, animated_1, 0)
    gen_properties_multi_dimensional_keyframed(orig_path_2, animated_2, 0)

    #################### SECTION 1 ###########################
    # Insert waypoints in the point1 and point2 parameter at the place where
    # expand parameter is animated
    time_list = set()
    get_animated_time_list(param_expand, time_list)
    for frame in time_list:
        insert_waypoint_at_frame(animated_1, orig_path_1, frame, "vector")
        insert_waypoint_at_frame(animated_2, orig_path_2, frame, "vector")

    #################### END OF SECTION 1 ####################

    ######################### SECTION 2 ##########################
    # Insert the waypoints at corresponding positions where point1 is animated
    # and point2 is animated
    for waypoint in animated_1:
        frame = get_frame(waypoint)
        insert_waypoint_at_frame(animated_2, orig_path_2, frame, "vector")

    for waypoint in animated_2:
        frame = get_frame(waypoint)
        insert_waypoint_at_frame(animated_1, orig_path_1, frame, "vector")
    ##################### END OF SECTION 2 #######################

    ##################### SECTION 3 ##############################
    # Add the impact of expand parameter amount towards point1 and point2
    # parameter
    assert len(animated_1) == len(animated_2) 
    for waypoint1, waypoint2 in zip(animated_1, animated_2):
        frame = get_frame(waypoint1)
        assert frame == get_frame(waypoint2)
        expand_amount = get_vector_at_frame(expand_path, frame)
        expand_amount = to_Synfig_axis(expand_amount, "real")

        pos1, pos2 = get_vector(waypoint1), get_vector(waypoint2)
        # Comparing the x-coordinates
        if pos1.val1 > pos2.val1:
            pos1.val1 += expand_amount
            pos2.val1 -= expand_amount
        else:
            pos1.val1 -= expand_amount
            pos2.val1 += expand_amount
        # Comparing the y-coordinates
        if pos1.val2 > pos2.val2:
            pos1.val2 += expand_amount
            pos2.val2 -= expand_amount
        else:
            pos1.val2 -= expand_amount
            pos2.val2 += expand_amount
        set_vector(waypoint1, pos1)
        set_vector(waypoint2, pos2)
    ##################### END OF SECTION 3 #######################

    #################### SECTION 4 #############################
    # Place waypoints at which the x and y cross each other/cross the extremas
    cross_list = get_cross_list(animated_1, animated_2, orig_path_1, orig_path_2) 
    for frame in cross_list:
        insert_waypoint_at_frame(animated_1, orig_path_1, frame, "vector")
        insert_waypoint_at_frame(animated_2, orig_path_2, frame, "vector")
    #################### END SECTION 4 #########################


    ################## SECTION 5 ################################################
    # Store the position of rectangle according to the waypoints in pos_animated
    # Store the size of rectangle according to the waypoints in size_animated
    pos_animated = copy.deepcopy(animated_1)
    size_animated = copy.deepcopy(animated_1)
    size_animated.attrib["type"] = "rectangle_size"

    i, i1 = 0, 0
    while i < len(animated_1) - 1:
        cur_get_after_1, cur_get_after_2 = animated_1[i].attrib["after"], animated_2[i].attrib["after"]
        next_get_before_1, next_get_before_2 = animated_1[i+1].attrib["before"], animated_2[i+1].attrib["before"]

        dic_1 = {"linear", "auto", "clamped", "halt"}
        dic_2 = {"constant"}
        constant_interval_1 = cur_get_after_1 in dic_2 or next_get_before_1 in dic_2
        constant_interval_2 = cur_get_after_2 in dic_2 or next_get_before_2 in dic_2

        # Case 1 no "constant" interval is present
        if (cur_get_after_1 in dic_1) and (cur_get_after_2 in dic_1) and (next_get_before_1 in dic_1) and (next_get_before_2 in dic_1):
            get_average(pos_animated[i1], animated_1[i], animated_2[i])
            copy_tcb_average(pos_animated[i1], animated_1[i], animated_2[i])

            get_difference(size_animated[i1], animated_1[i], animated_2[i])
            copy_tcb(size_animated[i1], pos_animated[i1])
            i, i1 = i + 1, i1 + 1
            get_average(pos_animated[i1], animated_1[i], animated_2[i])
            copy_tcb_average(pos_animated[i1], animated_1[i], animated_2[i])

            get_difference(size_animated[i1], animated_1[i], animated_2[i])
            copy_tcb(size_animated[i1], pos_animated[i1])

        # Case 2 only one "constant" interval: could mean two "constant"'s are present
        elif (constant_interval_1 and not constant_interval_2) or (not constant_interval_1 and constant_interval_2):
            if constant_interval_1:
                i, i1 = calc_pos_and_size(size_animated, pos_animated, animated_1, animated_2, orig_path_2, i, i1)
            elif constant_interval_2:
                i, i1 = calc_pos_and_size(size_animated, pos_animated, animated_2, animated_1, orig_path_1, i, i1)

        # Case 3 both are constant
        elif constant_interval_1 and constant_interval_2:
            # No need to copy tcb, as it's pos should be "constant"
            get_average(pos_animated[i1], animated_1[i], animated_2[i])
            get_difference(size_animated[i1], animated_1[i], animated_2[i])

            i, i1 = i + 1, i1 + 1
            get_difference(size_animated[i1], animated_1[i], animated_2[i])
            get_average(pos_animated[i1], animated_1[i], animated_2[i])
    ######################### SECTION 5 END ##############################

    ######################### SECTION 6 ##################################
    # Generate the position and size for lottie format
    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                               pos_animated,
                                               index.inc())
    gen_value_Keyframed(lottie["s"], size_animated, index.inc())
    ########################## END OF SECTION 6 ###########################


def insert_waypoint_at_frame(animated, orig_path, frame, animated_name):
    """
    This function will only insert a waypoint at 'frame' if no waypoint is
    present at that 'frame' already
    """
    i = 0
    while i < len(animated):
        at_frame = get_frame(animated[i])
        if frame == at_frame:
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


def get_cross_list(animation_1, animation_2, orig_path_1, orig_path_2):
    """
    This function will return a list('set' technically) at which the point1 and point2 of rectangle
    will cross each other.
    This set might contain frames at which waypoints are already present, hence
    this need to be taken care of
    """
    en_fr = max(get_frame(animation_1[-1]), get_frame(animation_2[-1]))
    # Function to determine the sign of a variable
    sign = lambda a: (1, -1)[a < 0]
    prev_1 = float(animation_1[0][0][0].text), float(animation_1[0][0][1].text)
    prev_2 = float(animation_2[0][0][0].text), float(animation_2[0][0][1].text)

    # The list to be returned
    ret_list = set()
    frame = 1
    # Loop for all the frames
    while frame <= en_fr:
        now_1 = get_vector_at_frame(orig_path_1, frame) 
        now_1 = to_Synfig_axis(now_1, "vector")
        now_2 = get_vector_at_frame(orig_path_2, frame)
        now_2 = to_Synfig_axis(now_2, "vector")
        is_needed = False
        if sign(prev_1[0] - prev_2[0]) != sign(now_1[0] - now_2[0]):
            is_needed = True
        elif sign(prev_1[1] - prev_2[1]) != sign(now_1[1] - now_2[1]):
            is_needed = True
        if is_needed:
            ret_list.add(frame - 1)
            ret_list.add(frame)
        prev_1, prev_2 = now_1, now_2
        frame += 1
    return ret_list


def print_animation(b):
    """
    Given any animation, b, It prints the animation in pretty way.
    Helpful in debugging
    """
    a = copy.deepcopy(b)
    """
    for i in range(len(a)):
        a[i].attrib["frames"] = str(get_frame(a[i]))
        a[i][0][0].text = str(float(a[i][0][0].text) * settings.PIX_PER_UNIT)
        a[i][0][1].text = str(float(a[i][0][1].text) * settings.PIX_PER_UNIT)
        """
    print(etree.tostring(a, method='xml', encoding='utf8').decode())
                

def calc_pos_and_size(size_animated, pos_animated, animated_1, animated_2, orig_path, i, i1):
    """
    Between two frames, this function is called if either "only point1's
    interval is constant" or "only point2's interval is constant". It calculates
    the position and size property for lottie format. It also adds a new
    waypoint just before the end of the frame if required.
    """
    pos_animated[i1].attrib["after"] = animated_2[i].attrib["after"]
    size_animated[i1].attrib["after"] = animated_2[i].attrib["after"]

    copy_tcb(pos_animated[i1], animated_2[i])
    copy_tcb(size_animated[i1], animated_2[i])

    get_average(pos_animated[i1], animated_1[i], animated_2[i])
    get_difference(size_animated[i1], animated_1[i], animated_2[i])
    # Inserting a waypoint just before the nextwaypoint
    # Only if a waypoint can be inserted
    t_next = get_frame(animated_2[i+1])
    t_present = get_frame(animated_2[i])

    ######### Need to check if t_next - t_present < 2 #####
    if abs(t_next - t_present) >= 2:
        pos = get_vector_at_frame(orig_path, t_next - 1)
        pos = to_Synfig_axis(pos, "vector")
        new_waypoint = copy.deepcopy(pos_animated[i1])
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"]
        new_waypoint.attrib["time"] = str((t_next - 1) / settings.lottie_format["fr"]) + "s"
        new_waypoint[0][0].text, new_waypoint[0][1].text = str(pos[0]), str(pos[1])

        n_size_waypoint = copy.deepcopy(new_waypoint)
        get_average(new_waypoint, new_waypoint, animated_1[i])
        get_difference(n_size_waypoint, n_size_waypoint, animated_1[i])

        pos_animated.insert(i1 + 1, new_waypoint)
        size_animated.insert(i1 + 1, n_size_waypoint)
        i1 += 1
    i, i1 = i + 1, i1 + 1
    get_average(pos_animated[i1], animated_1[i], animated_2[i])
    get_difference(size_animated[i1], animated_1[i], animated_2[i])
    return i, i1


def to_Synfig_axis(pos, animated_name):
    """
    Converts a Lottie format vector or values into Synfig format vector or
    values
    """
    if animated_name == "vector":
        pos[0], pos[1] = pos[0] - settings.lottie_format["w"]/2, pos[1] - settings.lottie_format["h"]/2
        pos[1] = -pos[1]
        ret = [x/settings.PIX_PER_UNIT for x in pos]
    elif animated_name == "real":
        ret = pos / settings.PIX_PER_UNIT
    return ret


def get_first_control_point(interval):
    """
    Returns the first control point of a bezier interval
    """
    if len(interval["s"]) >= 2:
        st = Vector(interval["s"][0], interval["s"][1])
    else:
        st = interval["s"][0]
    return st


def get_last_control_point(interval):
    """
    Returns the last control point of a bezier interval
    """
    if len(interval["e"]) >= 2:
        en = Vector(interval["e"][0], interval["e"][1])
    else:
        en = interval["e"][0]
    return en


def get_control_points(interval):
    """
    Returns all 4 control points of a bezier interval
    """
    # If the interval is for position or vector
    if "to" in interval.keys():
        st = Vector(interval["s"][0], interval["s"][1])
        en = Vector(interval["e"][0], interval["e"][1])
        to = Vector(interval["to"][0], interval["to"][1])
        ti = Vector(interval["ti"][0], interval["ti"][1])

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
    if i < len(keyfr) - 1:
        # If hold interpolation
        if 'h' in keyfr[i].keys():
            pos = get_first_control_point(keyfr[i])
        else:
            st, to, ti, en = get_control_points(keyfr[i])
            this_fr = keyfr[i]["t"]
            next_fr = keyfr[i+1]["t"]
            percent = (t - this_fr) / (next_fr - this_fr) 
            pos = get_bezier_val(st, st + to, en + ti, en, percent)
    elif i >= len(keyfr) - 1:
        pos = get_last_control_point(keyfr[-2])

    if isinstance(pos, Vector):
        return [pos.val1, pos.val2]
    else:
        return pos


def get_animated_time_list(child, time_list):
    """
    Appends all the frames corresponding to the waypoints in the
    animated(child[0]) list, in time_list
    """
    animated = child[0]
    is_animate = is_animated(animated)
    if is_animate in {0, 1}:
        return
    for waypoint in animated:
        frame = get_frame(waypoint)
        time_list.add(frame)


def get_difference(waypoint, way_1, way_2):
    """
    Returns the absolute difference of vector's of way_1 and way_2 in "waypoint"
    Helpful in calculating 'size' parameter of lottie format from 'point1' and
    'point2' of Synfig format
    """
    waypoint[0].tag = "real" # If was vector before
    waypoint[0].attrib["value"] = str(abs(float(way_1[0][0].text) - float(way_2[0][0].text)))
    waypoint[0].attrib["value2"] = str(abs(float(way_1[0][1].text) - float(way_2[0][1].text)))


def get_average(waypoint, way_1, way_2):
    """
    Returns the average of vector's of way_1 and way_2 in "waypoint"
    Helpful in calculating 'position' parameter of lottie format from 'point1'
    and 'point2' of Synfig format
    """
    waypoint[0][0].text = str((float(way_1[0][0].text) + float(way_2[0][0].text)) / 2)
    waypoint[0][1].text = str((float(way_1[0][1].text) + float(way_2[0][1].text)) / 2)


def copy_tcb_average(new_waypoint, waypoint, next_waypoint):
    """
    Copies the average of TCB values of waypoint and next_waypoint to
    'new_waypoint'
    This is just a way around to determine TCB values at in-between intervals,
    where new waypoints are introduced. Technically we will first calculate the
    tangents at those new waypoints and then calculate any random TCB values
    from those tangents: IMPROVEMENT
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

