"""
Will store all functions needed to generate the rectangle layer in lottie
"""

import sys
import copy
from lxml import etree
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis, Vector
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
        only_one_point_animated(points["2"], points["1"], p2_animate, lottie, index)

    # p1 is not animated and p2 is animated
    elif p1_animate in {0, 1} and p2_animate == 2:
        only_one_point_animated(points["1"], points["2"], p1_animate, lottie, index)

    # p1 is animated and p2 is animated
    elif p1_animate == 2 and p2_animate == 2:
        both_points_animated(points["1"], points["2"], lottie, index)


def both_points_animated(animated_1, animated_2, lottie, index):
    #################### SECTION 1 #############################
    # Place waypoint at point1 if point2 has a waypoint at that time and vice-versa
    animated_1, animated_2 = animated_1[0], animated_2[0]
    c_anim_1, c_anim_2 = copy.deepcopy(animated_1), copy.deepcopy(animated_2)
    orig_path_1, orig_path_2 = {}, {}
    gen_properties_multi_dimensional_keyframed(orig_path_1, animated_1, 0)
    gen_properties_multi_dimensional_keyframed(orig_path_2, animated_2, 0)

    i, j = 0, 0 # iterator for 1st and 2nd animations 
    i1, j1 = 0, 0 # copy iterator for 1st and 2nd animations
    while i < len(animated_1) and j < len(animated_2):
        print(i, j, i1, j1)
        if equal_time_frame(animated_1[i], animated_2[j]):
            i, j = i+1, j+1
            i1, j1 = i1+1, j1+1
            continue
        t1 = float(animated_1[i].attrib["time"][:-1])
        t2 = float(animated_2[j].attrib["time"][:-1])
        if t2 < t1:
            # Insert waypoint in first animation
            insert_waypoint(c_anim_1, i1, animated_1, i, t1, t2, orig_path_1)
            j += 1
            j1 += 1
            i1 += 1
        elif t1 < t2:
            # Insert waypoint in second animation
            insert_waypoint(c_anim_2, j1, animated_2, j, t2, t1, orig_path_2)
            i += 1
            i1 += 1
            j1 += 1

    # This means for sure animated_1's last point's time < animated_2's points
    while i == len(animated_1) and j < len(animated_2):
        t2 = float(animated_2[j].attrib["time"][:-1])
        new_waypoint = copy.deepcopy(animated_1[i-1])
        new_waypoint.attrib["time"] = str(t2) + "s"
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "linear"
        c_anim_1.insert(i1 + 1, new_waypoint)
        i1 += 1
        j += 1

    # This means for sure animated_2's last points's time < animated_1's points
    while j == len(animated_2) and i < len(animated_1):
        t1 = float(animated_1[i].attrib["time"][:-1])
        new_waypoint = copy.deepcopy(animated_2[j-1])
        new_waypoint.attrib["time"] = str(t1) + "s"
        new_waypoint.attrib["before"] = new_waypoint.attrib["after"] = "linear"
        c_anim_2.insert(j1 + 1, new_waypoint)
        j1 += 1
        i += 1
    print(len(c_anim_1), len(c_anim_2))
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
        print("check", i)
        cur_get_after_1, cur_get_after_2 = c_anim_1[i].attrib["after"], c_anim_2[i].attrib["after"]
        next_get_before_1, next_get_before_2 = c_anim_1[i+1].attrib["before"], c_anim_2[i+1].attrib["before"]

        dic_1 = {"linear", "auto", "clamped", "halt"}
        dic_2 = {"constant"}
        constant_interval_1 = cur_get_after_1 in dic_2 or cur_get_after_2 in dic_2
        constant_interval_2 = next_get_before_1 in dic_2 or next_get_before_2 in dic_2

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
                i, i1 = pos_helper(size_animated, pos_animated, c_anim_1, c_anim_2, orig_path_2, i, i1)
            elif constant_interval_2:
                i, i1 = pos_helper(size_animated, pos_animated, c_anim_2, c_anim_1, orig_path_1, i, i1)

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
                

