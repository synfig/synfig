"""
Will store all functions needed to generate the rectangle layer in lottie
"""

import sys
import copy
from lxml import etree
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis, Vector, get_time, get_frame
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

            elif child.attrib["name"] == "bevel":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    #gen_value_Keyframed(lottie["r"], child[0], index.inc())
                    pass
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

    # p1 not animated and p2 not animated
    if p1_animate in {0, 1} and p2_animate in {0, 1}:
        child = points["1"]
        x1_val, y1_val = get_child_value(p1_animate, child, "position")

        child = points["2"]
        x2_val, y2_val = get_child_value(p2_animate, child, "position")

        x_val = (x1_val + x2_val) / 2
        y_val = (y1_val + y2_val) / 2
        gen_properties_value(lottie["p"],
                            change_axis(x_val, y_val),
                            index.inc(),
                            settings.DEFAULT_ANIMATED,
                            settings.NO_INFO)

        size = [abs(x2_val - x1_val), abs(y2_val - y1_val)]
        gen_properties_value(lottie["s"],
                             size,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    # p1 is animated and p2 is not animated
    elif p1_animate == 2 and p2_animate in {0, 1}:
        only_one_point_animated_wrapper(points["2"], points["1"], p2_animate, lottie, index)

    # p1 is not animated and p2 is animated
    elif p1_animate in {0, 1} and p2_animate == 2:
        only_one_point_animated_wrapper(points["1"], points["2"], p1_animate, lottie, index)

    # p1 is animated and p2 is animated
    elif p1_animate == 2 and p2_animate == 2:
        both_points_animated(points["1"], points["2"], lottie, index)


def only_one_point_animated_wrapper(non_animated, yes_animated, is_animate, lottie, index):
    """
    This function serves as a wrapper when only one of the point is
    animated(point1 or point2). It creates dummy waypoints(whose value will
    remain same all the time), and makes it look like if they were also
    animated. Hence these animations can be passes to "both_points_animated()"
    function.
    """
    if is_animate == 0:
        st = '<param name="anything"><animated type="vector"><waypoint time="0s" before="constant" after="constant"></waypoint></animated></param>'
        root = etree.fromstring(st)
        root[0][0].append(copy.deepcopy(non_animated[0]))
        non_animated = root
    elif is_animate == 1:
        non_animated[0][0].attrib["before"] = non_animated[0][0].attrib["after"] = "constant"

    new_waypoint = copy.deepcopy(non_animated[0][0])
    frame = get_frame(non_animated[0][0])
    t_new = (frame + 1) / settings.lottie_format["fr"]
    t_new = str(t_new) + "s"
    new_waypoint.attrib["time"] = t_new
    non_animated[0].insert(1, new_waypoint)

    both_points_animated(non_animated, yes_animated, lottie, index)


def both_points_animated(animated_1, animated_2, lottie, index):
    """
    This function generates the lottie dictionary for position and size property
    of lottie(point1 and point2 are used from Synfig), when both point1 and
    point2 are animated
    """
    animated_1, animated_2 = animated_1[0], animated_2[0]
    orig_path_1, orig_path_2 = {}, {}
    gen_properties_multi_dimensional_keyframed(orig_path_1, animated_1, 0)
    gen_properties_multi_dimensional_keyframed(orig_path_2, animated_2, 0)

    #################### SECTION 0 #############################
    # Place waypoints at which the x and y cross each other/cross the extremas
    cross_list = get_cross_list(animated_1, animated_2, orig_path_1, orig_path_2) 
    for frame in cross_list:
        insert_waypoint_at_frame(animated_1, orig_path_1, frame, "vector")
    #################### END SECTION 0 #########################

    #################### SECTION 1 #############################
    # Place waypoint at point1 if point2 has a waypoint at that time and vice-versa
    c_anim_1, c_anim_2 = copy.deepcopy(animated_1), copy.deepcopy(animated_2)

    i, j = 0, 0 # iterator for 1st and 2nd animations 
    i1, j1 = 0, 0 # copy iterator for 1st and 2nd animations
    while i < len(animated_1) and j < len(animated_2):
        if equal_time_frame(animated_1[i], animated_2[j]):
            i, j = i+1, j+1
            i1, j1 = i1+1, j1+1
            continue
        t1 = get_time(animated_1[i])
        t2 = get_time(animated_2[j])
        if t2 < t1:
            # Insert waypoint in first animation
            insert_waypoint_param(c_anim_1, i1, animated_1, i, t1, t2, orig_path_1)
            j += 1
            j1 += 1
            i1 += 1
        elif t1 < t2:
            # Insert waypoint in second animation
            insert_waypoint_param(c_anim_2, j1, animated_2, j, t2, t1, orig_path_2)
            i += 1
            i1 += 1
            j1 += 1

    # This means for sure animated_1's last point's time < animated_2's points
    while i == len(animated_1) and j < len(animated_2):
        t2 = get_time(animated_2[j])
        new_waypoint = copy.deepcopy(animated_1[i-1])
        new_waypoint.attrib["time"] = str(t2) + "s"
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "constant"
        c_anim_1.insert(i1 + 1, new_waypoint)
        i1 += 1
        j += 1

    # This means for sure animated_2's last points's time < animated_1's points
    while j == len(animated_2) and i < len(animated_1):
        t1 = get_time(animated_1[i])
        new_waypoint = copy.deepcopy(animated_2[j-1])
        new_waypoint.attrib["time"] = str(t1) + "s"
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "constant"
        c_anim_2.insert(j1 + 1, new_waypoint)
        j1 += 1
        i += 1
    assert len(c_anim_1) == len(c_anim_2)
    ################## END OF SECTION 1 ##########################################

    ################## SECTION 2 ################################################
    # Store the position of rectangle according to the waypoints in pos_animated
    # Store the size of rectangle according to the waypoints in size_animated
    pos_animated = copy.deepcopy(c_anim_1)
    size_animated = copy.deepcopy(c_anim_1)
    size_animated.attrib["type"] = "rectangle_size"

    i, i1 = 0, 0
    while i < len(c_anim_1) - 1:
        cur_get_after_1, cur_get_after_2 = c_anim_1[i].attrib["after"], c_anim_2[i].attrib["after"]
        next_get_before_1, next_get_before_2 = c_anim_1[i+1].attrib["before"], c_anim_2[i+1].attrib["before"]

        dic_1 = {"linear", "auto", "clamped", "halt"}
        dic_2 = {"constant"}
        constant_interval_1 = cur_get_after_1 in dic_2 or next_get_before_1 in dic_2
        constant_interval_2 = cur_get_after_2 in dic_2 or next_get_before_2 in dic_2

        # Case 1 no "constant" interval is present
        if (cur_get_after_1 in dic_1) and (cur_get_after_2 in dic_1) and (next_get_before_1 in dic_1) and (next_get_before_2 in dic_1):
            get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
            copy_tcb_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])

            get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])
            copy_tcb(size_animated[i1], pos_animated[i1])
            i, i1 = i + 1, i1 + 1
            get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
            copy_tcb_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])

            get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])
            copy_tcb(size_animated[i1], pos_animated[i1])

        # Case 2 only one "constant" interval: could mean two "constant"'s are present
        elif (constant_interval_1 and not constant_interval_2) or (not constant_interval_1 and constant_interval_2):
            if constant_interval_1:
                i, i1 = calc_pos_and_size(size_animated, pos_animated, c_anim_1, c_anim_2, orig_path_2, i, i1)
            elif constant_interval_2:
                i, i1 = calc_pos_and_size(size_animated, pos_animated, c_anim_2, c_anim_1, orig_path_1, i, i1)

        # Case 3 both are constant
        elif constant_interval_1 and constant_interval_2:
            # No need to copy tcb, as it's pos should be "constant"
            get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
            get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])

            i, i1 = i + 1, i1 + 1
            get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])
            get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
    ######################### SECTION 2 END ##############################

    ######################### SECTION 3 ##################################
    # Generate the position and size for lottie format
    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                               pos_animated,
                                               index.inc())
    gen_value_Keyframed(lottie["s"], size_animated, index.inc())
    ########################## END OF SECTION 3 ###########################


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
                

