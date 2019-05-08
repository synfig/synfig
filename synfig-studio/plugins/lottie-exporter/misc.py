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
    """
    def __init__(self, x = 0, y = 0):
        self.x = x
        self.y = y

    def __str__(self):
        return "({0},{1})".format(self.x,self.y)

    def __add__(self, other):
        x = self.x + other.x
        y = self.y + other.y
        return Vector(x, y)

    def __sub__(self, other):
        x = self.x - other.x
        y = self.y - other.y
        return Vector(x, y)

    # other can only be of type real
    def __mul__(self, other):
        if not isinstance(other, self.__class__):
            x = self.x * other
            y = self.y * other
            return Vector(x, y)

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        if not isinstance(other, self.__class__):
            x = self.x / other
            y = self.y / other
            return Vector(x, y)

    def get_list(self):
        return [self.x, self.y]

class Color:
    """
    To store the colors in Synfig and operations on them
    """
    def __init__(self, r = 1, g = 1, b = 1):
        self.r = r
        self.g = g
        self.b = b

    def __str__(self):
        return "({0}, {1}, {2})".format(self.r, self.g, self.b)

    def __add__(self, other):
        r = self.r + other.r
        g = self.g + other.g
        b = self.b + other.b
        return Color(r, g, b)

    def __sub__(self, other):
        r = self.r - other.r
        g = self.g - other.g
        b = self.b - other.b
        return Color(r, g, b)

    def __mul__(self, other):
        if not isinstance(other, self.__class__):
            r = self.r * other
            g = self.g * other
            b = self.b * other
            return Color(r, g, b)

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        if not isinstance(other, self.__class__):
            r = self.r / other
            g = self.g / other
            b = self.b / other
            return Color(r, g, b)

    def get_list(self):
        return [self.r, self.g, self.b]

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
    elif animated.attrib["type"] == "real":
        pos = parse_value(animated, i)
    elif animated.attrib["type"] == "angle":
        pos = [get_angle(float(animated[i][0].attrib["value"])),
                float(animated[i].attrib["time"][:-1]) * settings.lottie_format["fr"]]
    return Vector(pos[0], pos[1])

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
    theta = theta % 360
    if theta < 90:
        theta = 90 - theta
    else:
        theta = 450 - theta
    theta = theta % 360
    return theta