def pos_helper(size_animated, pos_animated, c_anim_1, c_anim_2, orig_path, i, i1):
    pos_animated[i1].attrib["after"] = c_anim_2[i].attrib["after"]
    size_animated[i1].attrib["after"] = c_anim_2[i].attrib["after"]

    copy_tcb(pos_animated[i1], c_anim_2[i])
    copy_tcb(size_animated[i1], c_anim_2[i])

    get_average(pos_animated[i1], c_anim_1[i], c_anim_2[i])
    get_difference(size_animated[i1], c_anim_1[i], c_anim_2[i])
    # Inserting a waypoint just before the nextwaypoint
    t_next = float(c_anim_2[i+1].attrib["time"][:-1]) * settings.lottie_format["fr"]

    ######### Need to check if t_next - t_present <= 1 #####
    pos = get_vector_at_t(orig_path, t_next - 1)
    pos = to_Synfig_axis(pos)
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


def to_Synfig_axis(pos):
    pos[0], pos[1] = pos[0] - settings.lottie_format["w"]/2, pos[1] - settings.lottie_format["h"]/2
    pos[1] = -pos[1]
    ret = [x/settings.PIX_PER_UNIT for x in pos]
    return ret


def get_vector_at_t(path, t):
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
        return keyfr[0]["s"]
    if i != len(keyfr):
        # If hold interpolation
        if "h" in keyfr[i].keys():
            return keyfr[i]["s"]
        st = Vector(keyfr[i]["s"][0], keyfr[i]["s"][1])
        en = Vector(keyfr[i]["e"][0], keyfr[i]["e"][1])
        to = Vector(keyfr[i]["to"][0], keyfr[i]["to"][1])
        ti = Vector(keyfr[i]["ti"][0], keyfr[i]["ti"][1])
        this_fr = keyfr[i]["t"]
        next_fr = keyfr[i+1]["t"]
        percent = (t - this_fr) / (next_fr - this_fr) 
        pos = get_bezier_val(st, st + to, en + ti, ti, percent)
    elif i == len(keyfr):
        pos = Vector(keyfr[i-1]["e"][0], keyfr[i-1]["e"][1])
    return [pos.val1, pos.val2]


def get_difference(waypoint, way_1, way_2):
    waypoint[0].tag = "real" # If was vector before
    waypoint[0].attrib["value"] = str(abs(float(way_1[0][0].text) - float(way_2[0][0].text)))
    waypoint[0].attrib["value2"] = str(abs(float(way_1[0][1].text) - float(way_2[0][1].text)))


def get_average(waypoint, way_1, way_2):
    waypoint[0][0].text = str(float(way_1[0][0].text) + float(way_2[0][0].text) / 2)
    waypoint[0][1].text = str(float(way_1[0][1].text) + float(way_2[0][1].text) / 2)


def insert_waypoint(at_insert, i1, orig_at_insert, i, more_t, less_t, orig_path):
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
            prev_t = float(at_insert[i1-1].attrib["time"][:-1])
            t_at = (less_t - prev_t) / (more_t - prev_t)
            y_val = get_bezier_val(orig_path["k"][i-1]["s"][1],
                                   orig_path["k"][i-1]["s"][1] + orig_path["k"][i-1]["to"][1], # Convert to bezier format
                                   orig_path["k"][i-1]["e"][1] + orig_path["k"][i-1]["ti"][1], # Convert to bezier format
                                   orig_path["k"][i-1]["e"][1],
                                   t_at)
            ## Convert y value from lottie format to synfig format
            y_val -= settings.lottie_format["h"]/2
            y_val = -y_val
            y_val /= settings.PIX_PER_UNIT  # As this value if to be put back in xml format

            x_val = get_bezier_val(orig_path["k"][i-1]["s"][0],
                                   orig_path["k"][i-1]["s"][0] + orig_path["k"][i-1]["to"][0], # Convert to bezier format
                                   orig_path["k"][i-1]["e"][0] + orig_path["k"][i-1]["ti"][0], # Convert to bezier format
                                   orig_path["k"][i-1]["e"][0],
                                   t_at)
            ## Convert x value from lottie format to synfig format
            x_val -= settings.lottie_format["w"]/2
            x_val /= settings.PIX_PER_UNIT

            # Setting the x and y value for the new waypoint
            new_waypoint[0][0].text = str(x_val)
            new_waypoint[0][1].text = str(y_val)
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
    at_insert.insert(i1 + 1, new_waypoint)