def calc_pos_and_size(size_animated, pos_animated, c_anim_1, c_anim_2, orig_path, i, i1):
    """
    Between two frames, this function is called if either "only point1's
    interval is constant" or "only point2's interval is constant". It calculates
    the position and size property for lottie format. It also adds a new
    waypoint just before the end of the frame if required.
    """
    pos_animated[i1].attrib["after"] = c_anim_2[i].attrib["after"]
    size_animated[i1].attrib["after"] = c_anim_2[i].attrib["after"]

    copy_tcb(pos_animated[i1], c_anim_2[i])
    copy_tcb(size_animated[i1], c_anim_2[i])

    get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
    get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])
    # Inserting a waypoint just before the nextwaypoint
    # Only if a waypoint can be inserted
    t_next = get_frame(c_anim_2[i+1])
    t_present = get_frame(c_anim_2[i])

    ######### Need to check if t_next - t_present < 2 #####
    if abs(t_next - t_present) >= 2:
        pos = get_vector_at_frame(orig_path, t_next - 1)
        pos = to_Synfig_axis(pos, "vector")
        new_waypoint = copy.deepcopy(pos_animated[i1])
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"]
        new_waypoint.attrib["time"] = str((t_next - 1) / settings.lottie_format["fr"]) + "s"
        new_waypoint[0][0].text, new_waypoint[0][1].text = str(pos[0]), str(pos[1])

        n_size_waypoint = copy.deepcopy(new_waypoint)
        get_average(new_waypoint, new_waypoint, c_anim_1[i])
        get_difference(n_size_waypoint, n_size_waypoint, c_anim_1[i])

        pos_animated.insert(i1 + 1, new_waypoint)
        size_animated.insert(i1 + 1, n_size_waypoint)
        i1 += 1
    i, i1 = i + 1, i1 + 1
    get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
    get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])
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


