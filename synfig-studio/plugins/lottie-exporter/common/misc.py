# pylint: disable=line-too-long
"""
misc.py
Some miscellaneous functions and classes will be provided here
"""

import sys
import math
import settings
from common.Vector import Vector
from common.Color import Color
from common.Gradient import Gradient
sys.path.append("..")


def approximate_equal(a, b):
    """
    Need to define this function somewhere else, a and b are of type "float"

    Args:
        a (float) : First number to be compared
        b (float) : Second number to be compared

    Returns:
        (bool) : True if the numbers are approximately equal under precision
               : False otherwise
    """
    precision = 1e-8
    if a < b:
        return b - a < precision
    return a - b < precision


def real_high_precision():
    """
    Synfig format real high precision
    https://github.com/synfig/synfig/blob/ae11655a9bba068543be7a5df9090958579de78e/synfig-core/src/synfig/real.h#L55

    Args:
        (None)

    Returns:
        (float) : The value of real_high_precision as described in Synfig
    """
    return 1e-10


def calculate_pixels_per_unit():
    """
    Gives the value of 1 unit in terms of pixels according to the canvas defined

    Args:
        (None)

    Returns:
        (float) : Pixels per unit
    """
    image_width = float(settings.lottie_format["w"])
    image_area_width = settings.view_box_canvas["val"][2] - settings.view_box_canvas["val"][0]
    settings.PIX_PER_UNIT = image_width / image_area_width
    return settings.PIX_PER_UNIT


def change_axis(x_val, y_val, is_transform=True):
    """
    Convert synfig axis coordinates into lottie format

    Args:
        x_val (float | str) : x axis value in pixels
        y_val (float | str) : y axis value in pixels
        is_transform (`obj`: bool, optional) : Is this value used in transform module?

    Returns:
        (list)  : x and y axis value in Lottie format
    """
    x_val, y_val = float(x_val), float(y_val)
    if is_transform:
        x_val, y_val = x_val, -y_val
    else:
        x_val, y_val = x_val + settings.lottie_format["w"]/2, -y_val + settings.lottie_format["h"]/2
    return [x_val, y_val]


