"""
helpers.py
Some helper functions will be written here
"""
import settings

class Count:
    """
    Class to keep count of variable
    """
    def __init__(self):
        self.idx = -1
    def inc(self):
        """
        This method increases the count by 1 and returns the new count
        """
        self.idx += 1
        return self.idx

class Vector:
    """
    To store the position of layers
    val1 represents the x-axis value
    val2 represents the y-axis value

    For other parameters
    val1 represents the value of the parameter
    val2 represents the time parameter

    type represents what this vector is representing
    """
    def __init__(self, val1=0, val2=0):
        self.val1 = val1
        self.val2 = val2
        self.type = None

    def __str__(self):
        return "({0},{1})".format(self.val1, self.val2)

    def __add__(self, other):
        val1 = self.val1 + other.val1
        val2 = self.val2 + other.val2
        return Vector(val1, val2)

    def __sub__(self, other):
        val1 = self.val1 - other.val1
        val2 = self.val2 - other.val2
        return Vector(val1, val2)

    # other can only be of type real
    def __mul__(self, other):
        if not isinstance(other, self.__class__):
            val1 = self.val1 * other
            val2 = self.val2 * other
            return Vector(val1, val2)
        raise Exception('Multiplication with {} not defined'.format(type(other)))

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        if not isinstance(other, self.__class__):
            val1 = self.val1 / other
            val2 = self.val2 / other
            return Vector(val1, val2)
        raise Exception('Division with {} not defined'.format(type(other)))

    def get_list(self):
        """
        Get val1 and val2 values in the format required by lottie
        """
        return [self.val1, self.val2]

    def get_val(self):
        """
        Get only val1 value in the format required by lottie
        """
        if self.type == "circle_radius":
            return self.get_val2()
        return [self.val1]
    
    def get_val2(self):
        """
        Get val1 value twice as required in circle layer by lottie format
        """
        return [self.val1, self.val1]

    def set_type(self, _type):
        self.type = _type

class Color:
    """
    To store the colors in Synfig and operations on them
    """
    def __init__(self, red=1, green=1, blue=1, alpha=1):
        self.red = red
        self.green = green
        self.blue = blue
        self.alpha = alpha

    def __str__(self):
        return "({0}, {1}, {2}, {3})".format(self.red, self.green, self.blue,
                                             self.alpha)

    def __add__(self, other):
        red = self.red + other.red
        green = self.green + other.green
        blue = self.blue + other.blue
        return Color(red, green, blue, self.alpha)

    def __sub__(self, other):
        red = self.red - other.red
        green = self.green - other.green
        blue = self.blue - other.blue
        return Color(red, green, blue, self.alpha)

    def __mul__(self, other):
        if not isinstance(other, self.__class__):
            red = self.red * other
            green = self.green * other
            blue = self.blue * other
            return Color(red, green, blue, self.alpha)
        raise Exception('Multiplication with {} not defined'.format(type(other)))

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        if not isinstance(other, self.__class__):
            red = self.red / other
            green = self.green / other
            blue = self.blue / other
            return Color(red, green, blue, self.alpha)
        raise Exception('Division with {} not defined'.format(type(other)))

    def get_val(self):
        """
        Get the color in the format required by lottie
        """
        return [self.red, self.green, self.blue, self.alpha]

def calculate_pixels_per_unit():
    """
    Gives the value of 1 unit in terms of pixels according to the canvas defined
    """
    image_width = float(settings.lottie_format["w"])
    image_area_width = settings.view_box_canvas["val"][2] - settings.view_box_canvas["val"][0]
    settings.PIX_PER_UNIT = image_width / image_area_width
    return settings.PIX_PER_UNIT

def change_axis(x_val, y_val):
    """
    Convert synfig axis coordinates into lottie format
    x_val and y_val should be in pixels
    """
    x_val, y_val = float(x_val), float(y_val)
    x_val, y_val = x_val + settings.lottie_format["w"]/2, -y_val + settings.lottie_format["h"]/2
    return [int(x_val), int(y_val)]

def parse_position(animated, i):
    """
    To convert the synfig coordinates from units(initially a string) to pixels
    Depends on whether a vector is provided to it or a real value
    If real value is provided, then time is also taken into consideration
    """
    if animated.attrib["type"] == "vector":
        pos = [float(animated[i][0][0].text),
               float(animated[i][0][1].text)]
        pos = [settings.PIX_PER_UNIT*x for x in pos]

    elif animated.attrib["type"] in {"real", "circle_radius"}:
        pos = parse_value(animated, i)

    elif animated.attrib["type"] == "angle":
        pos = [get_angle(float(animated[i][0].attrib["value"])),
               float(animated[i].attrib["time"][:-1]) * settings.lottie_format["fr"]]

    elif animated.attrib["type"] == "opacity":
        pos = [float(animated[i][0].attrib["value"]) * settings.OPACITY_CONSTANT,
               float(animated[i].attrib["time"][:-1]) * settings.lottie_format["fr"]]

    elif animated.attrib["type"] == "points":
        pos = [int(animated[i][0].attrib["value"]),
               float(animated[i].attrib["time"][:-1]) * settings.lottie_format["fr"]]

    elif animated.attrib["type"] == "color":
        red = float(animated[i][0][0].text)
        green = float(animated[i][0][1].text)
        blue = float(animated[i][0][2].text)
        alpha = float(animated[i][0][3].text)
        red = red ** (1/settings.GAMMA)
        green = green ** (1/settings.GAMMA)
        blue = blue ** (1/settings.GAMMA)
        return Color(red, green, blue, alpha)

    ret = Vector(pos[0], pos[1])
    ret.set_type(animated.attrib["type"])
    return ret

def parse_value(animated, i):
    """
    To convert the synfig value parameter from units to pixels
    and also take into consideration the time parameter
    return: Vector(value, time)
    """
    pos = [float(animated[i][0].attrib["value"]) * settings.PIX_PER_UNIT,
           float(animated[i].attrib["time"][:-1]) * settings.lottie_format["fr"]]
    return pos

def get_angle(theta):
    """
    Converts the .sif angle into lottie angle
    .sif uses positive x-axis as the start point and goes anticlockwise
    lottie uses positive y-axis as the start point and goes clockwise
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
    Return: 0, if not animated
          : 1, if only single waypoint is present
          : 2, if more than one waypoint is present
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
