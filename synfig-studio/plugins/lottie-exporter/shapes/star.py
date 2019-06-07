"""
Will store all functions needed to generate the star layer in lottie
"""

import sys
import settings
from properties.value import gen_properties_value
from misc import get_angle, Count, change_axis, is_animated
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")


def gen_shapes_star(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/star.json

    Args:
        lottie (dict)               : The lottie generated star layer will be stored in it
        layer  (lxml.etree._Element): Synfig format star layer
        idx    (int)                : Stores the index of the star layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = "sr"     # Type: star
    lottie["pt"] = {}       # Number of points on the star
    lottie["p"] = {}        # Position of star
    lottie["r"] = {}        # Angle / Star's rotation
    lottie["ir"] = {}       # Inner radius
    lottie["or"] = {}       # Outer radius
    lottie["is"] = {}       # Inner roundness of the star
    lottie["os"] = {}       # Outer roundness of the star
    regular_polygon = {"prop" : "false"}
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "regular_polygon":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    regular_polygon["prop"] = "changing"
                    # Copy the child address to dictionary
                    regular_polygon["animated"] = child[0]
                elif is_animate == 1:
                    regular_polygon["prop"] = child[0][0][0].attrib["value"]
                else:
                    regular_polygon["prop"] = child[0].attrib["value"]
                regular_polygon["animate"] = is_animate

            elif child.attrib["name"] == "points":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    # To uniquely identify the points, attribute type is changed
                    child[0].attrib['type'] = 'points'
                    gen_value_Keyframed(lottie["pt"], child[0], index.inc())

                else:
                    num_points = 3      # default number of points
                    if is_animate == 0:
                        num_points = int(child[0].attrib["value"])
                    else:
                        num_points = int(child[0][0][0].attrib["value"])
                    gen_properties_value(lottie["pt"],
                                         num_points,
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
            elif child.attrib["name"] == "angle":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_value_Keyframed(lottie["r"], child[0], index.inc())
                else:
                    theta = 0           # default angle for the star
                    if is_animate == 0:
                        theta = get_angle(float(child[0].attrib["value"]))
                    else:
                        theta = get_angle(float(child[0][0][0].attrib["value"]))
                    gen_properties_value(lottie["r"],
                                         theta,
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
            elif child.attrib["name"] == "radius1":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_value_Keyframed(lottie["or"], child[0], index.inc())
                else:
                    r_outer = 0             # default value for outer radius
                    if is_animate == 0:
                        r_outer = float(child[0].attrib["value"])
                    else:
                        r_outer = float(child[0][0][0].attrib["value"])

                    gen_properties_value(lottie["or"],
                                         int(settings.PIX_PER_UNIT * r_outer),
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
            elif child.attrib["name"] == "radius2":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_value_Keyframed(lottie["ir"], child[0], index.inc())
                else:
                    r_inner = 0             # default value for inner radius
                    if is_animate == 0:
                        r_inner = float(child[0].attrib["value"])
                    else:
                        r_inner = float(child[0][0][0].attrib["value"])
                    gen_properties_value(lottie["ir"],
                                         int(settings.PIX_PER_UNIT * r_inner),
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
            elif child.attrib["name"] == "origin":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                                               child[0],
                                                               index.inc())
                else:
                    x_val, y_val = 0, 0
                    if is_animate == 0:
                        x_val = float(child[0][0].text) * settings.PIX_PER_UNIT
                        y_val = float(child[0][1].text) * settings.PIX_PER_UNIT
                    else:
                        x_val = float(child[0][0][0][0].text) * settings.PIX_PER_UNIT
                        y_val = float(child[0][0][0][1].text) * settings.PIX_PER_UNIT
                    gen_properties_value(lottie["p"],
                                         change_axis(x_val, y_val),
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)

    # If not animated, then go to if, else
    if regular_polygon["animate"] in {0, 1}:
        if regular_polygon["prop"] == "false":
            lottie["sy"] = 1    # Star Type

            # inner property is only needed if type is star
            gen_properties_value(lottie["is"],
                                 0,
                                 index.inc(),
                                 settings.DEFAULT_ANIMATED,
                                 settings.NO_INFO)
        else:
            lottie["sy"] = 2    # Polygon Type

            # for polygon type, "ir" and "is" must not be present
            del lottie["ir"]

    # If animated, it will always be of type star
    else:
        polygon_correction(lottie, regular_polygon["animated"])

        lottie["sy"] = 1
        gen_properties_value(lottie["is"],
                             0,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    gen_properties_value(lottie["os"],
                         0,
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)
    lottie["ix"] = idx


"""
Below 3 functions are not complete yet:
Issues: If outer radius is animated, how do I change inner radius along with it
such that the shape remains polygon only
"""
def polygon_correction(lottie, animated):
    """
    Nothing here
    """
    length = len(animated) - 1
    st = 0

    true_arr = {}
    true_arr["arr"] = []
    true_arr["start"] = animated[0][0].attrib["value"]

    while st <= length:
        j = st + 1
        while j <= length and animated[st][0].attrib["value"] == animated[j][0].attrib["value"]:
            j += 1
        true_arr["arr"].append(j - 1)
        st = j

    # These operations will be performed on this new array created
    now = true_arr["start"]
    for st in range(len(true_arr["arr"])):
        if now == "false":
            i = true_arr["arr"][st]
            s_frame = float(animated[i].attrib["time"][:-1]) * settings.lottie_format["fr"]
            s_frame += 1

            # Till the end it is a star
            if st + 1 == len(true_arr["arr"]):
                break
            else:
                j = true_arr["arr"][st+1]
                e_frame = float(animated[j].attrib["time"][:-1]) * settings.lottie_format["fr"]
                e_frame -= 1
        elif now == "true":
            pass
        #modify(lottie, animated, s_frame, e_frame)
        now = toggle(now)


def toggle(val):
    """
    Nothing here
    """
    ret = "false"
    if val == "false":
        ret = "true"
    return ret


def modify(lottie, animated, s_frame, e_frame):
    """
    Nothing here
    """
    if e_frame < s_frame:
        return
    # If animated
    if lottie["ir"]["a"] == 1:
        st = 0
        length = len(lottie["ir"]["k"]) - 1
        while st <= length:
            if lottie["ir"]["k"][st]["t"] > t_insert:
                break
            st += 1
    return
