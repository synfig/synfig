"""
Will store all functions needed to generate the rectangle layer in lottie
"""
import sys
import copy
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
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
    new_child = copy.deepcopy(yes_animated)
    animated = new_child[0]
    for i in range(len(animated)):
        animated[i][0][0].text = str((float(animated[i][0][0].text) + x2_val) / 2)
        animated[i][0][1].text = str((float(animated[i][0][1].text) + y2_val) / 2)

    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                              animated,
                                              index.inc())

    # Have to store the size of the rectangle as per the lottie format
    # Hence we will create a new node in xml format that is to be supplied
    # to the already build function
    new_child = copy.deepcopy(yes_animated)
    animated = new_child[0]
    animated.attrib["type"] = "rectangle_size"
    for i in range(len(animated)):
        animated[i][0].tag = "real" # It was vector before
        animated[i][0].attrib["value"] = str(abs(x2_val - float(animated[i][0][0].text)))
        animated[i][0].attrib["value2"] = str(abs(y2_val - float(animated[i][0][1].text)))
    gen_value_Keyframed(lottie["s"], animated, index.inc())

