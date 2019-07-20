# pylint: disable=line-too-long
"""
Module contains all functions required for calculation/help in bezier related
queries
"""

import sys
sys.path.append("..")


def get_bezier_time(P0, P1, P2, P3, point, frames):
    """
    Returns the fraction of time at which the value of bezier curve equals to
    the value of point
    B(t) = (1 - t)**3 P0 + 3(1 - t)**2 tP1 + 3(1 - t)t**2 P2 + t**3 P3
    where 0 < t < 1
    To optimize, binary search could be used here or later numpy might be used

    Args:
        P0 (float | common.Vector.Vector | common.Color.Color) : First control point
        P1 (float | common.Vector.Vector | common.Color.Color) : Second control point
        P2 (float | common.Vector.Vector | common.Color.Color) : Third control point
        P3 (float | common.Vector.Vector | common.Color.Color) : Fourth control point
        point (float | common.Vector.Vector | common.Color.Color) : point at which bezier curve is equal to this point
        frames (int) : Total number of frames present in this curve

    Returns:
        (float) : Time at which the point is present on the bezier curve
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

    Args:
        P0 (float | common.Vector.Vector | common.Color.Color) : First control point
        P1 (float | common.Vector.Vector | common.Color.Color) : Second control point
        P2 (float | common.Vector.Vector | common.Color.Color) : Third control point
        P3 (float | common.Vector.Vector | common.Color.Color) : Fourth control point
        t  (float)                            : Time

    Returns:
        (float | common.Vector.Vector | common.Color.Color) : value of bezier curve at time t
    """
    bezier = (((1 - t)**3) * P0) + (3*((1 - t)**2) * t*P1) + (3*(1 - t)*(t**2)*P2) + ((t**3)*P3)
    return bezier
