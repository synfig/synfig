"""
Will store all functions needed to generate the star layer in lottie
"""

import sys
import settings
from properties.value import gen_properties_value
from common.misc import get_frame, get_angle, change_axis, is_animated
from common.Count import Count
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")


def gen_shapes_star(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/star.json

    Args:
        lottie (dict)               : The lottie generated star layer will be stored in it
        layer  (common.Layer.Layer)  : Synfig format star layer
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

    # Regular polygon
    rp = layer.get_param("regular_polygon").get()
    is_animate = is_animated(rp[0])
    if is_animate == 2:
        regular_polygon["prop"] = "changing"
        regular_polygon["animated"] = rp[0]
    elif is_animate == 1:
        regular_polygon["prop"] = rp[0][0][0].attrib["value"]
    else:
        regular_polygon["prop"] = rp[0].attrib["value"]
    regular_polygon["animate"] = is_animate

    # Points
    points = layer.get_param("points").get()
    is_animate = is_animated(points[0])
    if is_animate == 2:
        # To uniquely identify the points, attribute type is changed
        points[0].attrib['type'] = 'points'
        gen_value_Keyframed(lottie["pt"], points[0], index.inc())
    else:
        num_points = 3      # default number of points
        if is_animate == 0:
            num_points = int(points[0].attrib["value"])
        else:
            num_points = int(points[0][0][0].attrib["value"])
        gen_properties_value(lottie["pt"],
                             num_points,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    # Angle
    angle = layer.get_param("angle").get()
    is_animate = is_animated(angle[0])
    if is_animate == 2:
        gen_value_Keyframed(lottie["r"], angle[0], index.inc())
    else:
        theta = 0           # default angle for the star
        if is_animate == 0:
            theta = get_angle(float(angle[0].attrib["value"]))
        else:
            theta = get_angle(float(angle[0][0][0].attrib["value"]))
        gen_properties_value(lottie["r"],
                             theta,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    # Radius1
    radius1 = layer.get_param("radius1").get()
    is_animate = is_animated(radius1[0])
    if is_animate == 2:
        gen_value_Keyframed(lottie["or"], radius1[0], index.inc())
    else:
        r_outer = 0             # default value for outer radius
        if is_animate == 0:
            r_outer = float(radius1[0].attrib["value"])
        else:
            r_outer = float(radius1[0][0][0].attrib["value"])
        gen_properties_value(lottie["or"],
                             int(settings.PIX_PER_UNIT * r_outer),
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    # Radius2
    radius2 = layer.get_param("radius2").get()
    is_animate = is_animated(radius2[0])
    if is_animate == 2:
        gen_value_Keyframed(lottie["ir"], radius2[0], index.inc())
    else:
        r_inner = 0             # default value for inner radius
        if is_animate == 0:
            r_inner = float(radius2[0].attrib["value"])
        else:
            r_inner = float(radius2[0][0][0].attrib["value"])
        gen_properties_value(lottie["ir"],
                             int(settings.PIX_PER_UNIT * r_inner),
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    # Origin
    origin = layer.get_param("origin").get()
    is_animate = is_animated(origin[0])
    if is_animate == 2:
        gen_properties_multi_dimensional_keyframed(lottie["p"],
                                                   origin[0],
                                                   index.inc())
    else:
        x_val, y_val = 0, 0
        if is_animate == 0:
            x_val = float(origin[0][0].text) * settings.PIX_PER_UNIT
            y_val = float(origin[0][1].text) * settings.PIX_PER_UNIT
        else:
            x_val = float(origin[0][0][0][0].text) * settings.PIX_PER_UNIT
            y_val = float(origin[0][0][0][1].text) * settings.PIX_PER_UNIT
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
