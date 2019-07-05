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

def update_origins(root):
    """
    This will update the origins according to the pre compositions and there
    angle of rotations so as to accomodate/increase the overall size of that
    composition
    """
    offset = Vector(settings.lottie_format["w"], -settings.lottie_format["h"])
    offset /= settings.PIX_PER_UNIT
    offset = Vector(0, 0)
    rotate_layer = False
    for layer in reversed(root):
        if layer.tag == "layer" and layer.attrib["active"] == "true" and layer.attrib["exclude_from_rendering"] == "false":
            if layer.attrib["type"] == "rotate":
                if rotate_layer:
                    # Add only simple amount
                    add_straight_amount(layer, -offset)
                # Add radial amount here
                add_radial_amount(layer, offset)
                rotate_layer = True
            else:
                if layer.attrib["type"] == "SolidColor":
                    continue
                if rotate_layer:
                    # Add only simple amount here
                    add_straight_amount(layer, offset)


def add_radial_amount(layer, offset):
    for param in layer:
        if param.attrib["name"] == "amount":
            angle = param
        elif param.attrib["name"] == "origin":
            origin = param
    angle1 = float(angle[0].attrib["value"]) + 90
    angle2 = angle1 + 90
    angle1, angle2 = math.radians(angle1), math.radians(angle2)
    dir1 = Vector(math.cos(angle1), math.sin(angle1))
    dir2 = Vector(math.cos(angle2), math.sin(angle2))
    first = offset[1] * dir1
    second = offset[0] * dir2
    fin = first + second
    print(fin)

    is_animate = is_animated(origin[0])
    if is_animate == 0:
        add(origin[0], fin)
    else:
        for waypoint in orgin[0]:
            add(waypoint[0], offset)
    print_animation(origin)


def add_straight_amount(layer, offset):
    for param in layer:
        if param.attrib["name"] == "origin":
            origin = param

    is_animate = is_animated(origin[0])
    if is_animate == 0:
        add(origin[0], offset)
    else:
        for waypoint in origin[0]:
            add(waypoint[0], offset)
     

def update_precomp(node):
    """
    Inserts necassary offset in the positions of the layers if they lie inside
    another composition of Lottie
    """ 
    offset = Vector(settings.lottie_format["w"], -settings.lottie_format["h"])
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

# Should be -x, +y -> the offset
