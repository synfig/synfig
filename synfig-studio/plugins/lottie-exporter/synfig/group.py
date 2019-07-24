# pylint: disable=line-too-long
"""
Will store all the functions related to modify and play with group layer of
Synfig/precomp layer of Lottie
"""

import sys
import settings
from common.misc import is_animated
from common.Vector import Vector
import common
from synfig.animation import print_animation
sys.path.append("..")


def get_offset():
    """
    Computes the offset by which the layers need to be moved with

    Args:
        (None)
    Returns:
        (common.Vector.Vector) : offset
    """
    x = settings.ADDITIONAL_PRECOMP_WIDTH / 2
    y = settings.ADDITIONAL_PRECOMP_HEIGHT / 2
    offset = Vector(x, -y)
    offset /= settings.PIX_PER_UNIT
    return offset


def update_layer(node):
    """
    Inserts necassary offset in the positions of the layers if they lie inside
    another composition of Lottie

    Args:
        node (common.Layer.Layer | lxml.etree._Element) : Can be a layer or parameter of a layer

    Returns:
        (None)
    """
    # This if condition is not applicable for group, rotate, precomp... layers
    if not settings.INSIDE_PRECOMP:
        return

    update_dict = []
    if isinstance(node, common.Layer.Layer):
        compare = {"center", "origin", "point1", "point2", "tl", "br"}
        for param in compare:
            get = node.get_param(param).get()
            if get is not None:
                update_dict.append(get)
    else:
        update_dict.append(node)

    for param in update_dict:
        update_pos(param)


def update_pos(origin):
    """
    Updates the position of parameter in Synfig format

    Args:
        origin (lxml.etree._Element) : Stores the position of element

    Returns:
        (None)
    """
    offset = get_offset()
    is_animate = is_animated(origin[0])
    if is_animate == 0:
        add(origin[0], offset)
    else:
        for waypoint in origin[0]:
            add(waypoint[0], offset)


def add(vector, offset):
    """
    Helper function to modify Synfig xml

    Args:
        vector (lxml.etree._Element) : Position in Synfig format
        offset (common.Vector.Vector) : offset to be added to that position

    Returns:
        (None)
    """
    vector[0].text = str(float(vector[0].text) + offset[0])
    vector[1].text = str(float(vector[1].text) + offset[1])


def get_additional_width():
    """
    Returns the increase in pre-composition width depending on whether we are
    inside a pre-composition yet or not

    Args:
        (None)
    Returns:
        (float | int) : the additional width
    """
    if settings.INSIDE_PRECOMP:
        return settings.ADDITIONAL_PRECOMP_WIDTH
    return 0


def get_additional_height():
    """
    Returns the increase in pre-composition height depending on whether we are
    inside a pre-composition yet or not

    Args:
        (None)
    Returns:
        (float | int) : the additional height
    """
    if settings.INSIDE_PRECOMP:
        return settings.ADDITIONAL_PRECOMP_HEIGHT
    return 0

# Should be -x, +y -> the offset
