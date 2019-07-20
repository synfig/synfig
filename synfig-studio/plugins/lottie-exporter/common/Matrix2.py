# pylint: disable=line-too-long
"""
Matrix2.py
Will store class for a Matrix 2X2
"""

import sys
import math
from common.Vector import Vector
sys.path.append("..")


class Matrix2:
    """
    Will store a Matrix 2X2
    """
    def __init__(self, m00=1.0, m01=0.0, m10=0.0, m11=1.0):
        """
        Args:
            m00 (:obj: `float`, optional) : 0th row, 0th column value
            m01 (:obj: `float`, optional) : 0th row, 1st column value
            m10 (:obj: `float`, optional) : 1st row, 0th column value
            m11 (:obj: `float`, optional) : 1st row, 1st column value

        Returns:
            (None)
        """
        self.m00 = m00
        self.m01 = m01
        self.m10 = m10
        self.m11 = m11

    def set_rotate(self, angle):
        """
        This will serve as a rotation matrix with angle `angle`

        Args:
            angle (float) : Angle in radians

        Returns:
            (None)
        """
        c = math.cos(angle)
        s = math.sin(angle)
        self.m00, self.m01 = c, s
        self.m10, self.m11 = -s, c

    def get_transformed(self, v):
        """
        Rotate or transform a vector using this Matrix

        Args:
            v (common.Vector.Vector) : Vector to be rotated using this matrix

        Returns:
            (common.Vector.Vector) : Rotated vector
        """
        ret = Vector()
        x, y = v[0], v[1]
        ret[0] = x*self.m00 + y*self.m10
        ret[1] = x*self.m01 + y*self.m11
        return ret
