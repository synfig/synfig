# pylint: disable=line-too-long
"""
Will store all the functions related to modify and play with group layer of
Synfig/precomp layer of Lottie
"""

import sys
import math
import settings
from misc import is_animated, Vector
from synfig.animation import print_animation
sys.path.append("..")


def update_precomp(node):
    """
    Inserts necassary offset in the positions of the layers if they lie inside
    another composition of Lottie
    """ 
    x = settings.ADDITIONAL_PRECOMP_WIDTH / 2
    y = settings.ADDITIONAL_PRECOMP_HEIGHT / 2
    offset = Vector(x, -y)
    offset /= settings.PIX_PER_UNIT

    if node.tag == "layer":
        for child in node:
            if child.tag == "param" and child.attrib["name"] == "origin":
                origin = child 
        is_animate = is_animated(origin[0])
        if is_animate == 0:
            add(origin[0], offset)
        else:
            for waypoint in origin[0]:
                add(waypoint[0], offset)
    else:
        origin = node
        for waypoint in origin:
            add(waypoint[0], offset)


def add(vector, offset):
    """
    """
    vector[0].text = str(float(vector[0].text) + offset[0])
    vector[1].text = str(float(vector[1].text) + offset[1])


def get_additional_width():
    if settings.INSIDE_PRECOMP:
        return settings.ADDITIONAL_PRECOMP_WIDTH
    return 0


def get_additional_height():
    if settings.INSIDE_PRECOMP:
        return settings.ADDITIONAL_PRECOMP_HEIGHT
    return 0

# Should be -x, +y -> the offset
