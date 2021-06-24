# pylint: disable=line-too-long
"""
Angle.py
This module will store the Angle class

CHECK:
https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/ETL/ETL/_angle.h
"""

import sys
import math
from functools import total_ordering
sys.path.append("..")

ANGLE_EPSILON = 1e-6
PI = 3.1415926535897932384626433832795029

@total_ordering
class Angle:
    """
     
    """
    def __init__(self, v = 0):
        self.v = v

    def __add__(self, other):
        addition = self.v + other.v
        return Angle(addition)

    def __sub__(self, other):
        sub = self.v - other.v
        return Angle(sub)

    def __mul__(self, other):
        v = 0
        if not isinstance(other, self.__class__):
            v = self.v * other
            return Angle(v)
        else:
            v = self.v * other.v
        return Angle(v)

    def __rmul__(self, other):
        return self.__mul__(other)

    def __truediv__(self, other):
        div = 0
        if not isinstance(other, self.__class__):
            div = self.v / other
        else:
            div = self.v / other.v
        return Angle(div)

    def __neg__(self):
        neg = -self.v
        return Angle(neg)

    def __lt__(self, rhs):
        return self.v < rhs.v

    def __eq__(self, rhs):
        return math.fabs(self.v - rhs.v) < ANGLE_EPSILON


class RadAngle(Angle):
    def __init__(self, v):
        if isinstance(v, Angle):
            Angle.__init__(self, v.v)
        else:
            Angle.__init__(self, v)

    def get(self):
        return self.v


class DegAngle(Angle):
    def __init__(self, x):
        if isinstance(x, Angle):
            Angle.__init__(self, x.v)
        else:
            v = x * ((PI*2)/360)
            Angle.__init__(self, v)

    def get(self):
        return self.v * 360 / (PI*2)

class SinAngle(Angle):
    def __init__(self, x):
        if isinstance(x, Angle):
            Angle.__init__(self, x.v)
        else:
            v = math.asin(x)
            Angle.__init__(self, v)

    def get(self):
        return math.sin(self.v)

class CosAngle(Angle):
    def __init__(self, x):
        if isinstance(x, Angle):
            Angle.__init__(self, x.v)
        else:
            v = math.acos(x)
            Angle.__init__(self, v)

    def get(self):
        return math.cos(self.v)
