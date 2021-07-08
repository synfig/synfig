"""
Color.py
Will store the Color class

NOTE: need to introduce clamping function
"""

import sys
sys.path.append("..")


class Color:
    """
    To store the colors in Synfig and operations on them
    """

    def __init__(self, red=1, green=1, blue=1, alpha=1):
        """
        Args:
            red (:obj: `float`, optional) : Red value of color
            green (:obj: `float`, optional) : Green value of color
            blue (:obj: `float`, optional) : Blue value of color
            alpha (:obj: `float`, optional) : Alpha value of color

        Returns:
            (None)
        """
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

        Args:
            (None)

        Returns:
            (list) : Stores color in list format
        """
        return [self.red, self.green, self.blue, self.alpha]
