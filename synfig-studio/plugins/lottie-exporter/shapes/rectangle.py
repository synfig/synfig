"""
Will store all functions needed to generate the rectangle layer in lottie
"""
import sys
import copy
from lxml import etree
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis
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
                ###################### IMP ##################################

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
                    div_animated.insert(j + 1, new_waypoint)
                    j += 1

                # y value is crossing the extrema
                elif flag == 2:
                    new_waypoint[0][0].text = str(x_change_val)
                    new_waypoint[0][1].text = str(y2_val)
                    new_waypoint.attrib["time"] = str(float(waypoint.attrib["time"][:-1]) + time_diff * t_y_cross) + "s"
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
                        div_animated.insert(j + 1, new_waypoint)
                        div_animated.insert(j + 2, two_new_waypoint)
                    else:
                        div_animated.insert(j + 1, two_new_waypoint)
                        div_animated.insert(j + 2, new_waypoint)
                    j += 2
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
