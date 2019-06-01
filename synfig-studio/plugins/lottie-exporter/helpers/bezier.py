"""
Module contains all functions required for calculation/help in bezier related
queries 
"""

import sys
import settings
import numpy as np
sys.path.append("..")


def get_bezier_time(P0, P1, P2, P3, point, frames):
    """
    Returns the fraction of time at which the value of bezier curve equals to
    the value of point
    B(t) = (1 - t)**3 P0 + 3(1 - t)**2 tP1 + 3(1 - t)t**2 P2 + t**3 P3
    where 0 < t < 1
    To optimize, binary search could be used here or later numpy might be used
    """
    min_diff = sys.maxsize
    fr = 1  # On frame number 1
    ret = 0
    while fr <= frames:
        t = fr/frames
        val = get_bezier_val(P0, P1, P2, P3, t)
        if abs(val - point) < min_diff:
            min_diff = abs(val - point)
            ret = t
        fr += 1
    return ret

     
def get_bezier_val(P0, P1, P2, P3, t):
    """
    Returns the value of bezier function at time t
    """
    bezier = (((1 - t)**3) * P0) + (3*((1 - t)**2) * t*P1) + (3*(1 - t)*(t**2)*P2) + ((t**3)*P3)
    return bezier
