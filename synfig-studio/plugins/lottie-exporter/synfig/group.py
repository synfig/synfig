# pylint: disable=line-too-long
"""
Will store all the functions related to modify and play with group layer of
Synfig/precomp layer of Lottie
"""

import sys
import settings
from misc import is_animated, Vector
sys.path.append("..")

def update_precomp(layer):
    """
    Inserts necassary offset in the positions of the layers if they lie inside
    another composition of Lottie
    """ 
    name = layer.attrib["type"]
    if name not in {"rotate"} and (not settings.INSIDE_PRECOMP):
        return

    offset = Vector(settings.lottie_format["w"], -settings.lottie_format["h"])
    offset /= settings.PIX_PER_UNIT
    if name in {"rotate"}:
        if settings.INSIDE_PRECOMP:
            offset = offset/2
        else:
            offset = offset/2

    for child in layer:
        if child.tag == "param" and child.attrib["name"] == "origin":
            origin = child

    is_animate = is_animated(origin[0])

    if is_animate == 0:
        add(origin[0], offset)
    else:
        for waypoint in origin[0]:
            add(waypoint[0], offset)

def add(vector, offset):
    """
    """
    vector[0].text = str(float(vector[0].text) + offset[0])
    vector[1].text = str(float(vector[1].text) + offset[1])

# Should be -x, +y -> the offset
