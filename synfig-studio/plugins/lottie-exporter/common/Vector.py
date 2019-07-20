# pylint: disable=line-too-long
"""
Vector.py
This module will store the Vector class
"""

import sys
import math
import common
sys.path.append("..")


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

    def __init__(self, val1=0, val2=0, _type=None):
        """
        Args:
            val1  (float) : First value of the vector
            val2  (float) : Second value of the vector
            _type (:obj: `str`, optional)  : Type of vector

        Returns:
            (None)
        """
        self.val1 = val1
        self.val2 = val2
        self.type = _type

    def __str__(self):
        return "({0},{1}, {2})".format(self.val1, self.val2, self.type)

    def __add__(self, other):
        val1 = self.val1 + other.val1
        val2 = self.val2 + other.val2
        return Vector(val1, val2, self.type)

    def __sub__(self, other):
        val1 = self.val1 - other.val1
        val2 = self.val2 - other.val2
        return Vector(val1, val2, self.type)

    def __neg__(self):
        return -1 * self

    def __getitem__(self, key):
        if key:
            return round(self.val2, 3)
        return round(self.val1, 3)

    def __setitem__(self, key, value):
        if key:
            self.val2 = value
        else:
            self.val1 = value

    def mag(self):
        """
        Returns the magnitude of the vector

        Args:
            (None)
        Returns:
            (float) : The magnitude of the vector
        """
        return math.sqrt(self.mag_squared())

    def inv_mag(self):
        """
        Returns the inverse of the magnitude of the vector

        Args:
            (None)
        Returns:
            (float) : Magnitude inversed
        """
        return 1.0 / self.mag()

    def perp(self):
        """
        Returns a perpendicular version of the vector

        Args:
            (None)
        Returns:
            (common.Vector.Vector) : Perpendicular vector with same magnitude
        """
        return Vector(self.val2, -self.val1)

    def is_equal_to(self, other):
        """
        Tells if the current vector is equal to `other` vector

        Args:
            other (common.Vector.Vector) : The vector to be compared with

        Returns:
            (bool) : True if the vectors are equal
                   : False otherwise
        """
        return common.misc.approximate_equal((self - other).mag_squared(), 0)

    def mag_squared(self):
        """
        Returns the squared magnitude of the vector

        Args:
            (None)
        Returns:
            (float) : squared magnitude
        """
        ret = self.val1 * self.val1 + self.val2 * self.val2
        return ret

    def norm(self):
        """
        Returns a normalised version of the vector

        Args:
            (None)
        Returns:
            (common.Vector.Vector) : itselves whose magnitude is 1
        """
        obj = self * self.inv_mag()
        self.__dict__.update(obj.__dict__)
        return self

    # other can only be of type real
    def __mul__(self, other):
        if not isinstance(other, self.__class__):
            val1 = self.val1 * other
            val2 = self.val2 * other
            return Vector(val1, val2, self.type)
        elif isinstance(other, self.__class__):
            return self.val1*other.val1 + self.val2*other.val2
        raise Exception('Multiplication with {} not defined'.format(type(other)))

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        if not isinstance(other, self.__class__):
            val1 = self.val1 / other
            val2 = self.val2 / other
            return Vector(val1, val2, self.type)
        raise Exception('Division with {} not defined'.format(type(other)))

    def get_list(self):
        """
        Get val1 and val2 values in the format required by lottie

        Args:
            (None)

        Returns:
            (list) : Contains the Vector in list format
        """
        return [self.val1, self.val2]

    def get_val(self):
        """
        Get value in the format required by lottie

        Args:
            (None)

        Returns:
            (list) : Depending upon _type a list is returned
        """
        if self.type == "origin":
            ret = [self.val1, self.val2]
        elif self.type == "circle_radius":
            ret = [self.val1, self.val1]
        elif self.type in {"rectangle_size", "image_scale", "scale_layer_zoom", "group_layer_scale"}:
            ret = [self.val1, self.val3]
        else:
            ret = [self.val1]
        return ret

    def add_new_val(self, val3):
        """
        This function store an additional value in the vector.
        This is currently required by the rectangle layer

        Args:
            val3 (float) : Some Vectors need additional value to be used later

        Returns:
            (None)
        """
        self.val3 = val3

    def set_type(self, _type):
        """
        This set's the type of the Vector

        Args:
            _type (str) : Type of Vector to be set

        Returns:
            (None)
        """
        self.type = _type