def parse_position(animated, i):
    """
    To convert the synfig coordinates from units(initially a string) to pixels
    Depends on whether a vector is provided to it or a real value
    If real value is provided, then time is also taken into consideration

    Args:
        animated (lxml.etree._Element) : Stores animation which contains waypoints
        i        (int)                 : Iterator over animation

    Returns:
        (common.Vector.Vector) If the animated type is not color
        (common.Color.Color)  Else if the animated type is color
    """
    if animated.attrib["type"] == "vector":
        pos = [float(animated[i][0][0].text),
               float(animated[i][0][1].text)]
        pos = [settings.PIX_PER_UNIT*x for x in pos]

    elif animated.attrib["type"] in {"real", "circle_radius"}:
        pos = parse_value(animated, i)

    elif animated.attrib["type"] == "angle":
        pos = [get_angle(float(animated[i][0].attrib["value"])),
               get_frame(animated[i])]

    elif animated.attrib["type"] in {"composite_convert", "region_angle", "star_angle_new", "scalar_multiply"}:
        pos = [float(animated[i][0].attrib["value"]),
               get_frame(animated[i])]

    elif animated.attrib["type"] == "rotate_layer_angle":
        # Angle needs to made neg of what they are
        pos = [-float(animated[i][0].attrib["value"]),
               get_frame(animated[i])]

    elif animated.attrib["type"] == "opacity":
        pos = [float(animated[i][0].attrib["value"]) * settings.OPACITY_CONSTANT,
               get_frame(animated[i])]

    elif animated.attrib["type"] == "effects_opacity":
        pos = [float(animated[i][0].attrib["value"]),
               get_frame(animated[i])]

    elif animated.attrib["type"] == "points":
        pos = [int(animated[i][0].attrib["value"]) * settings.PIX_PER_UNIT,
               get_frame(animated[i])]

    elif animated.attrib["type"] == "bool":
        val = animated[i][0].attrib["value"]
        if val == "false":
            val = 0
        else:
            val = 1
        pos = [val, get_frame(animated[i])]

    elif animated.attrib["type"] == "rectangle_size":
        pos = parse_value(animated, i)
        vec = Vector(pos[0], pos[1], animated.attrib["type"])
        vec.add_new_val(float(animated[i][0].attrib["value2"]) * settings.PIX_PER_UNIT)
        return vec

    elif animated.attrib["type"] == "image_scale":
        val = float(animated[i][0].attrib["value"])
        val2 = get_frame(animated[i])
        vec = Vector(val, val2, animated.attrib["type"])
        vec.add_new_val(float(animated[i][0].attrib["value2"]))
        return vec

    elif animated.attrib["type"] == "scale_layer_zoom":
        val = (math.e ** float(animated[i][0].attrib["value"])) * 100
        val2 = get_frame(animated[i])
        vec = Vector(val, val2, animated.attrib["type"])
        vec.add_new_val(val)
        return vec

    elif animated.attrib["type"] == "stretch_layer_scale":
        val1 = float(animated[i][0][0].text) * 100
        val3 = float(animated[i][0][1].text) * 100
        vec = Vector(val1, get_frame(animated[i]), animated.attrib["type"])
        vec.add_new_val(val3)
        return vec

    elif animated.attrib["type"] == "group_layer_scale":
        val1 = float(animated[i][0][0].text) * 100
        val3 = float(animated[i][0][1].text) * 100
        vec = Vector(val1, get_frame(animated[i]), animated.attrib["type"])
        vec.add_new_val(val3)
        return vec

    elif animated.attrib["type"] == "time":
        val = parse_time(animated[i][0].attrib["value"])    # Needed in seconds
        val2 = get_frame(animated[i])   # Needed in frames
        return Vector(val, val2, animated.attrib["type"])

    elif animated.attrib["type"] == "color":
        red = float(animated[i][0][0].text)
        green = float(animated[i][0][1].text)
        blue = float(animated[i][0][2].text)
        alpha = float(animated[i][0][3].text)
        red = red ** (1/settings.GAMMA[0])
        green = green ** (1/settings.GAMMA[1])
        blue = blue ** (1/settings.GAMMA[2])
        return Color(red, green, blue, alpha)
    
    elif animated.attrib["type"] == "gradient":
        return Gradient(animated[i][0])

    return Vector(pos[0], pos[1], animated.attrib["type"])


def parse_value(animated, i):
    """
    To convert the synfig value parameter from units to pixels
    and also take into consideration the time parameter

    Args:
        animated (lxml.etree._Element) : Stores animation which holds waypoints
        i        (int)                 : Iterator for animation

    Returns:
        (list)  : [value, time] is returned
    """
    pos = [float(animated[i][0].attrib["value"]) * settings.PIX_PER_UNIT,
           get_frame(animated[i])]
    return pos


def get_angle(theta):
    """
    Converts the .sif angle into lottie angle
    .sif uses positive x-axis as the start point and goes anticlockwise
    lottie uses positive y-axis as the start point and goes clockwise

    Args:
        theta (float) : Stores Synfig format angle

    Returns:
        (int)   : Lottie format angle
    """
    theta = int(theta)
    shift = -int(theta / 360)
    theta = theta % 360
    theta = (90 - theta) % 360

    theta = theta + shift * 360
    return theta


def is_animated(node):
    """
    Tells whether a parater is animated or not

    Args:
        node (lxml.etree._Element) : Animation which stores waypoints

    Returns:
        (int) : Depending upon whether the parameter is animated, following
                values are returned
                0: If not animated
                1: If only single waypoint is present
                2: If more than one waypoint is present
    """
    case = 0
    if node.tag == "animated":
        if len(node) == 1:
            case = 1
        else:
            case = 2
    else:
        case = 0
    return case


def clamp_col(color, gamma):
    """
    This function converts the colors into int and takes them to the range of
    0-255

    Args:
        color (float) : Synfig format color value

    Returns:
        (int) : Color value between 0-255
    """
    color = color ** (1/gamma)
    color *= 255
    color = int(color)
    return max(0, min(color, 255))