def get_control_points(interval):
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
        st, to, ti, en = get_control_points(keyfr[0])
        pos = st
    if i < len(keyfr) - 1:
        # If hold interpolation
        if 'h' in keyfr[i].keys():
            return copy.deepcopy(keyfr[i]["s"])
        st, to, ti, en = get_control_points(keyfr[i])
        this_fr = keyfr[i]["t"]
        next_fr = keyfr[i+1]["t"]
        percent = (t - this_fr) / (next_fr - this_fr) 
        pos = get_bezier_val(st, st + to, en + ti, en, percent)
    elif i >= len(keyfr) - 1:
        st, to, ti, en = get_control_points(keyfr[-2])
        pos = en
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


def gen_param_expand(layer):
    points = {}
    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "expand":
                child = chld
            elif chld.attrib["name"] == "point1":
                points["1"] = chld
            elif chld.attrib["name"] == "point2":
                points["2"] = chld

    expand_path, point1_path, point2_path = {}, {}, {}
    gen_value_Keyframed(expand_path, child[0], 0)
    gen_properties_multi_dimensional_keyframed(point1_path, points["1"][0], 0)
    gen_properties_multi_dimensional_keyframed(point2_path, points["2"][0], 0)

    time_list = set()
    get_animated_time_list(points["1"], time_list)
    get_animated_time_list(points["2"], time_list)
    is_animate = is_animated(child[0]) 
    if is_animate == 2:
        for frame in time_list:
            insert_waypoint_at_frame(child[0], expand_path, frame, "real")

    # Will handle these cases later
    """
    elif is_animate == 1:
        pass
    else:
        expand = float(child[0].attrib["value"]) * settings.PIX_PER_UNIT
        pass
    """     
    for waypoint in child[0]:
        expand = float(waypoint[0].attrib["value"]) * settings.PIX_PER_UNIT
        expand = abs(expand)    # Negative value is treated positive in synfig expand parameter
        frame = get_frame(waypoint)
        point1_pos, point2_pos = get_vector_at_frame(point1_path, frame), get_vector_at_frame(point2_path, frame) 
        scale_x = 1 + (2 * expand) / abs(point1_pos[0] - point2_pos[0])
        scale_y = 1 + (2 * expand) / abs(point1_pos[1] - point2_pos[1])
        scale_x, scale_y = scale_x * 100, scale_y * 100
        waypoint[0].attrib["scale_x"] = str(scale_x)
        waypoint[0].attrib["scale_y"] = str(scale_y)
    child[0].attrib["type"] = "rectangle_expand"
    print_animation(child[0])
    return child[0]


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


def insert_waypoint_param(at_insert, i1, orig_at_insert, i, more_t, less_t, orig_path):
    """
    Given point1 and point2, this function helps in inserting waypoints at
    corresponding time, if a waypoint is not present there already
    """
    assert more_t > less_t 
    new_waypoint = copy.deepcopy(orig_at_insert[i])
    new_waypoint.attrib["time"] = str(less_t) + "s"

    if orig_at_insert[i].attrib["before"] != "constant" and (i == 0 or orig_at_insert[i-1].attrib["after"] != "constant"):
        if i == 0:
            # Means same values of x and y will be taken from next waypoint
            # But the before and after interpolations need to be changed to
            # linear type
            new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "linear"

        else:
            copy_tcb_average(new_waypoint, orig_at_insert[i-1], orig_at_insert[i]) 
            pos = get_vector_at_frame(orig_path, less_t * settings.lottie_format["fr"])
            pos = to_Synfig_axis(pos, "vector")

            # Setting the x and y value for the new waypoint
            new_waypoint[0][0].text = str(pos[0])
            new_waypoint[0][1].text = str(pos[1])
    elif orig_at_insert[i].attrib["before"] == "constant":
        new_waypoint.attrib["after"] = "constant"
        if i != 0:
            new_waypoint[0][0].text = orig_at_insert[i-1][0][0].text
            new_waypoint[0][1].text = orig_at_insert[i-1][0][1].text
    elif i != 0 and orig_at_insert[i-1].attrib["after"] == "constant":
        new_waypoint.attrib["after"] = new_waypoint.attrib["before"] = "constant"
        new_waypoint[0][0].text = orig_at_insert[i-1][0][0].text
        new_waypoint[0][1].text = orig_at_insert[i-1][0][1].text

    # Need to set interpolations and vectors before this
    at_insert.insert(i1, new_waypoint)


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


def equal_time_frame(waypoint1, waypoint2):
    """
    Returns 1 if two waypoints are present on the same frame
    Else returns 0
    """
    t1 = get_frame(waypoint1)
    t2 = get_frame(waypoint2)
    if abs(t1 - t2) < 0.9999999:
        return 1
    return 0
