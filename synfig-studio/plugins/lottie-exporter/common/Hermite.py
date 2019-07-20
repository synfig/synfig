# pylint: disable=line-too-long
"""
Hermite.py
This will store the class Hermite which will store a Hermite curve
"""

import sys
sys.path.append("..")


class Hermite:
    """
    To keep the hermite curve stored inside this
    """
    def __init__(self, p1, p2, t1, t2):
        """
        Args:
            p1 (float)  : First control point of bezier curve
            p2 (float)  : Fourth control point
            t1 (float)  : Second control point
            t2 (float)  : Third control point

        Returns:
            (None)
        """
        self.p1 = p1
        self.p2 = p2
        self.t1 = t1
        self.t2 = t2
        self.sync()

    def sync(self):
        """
        Calculates the coefficients and variables required in calculation of
        derivative and value of the curve at time t.
        Here p1, p2, t1, t2 are control points of bezier curve and a, b, c, d
        are control points of hermite

        Args:
            (None)
        Returns:
            (None)
        """
        self.a = self.p1
        self.b = self.p1 + self.t1/3
        self.c = self.p2 - self.t2/3
        self.d = self.p2

        self.coeff0 = self.a
        self.coeff1 = self.b*3- self.a*3
        self.coeff2 = self.c*3 - self.b*6 + self.a*3
        self.coeff3 = self.d - self.c*3 + self.b*3 - self.a

    def derivative(self, x):
        """
        Calculates the derivative of the curve at x

        Args:
            x (float) : Time at which the derivative is needed

        Returns:
            (float) : The value of the derivative at time x
        """
        y = 1 - x
        ret = ((self.b - self.a) * y * y + (self.c - self.b) * x * y * 2 + (self.d - self.c) * x * x) * 3
        return ret

    def value(self, t):
        """
        Calculates the value of the curve at time t

        Args:
            t (float) : Time at which the value is needed

        Returns:
            (float) : Value of the curve at time t
        """
        ret = self.coeff0 + (self.coeff1 + (self.coeff2 + (self.coeff3)*t)*t)*t
        return ret
