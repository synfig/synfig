# pylint: disable=line-too-long
"""
Will store all functions needed to generate the rectangle layer in lottie
"""

import sys
import copy
from lxml import etree
import settings
from common.Count import Count
from common.misc import is_animated, get_frame, get_vector, set_vector
from properties.value import gen_properties_value
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from synfig.animation import gen_dummy_waypoint, insert_waypoint_at_frame, to_Synfig_axis, get_vector_at_frame, get_animated_time_list, copy_tcb_average, copy_tcb
sys.path.append("..")


def get_child_value(is_animate, child, what_type):
    """
    Depending upon the is_animate type, value of the position or other
    parameters are extracted by this function

    Args:
        is_animate (int)                : Decides whether a parameter is animated
        child      (lxml.etree._Element): Holds the waypoint values
        what_type  (str)                : Decides the type of waypoint

    Returns:
        (float, float)    :   If the type of waypoint is "position"
        (float)           :   If the type of waypoint is "value"
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


def gen_custom_rectangle(lottie, idx):
    """
    Generates a dictionary of a rectangle which will cover the whole canvas of Synfig. This
    will be helpful to implement linear gradient of Synfig. (Gradient ramp from Adobe AE is
    not yet supported by lottie format)

    Args:
        lottie (dict) : The lottie generated custom rectangle will be stored in this
        idx    (int)  : Stores the index of the rectangle layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = "rc"     # Type: rectangle
    lottie["p"] = {}        # Position of rectangle
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of rectangle
    lottie["ix"] = idx      # setting the index
    lottie["r"] = {}        # Rounded corners of rectangle

    bevel = 0
    gen_properties_value(lottie["r"], bevel, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)

    # Set the position of the rectangle to the center of the Synfig canvas
    p1, p2 = settings.lottie_format["w"] / 2, settings.lottie_format["h"] / 2
    gen_properties_value(lottie["p"],
                         [p1, p2],
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

    # Set the size of the rectangle so as to fill the canvas
    s1, s2 = settings.lottie_format["w"], settings.lottie_format["h"]
    gen_properties_value(lottie["s"],
                         [s1, s2],
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

def gen_shapes_rectangle(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/rect.json

    Args:
        lottie (dict)               : The lottie generated rectangle layer will be stored in it
        layer  (lxml.etree._Element): Synfig format rectangle layer
        idx    (int)                : Stores the index of the rectangle layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = "rc"     # Type: rectangle
    lottie["p"] = {}        # Position of rectangle
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of rectangle
    lottie["ix"] = idx      # setting the index
    lottie["r"] = {}        # Rounded corners of rectangle
    points = {}
    bevel_found = False
    expand_found = False    # For filled rectangle layers

    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "point1":
                points["1"] = child # Store address of child here

            elif child.attrib["name"] == "point2":
                points["2"] = child # Store address of child here

            elif child.attrib["name"] == "expand":
                expand_found = True
                param_expand = child

            elif child.attrib["name"] == "bevel":
                bevel_found = True
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
    if not bevel_found:     # For rectangle layer in stable version 1.2.2
        bevel = 0
        gen_properties_value(lottie["r"], bevel, index.inc(),
                             settings.DEFAULT_ANIMATED, settings.NO_INFO)

    if not expand_found:    # Means filled rectangle layer, gen expand param
        st = "<param name='expand'><real value='0.0'/></param>"
        param_expand = etree.fromstring(st)

    # If expand parameter is not animated
    param_expand = gen_dummy_waypoint(param_expand, "param", "real")

    # expand parameter needs to be positive: required by Synfig
    make_positive_valued(param_expand)

    # If p1 not animated
    points["1"] = gen_dummy_waypoint(points["1"], "param", "vector")

    # If p2 not animated
    points["2"] = gen_dummy_waypoint(points["2"], "param", "vector")

    both_points_animated(points["1"], points["2"], param_expand, lottie, index)


def make_positive_valued(param):
    """
    User might mistakenly provide negative values to expand parameter; This is
    taken care of in this function
    """
    animated = param[0]
    for waypoint in animated:
        val = float(waypoint[0].attrib["value"])
        val = abs(val)
        waypoint[0].attrib["value"] = str(val)


def both_points_animated(animated_1, animated_2, param_expand, lottie, index):
    """
    This function generates the lottie dictionary for position and size property
    of lottie(point1 and point2 are used from Synfig), when both point1 and
    point2 are animated

    Args:
        animated_1      (lxml.etree._Element): Holds the parameter `point1`'s animation in Synfig xml format
        animated_2      (lxml.etree._Element): Holds the parameter `point2`'s animation in Synfig xml format
        param_expand    (lxml.etree._Element): Holds the parameter `expand`'s animation in Synfig xml format
        lottie          (dict)               : Lottie format rectangle layer will be store in this
        index           (int)                : Stores the index of parameters in rectangle layer

    Returns:
        (None)
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

    ### SECTION TRY ###
    # Every frames value is precomputed in order to achieve maximum similarity
    # to that of Synfig
    st_fr = min(get_frame(animated_1[0]), get_frame(animated_2[0]))
    en_fr = max(get_frame(animated_1[-1]), get_frame(animated_2[-1]))
    fra = st_fr
    while fra <= en_fr:
        insert_waypoint_at_frame(animated_1, orig_path_1, fra, "vector")
        insert_waypoint_at_frame(animated_2, orig_path_2, fra, "vector")
        fra += 1
    ### END SECTION ###

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
        if pos1[0] > pos2[0]:
            pos1[0] += expand_amount
            pos2[0] -= expand_amount
        else:
            pos1[0] -= expand_amount
            pos2[0] += expand_amount
        # Comparing the y-coordinates
        if pos1[1] > pos2[1]:
            pos1[1] += expand_amount
            pos2[1] -= expand_amount
        else:
            pos1[1] -= expand_amount
            pos2[1] += expand_amount
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


def get_cross_list(animation_1, animation_2, orig_path_1, orig_path_2):
    """
    This function will return a list('set' technically) at which the point1 and point2 of rectangle
    will cross each other.
    This set might contain frames at which waypoints are already present, hence
    this need to be taken care of

    Args:
        animation_1 (lxml.etree._Element): Stores the animation of `point1` parameter in Synfig format
        animation_2 (lxml.etree._Element): Stores the animation of `point2` parameter in Synfig format
        orig_path_1 (dict)               : Stores the animation of `point1` parameter in Lottie format
        orig_path_2 (dict)               : Stores the animation of `point2` parameter in Lottie format

    Returns:
        (set) : Contains the frames at which point1 and point2 cross each other
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


def calc_pos_and_size(size_animated, pos_animated, animated_1, animated_2, orig_path, i, i1):
    """
    Between two frames, this function is called if either "only point1's
    interval is constant" or "only point2's interval is constant". It calculates
    the position and size property for lottie format. It also adds a new
    waypoint just before the end of the frame if required.
    param3: Can be param1 or param2 of rectangle layer
    param4: Can be param2 or param1 of rectangle layer, but opposite of param3

    Args:
        size_animated (lxml.etree._Element): Holds the size parameter of rectangle layer in Synfig format
        pos_animated  (lxml.etree._Element): Holds the position parameter of rectangle layer in Synfig format
        animated_1    (lxml.etree._Element): Holds the param3 in Synfig format
        animated_2    (lxml.etree._Element): Holds the param4 in Synfig format
        orig_path     (dict)               : Holds the param4 in Lottie format
        i             (int)                : Iterator for animated_2
        i1            (int)                : Iterator for pos_animated and size_animated
    Returns:
        (int, int)    : Updated iterators i and i1 are returned
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


def get_difference(waypoint, way_1, way_2):
    """
    Returns the absolute difference of vector's of way_1 and way_2 in "waypoint"
    Helpful in calculating 'size' parameter of lottie format from 'point1' and
    'point2' of Synfig format

    Args:
        waypoint (lxml.etree._Element) : Holds the waypoint at which the return value is to be stored
        way_1    (lxml.etree._Element) : Waypoint contributing in calculating absolute difference
        way_2    (lxml.etree._Element) : Waypoint contributing in calculating absolute difference

    Returns:
        (None)
    """
    waypoint[0].tag = "real" # If was vector before
    waypoint[0].attrib["value"] = str(abs(float(way_1[0][0].text) - float(way_2[0][0].text)))
    waypoint[0].attrib["value2"] = str(abs(float(way_1[0][1].text) - float(way_2[0][1].text)))


def get_average(waypoint, way_1, way_2):
    """
    Returns the average of vector's of way_1 and way_2 in "waypoint"
    Helpful in calculating 'position' parameter of lottie format from 'point1'
    and 'point2' of Synfig format

    Args:
        waypoint (lxml.etree._Element) : Holds the waypoint at which the return value is to be stored
        way_1    (lxml.etree._Element) : Waypoint contributing in calculating average
        way_2    (lxml.etree._Element) : Waypoint contributing in calculating average

    Returns:
        (None)
    """
    waypoint[0][0].text = str((float(way_1[0][0].text) + float(way_2[0][0].text)) / 2)
    waypoint[0][1].text = str((float(way_1[0][1].text) + float(way_2[0][1].text)) / 2)