def get_color_hex(node):
    """
    Convert the <color></color> from rgba to hex format

    Args:
        node (lxml.etree._Element) : Synfig format color parameter

    Returns:
        (str) : hex format of color
    """
    red, green, blue = 1, 0, 0
    for col in node:
        if col.tag == "r":
            red = float(col.text)
        elif col.tag == "g":
            green = float(col.text)
        elif col.tag == "b":
            blue = float(col.text)
    # Convert to 0-255 range
    red, green, blue = clamp_col(red, settings.GAMMA[0]), clamp_col(green, settings.GAMMA[1]), clamp_col(blue, settings.GAMMA[2])

    # https://stackoverflow.com/questions/3380726/converting-a-rgb-color-tuple-to-a-six-digit-code-in-python/3380739#3380739
    ret = "#{0:02x}{1:02x}{2:02x}".format(red, green, blue)
    return ret


def get_frame(waypoint):
    """
    Given a waypoint, it parses the time to frames

    Args:
        waypoint (lxml.etree._Element) : Synfig format waypoint

    Returns:
        (int) : the frame at which waypoint is present
    """
    time = get_time(waypoint)
    frame = time * settings.lottie_format["fr"]
    frame = round(frame)
    return frame


def get_time(waypoint):
    """
    Given a waypoint, it parses the string time to float time

    Args:
        waypoint (lxml.etree._Element) : Synfig format waypoint

    Returns:
        (float) : the time in seconds at which the waypoint is present
    """
    return parse_time(waypoint.attrib["time"])


def parse_time(time_in_str):
    """
    Given a string, it parses time to float time

    Args:
        time_in_str (str) : Time in string format

    Returns:
        (float) : the time in seconds at represented by the string
    """
    time = time_in_str.split(" ")
    final = 0
    for frame in time:
        if frame[-1] == "h":
            final += float(frame[:-1]) * 60 * 60
        elif frame[-1] == "m":
            final += float(frame[:-1]) * 60
        elif frame[-1] == "s":
            final += float(frame[:-1])
        elif frame[-1] == "f":  # This should never happen according to my code
            raise ValueError("In waypoint, time is never expected in frames.")
    return final


def get_vector(waypoint):
    """
    Given a waypoint, it parses the string vector into Vector class defined in
    this converter

    Args:
        waypoint (lxml.etree._Element) : Synfig format waypoint

    Returns:
        (common.Vector.Vector) : x and y axis values stores in Vector format
    """
    # converting radius and angle to a vector
    if waypoint.tag == "radial_composite":
        for child in waypoint:
            if child.tag == "radius":
                radius = float(child[0].attrib["value"])
                radius *= settings.PIX_PER_UNIT
            elif child.tag == "theta":
                angle = float(child[0].attrib["value"])
        x, y = radial_to_tangent(radius, angle)
    else:
        x = float(waypoint[0][0].text)
        y = float(waypoint[0][1].text)
    return Vector(x, y)


def radial_to_tangent(radius, angle):
    """
    Converts a tangent from radius and angle format to x, y axis co-ordinate
    system

    Args:
        radius (float) : radius of a vector
        angle  (float) : angle in degrees

    Returns:
        (float) : The x-coordinate of the vector
        (float) : The y-coordinate of the vector
    """
    angle = math.radians(angle)
    x = radius * math.cos(angle)
    y = radius * math.sin(angle)
    return x, y


def set_vector(waypoint, pos):
    """
    Given a waypoint and pos(Vector), it set's the waypoint's vectors

    Args:
        waypoint (lxml.etree._Element) : Synfig format waypoint

    Returns:
        (None)
    """
    waypoint[0][0].text = str(pos.val1)
    waypoint[0][1].text = str(pos.val2)


def modify_final_dump(obj):
    """
    This function will remove unwanted keys from the final dictionary and also
    modify the floats according to our precision

    Args:
        obj (float | dict | list | tuple) : The object to be modified

    Returns:
        (float | dict | list) : Depending upon arguments, obj type is decided
    """
    if isinstance(obj, float):
        return round(obj, settings.FLOAT_PRECISION)
    elif isinstance(obj, dict):
        return dict((k, modify_final_dump(v)) for k, v in obj.items() if k not in ["synfig_i", "synfig_o"])
    elif isinstance(obj, (list, tuple)):
        return list(map(modify_final_dump, obj))
    return obj