def only_one_point_animated(non_animated, yes_animated, is_animate, lottie, index):
    child = non_animated
    x2_val, y2_val = get_child_value(is_animate, child, "position")
    x2_val /= settings.PIX_PER_UNIT
    y2_val /= settings.PIX_PER_UNIT

    # Creating a new copy, we will modify this so as to store the position
    # of the rectangle layer[position means center of the rectangle here]
    divided_child = copy.deepcopy(yes_animated)
    div_animated = divided_child[0]

    o_animated = yes_animated[0]   # Original copy of animated
    orig_len = len(div_animated)
    j = 0                           # Iterator for newly created animation

    orig_path = {}
    gen_properties_multi_dimensional_keyframed(orig_path, o_animated, 0)

    for i in range(orig_len - 1):
        # Need to check if a point crosses other point's min or max value
        # Will add new waypoint if a point crosses that value
        waypoint, next_waypoint = o_animated[i], o_animated[i+1]
        cur_get_after, next_get_before = waypoint.attrib["after"], next_waypoint.attrib["before"]
        if cur_get_after == "constant" or next_get_before == "constant":
            # No waypoint is to be added
            pass
        else:
            flag = switch_case(o_animated, i, x2_val, y2_val)
            # One of x value or y value is crossing the extrema
            if flag in {1, 2, 3}:
                new_waypoint = copy.deepcopy(o_animated[i])
                new_waypoint.attrib["before"] = waypoint.attrib["after"]
                new_waypoint.attrib["after"] = next_waypoint.attrib["before"]
                num_frames = orig_path["k"][i+1]["t"] - orig_path["k"][i]["t"]
                
                ############# COPY THE TCB VALUES TO NEW WAYPOINT AS AVERAGE ##############
                copy_tcb_average(new_waypoint, waypoint, next_waypoint)

                # Calculate the time at which it crosses the x extrema
                t_x_cross = get_bezier_time(orig_path["k"][i]["s"][0],
                                    orig_path["k"][i]["s"][0] + orig_path["k"][i]["to"][0], # Convert to bezier format
                                    orig_path["k"][i]["e"][0] + orig_path["k"][i]["ti"][0], # Convert to bezier format
                                    orig_path["k"][i]["e"][0],
                                    x2_val * settings.PIX_PER_UNIT +\
                                    settings.lottie_format["w"]/2,
                                    num_frames)
                y_change_val = get_bezier_val(orig_path["k"][i]["s"][1],
                                              orig_path["k"][i]["s"][1] + orig_path["k"][i]["to"][1], # Convert to bezier format
                                              orig_path["k"][i]["e"][1] + orig_path["k"][i]["ti"][1], # Convert to bezier format
                                              orig_path["k"][i]["e"][1],
                                              t_x_cross)
                ## Convert y value from lottie format to synfig format
                y_change_val -= settings.lottie_format["h"]/2
                y_change_val = -y_change_val
                y_change_val /= settings.PIX_PER_UNIT  # As this value if to be put back in xml format

                # Calculate the time at which it crosses the y extrema
                t_y_cross = get_bezier_time(orig_path["k"][i]["s"][1],
                                    orig_path["k"][i]["s"][1] + orig_path["k"][i]["to"][1], # Convert to bezier format
                                    orig_path["k"][i]["e"][1] + orig_path["k"][i]["ti"][1], # Convert to bezier format
                                    orig_path["k"][i]["e"][1],
                                    -y2_val * settings.PIX_PER_UNIT +\
                                    settings.lottie_format["h"]/2,
                                    num_frames)
                x_change_val = get_bezier_val(orig_path["k"][i]["s"][0],
                                              orig_path["k"][i]["s"][0] + orig_path["k"][i]["to"][0], # Convert to bezier format
                                              orig_path["k"][i]["e"][0] + orig_path["k"][i]["ti"][0], # Convert to bezier format
                                              orig_path["k"][i]["e"][0],
                                              t_y_cross)
                ## Convert x value from lottie format to synfig format
                x_change_val -= settings.lottie_format["w"]/2
                x_change_val /= settings.PIX_PER_UNIT

                # Time difference calculation
                time_diff = float(next_waypoint.attrib["time"][:-1]) - float(waypoint.attrib["time"][:-1])

                # x value is crossing the extrema
                if flag == 1:
                    new_waypoint[0][0].text = str(x2_val)
                    new_waypoint[0][1].text = str(y_change_val)
                    new_waypoint.attrib["time"] = str(float(waypoint.attrib["time"][:-1]) + time_diff * t_x_cross) + "s"
                    # The waypoints to be inserted should not be conjusted i.e.
                    # be on same frame
                    if equal_time_frame(waypoint, new_waypoint) or equal_time_frame(new_waypoint, next_waypoint):
                        pass
                    else:
                        div_animated.insert(j + 1, new_waypoint)
                        j += 1

                # y value is crossing the extrema
                elif flag == 2:
                    new_waypoint[0][0].text = str(x_change_val)
                    new_waypoint[0][1].text = str(y2_val)
                    new_waypoint.attrib["time"] = str(float(waypoint.attrib["time"][:-1]) + time_diff * t_y_cross) + "s"
                    # The waypoints to be inserted should not be conjusted i.e.
                    # be on same frame
                    if equal_time_frame(waypoint, new_waypoint) or equal_time_frame(new_waypoint, next_waypoint):
                        pass
                    else:
                        div_animated.insert(j + 1, new_waypoint)
                        j += 1
                # both x value and y value are crossing the extrema
                elif flag == 3:
                    two_new_waypoint = copy.deepcopy(new_waypoint)
                    new_waypoint[0][0].text = str(x2_val)
                    new_waypoint[0][1].text = str(y_change_val)
                    new_waypoint.attrib["time"] = str(float(waypoint.attrib["time"][:-1]) + time_diff * t_x_cross) + "s"
                     
                    two_new_waypoint[0][0].text = str(x_change_val)
                    two_new_waypoint[0][1].text = str(y2_val)
                    two_new_waypoint.attrib["time"] = str(float(waypoint.attrib["time"][:-1]) + time_diff * t_y_cross) + "s"
                    if t_x_cross < t_y_cross:
                        if not equal_time_frame(waypoint, new_waypoint) and not equal_time_frame(new_waypoint, two_new_waypoint):
                            div_animated.insert(j + 1, new_waypoint)
                            j += 1
                        if not equal_time_frame(waypoint, two_new_waypoint) and not equal_time_frame(two_new_waypoint, next_waypoint):
                            div_animated.insert(j + 1, two_new_waypoint)
                            j += 1
                    else:
                        if not equal_time_frame(waypoint, two_new_waypoint) and not equal_time_frame(two_new_waypoint, new_waypoint):
                            div_animated.insert(j + 1, two_new_waypoint)
                            j += 1
                        if not equal_time_frame(waypoint, new_waypoint) and not equal_time_frame(new_waypoint, next_waypoint):
                            div_animated.insert(j + 1, new_waypoint)
                            j += 1
        j += 1

    pos_animated = copy.deepcopy(div_animated)

    for i in range(len(pos_animated)):
        pos_animated[i][0][0].text = str((float(pos_animated[i][0][0].text) + x2_val) / 2)
        pos_animated[i][0][1].text = str((float(pos_animated[i][0][1].text) + y2_val) / 2)

    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                              pos_animated,
                                              index.inc())

    # Have to store the size of the rectangle as per the lottie format
    # Hence we will create a new node in xml format that is to be supplied
    # to the already build function
    size_animated = copy.deepcopy(div_animated)
    size_animated.attrib["type"] = "rectangle_size"
    for i in range(len(size_animated)):
        size_animated[i][0].tag = "real" # It was vector before
        size_animated[i][0].attrib["value"] = str(abs(x2_val - float(size_animated[i][0][0].text)))
        size_animated[i][0].attrib["value2"] = str(abs(y2_val - float(size_animated[i][0][1].text)))
    gen_value_Keyframed(lottie["s"], size_animated, index.inc())


def copy_tcb_average(new_waypoint, waypoint, next_waypoint):
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


def switch_case(animated, i, x2_val, y2_val):
    x_now, y_now = float(animated[i][0][0].text), float(animated[i][0][1].text)
    x_next, y_next = float(animated[i+1][0][0].text), float(animated[i+1][0][1].text)
    sign = lambda a: (1, -1)[a < 0]
    x_change, y_change = 0, 0
    if sign(x_now - x2_val) != sign(x_next - x2_val):
        x_change = 1
    if sign(y_now - y2_val) != sign(y_next - y2_val):
        y_change = 1
    
    if x_change == 0 and y_change == 0:
        return 0
    elif x_change == 1 and y_change == 0:
        return 1
    elif x_change == 0 and y_change == 1:
        return 2
    elif x_change == 1 and y_change == 1:
        return 3


def equal_time_frame(waypoint1, waypoint2):
    t1 = float(waypoint1.attrib["time"][:-1]) * settings.lottie_format["fr"] 
    t2 = float(waypoint2.attrib["time"][:-1]) * settings.lottie_format["fr"] 
    if abs(t1 - t2) < 0.9999999:
        return 1
    return 0
