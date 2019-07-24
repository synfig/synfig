# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of rectangle layer
in Lottie format
"""

import sys
from lxml import etree
from common.misc import approximate_equal
from common.Vector import Vector
from common.Layer import Layer
from common.Param import Param
from synfig.animation import to_Synfig_axis
from properties.valueKeyframed import gen_value_Keyframed
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import add, insert_dict_at, update_child_at_parent, update_frame_window, quadratic_to_cubic
sys.path.append("../../")


def gen_list_rectangle(lottie, layer):
    """
    Generates a shape layer corresponding to rectangle layer by manipulating the
    parameters of the rectangle

    Args:
        lottie (dict) : Lottie format rectangle layer will be stored in this
        layer  (common.Layer.Layer) : Synfig format rectangle layer

    Returns:
        (None)
    """
    ################### SECTION 1 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    point1 = layer.get_param("point1")
    point2 = layer.get_param("point2")
    expand = layer.get_param("expand")
    bevel = layer.get_param("bevel")
    bevCircle = layer.get_param("bevCircle")

    if expand.get() is None:    # Means filled rectangle layer
        st = "<param name='expand'><real value='0.0'/></param>"
        expand = etree.fromstring(st)
        expand = layer.add_param("expand", expand)

    if bevel.get() is None:     # For rectangle layer in stable version 1.2.2
        st = "<param name='bevel'><real value='0.0'/></param>"
        bevel = etree.fromstring(st)
        bevel = layer.add_param("bevel", bevel)
        st = "<param name='bevCircle'><bool value='false'/></param>"
        bevCircle = etree.fromstring(st)
        bevCircle = layer.add_param("bevCircle", bevCircle)

    # Animating point1
    update_frame_window(point1[0], window)
    point1.animate("vector")
    point1.gen_path("vector")
    p1_dict = point1.get_path()

    # Animating point2
    update_frame_window(point2[0], window)
    point2.animate("vector")
    point2.gen_path("vector")
    p2_dict = point2.get_path()

    # Animating expand
    update_frame_window(expand[0], window)
    expand.animate("real")
    expand.gen_path()
    expand_dict = expand.get_path()

    # Animating bevel
    update_frame_window(bevel[0], window)
    bevel.animate("real")
    bevel.gen_path()
    bevel_dict = bevel.get_path()

    # Animating bevCircle
    update_frame_window(bevCircle[0], window)
    bevCircle.animate("bool")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)

        synfig_rectangle(st_val, point1, point2, expand, bevel, bevCircle, fr)
        synfig_rectangle(en_val, point1, point2, expand, bevel, bevCircle, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def synfig_rectangle(st_val, point1_p, point2_p, expand_p, bevel_p, bevCircle, fr):
    """
    Calculates the points for the rectangle layer as in Synfig:
    https://github.com/synfig/synfig/blob/678cc3a7b1208fcca18c8b54a29a20576c499927/synfig-core/src/modules/mod_geometry/rectangle.cpp

    Args:
        st_val (dict) : Lottie format rectangle will be stored in this
        p1_dict (dict) : Lottie format point1 animation
        p2_dict (dict) : Lottie format point2 animation
        expand_dict (dict) : Lottie format expand parameter animation
        bevel_dict (dict) : Lottie format bevel parameter animation
        bevCircle (lxml.etree._Element) : Animation of bevCircle in Synfig format
        fr (int) : Frame Number

    Returns:
        (None)
    """

    expand = abs(to_Synfig_axis(expand_p.get_value(fr), "real"))
    bevel = abs(to_Synfig_axis(bevel_p.get_value(fr), "real"))
    p0 = to_Synfig_axis(point1_p.get_value(fr), "vector")
    p0 = Vector(p0[0], p0[1])
    p1 = to_Synfig_axis(point2_p.get_value(fr), "vector")
    p1 = Vector(p1[0], p1[1])
    if p1[0] < p0[0]:
        p0[0], p1[0] = p1[0], p0[0]
    if p1[1] < p0[1]:
        p0[1], p1[1] = p1[1], p0[1]

    bev_circle = bevCircle.get_value(fr)

    w = p1[0] - p0[0] + 2*expand
    h = p1[1] - p0[1] + 2*expand
    bev = bevel
    if bevel > 1:
        bev = 1
    if bev_circle:
        bevx = min(w*bev/2.0, h*bev/2.0)
        bevy = min(w*bev/2.0, h*bev/2.0)
    else:
        bevx = w*bev/2.0
        bevy = h*bev/2.0

    # Setup chunk list
    chunk_list = []

    if approximate_equal(bevel, 0.0):
        chunk_list.append([Vector(p0[0] - expand, p0[1] - expand), Vector(), Vector()])
        chunk_list.append([Vector(p1[0] + expand, p0[1] - expand), Vector(), Vector()])
        chunk_list.append([Vector(p1[0] + expand, p1[1] + expand), Vector(), Vector()])
        chunk_list.append([Vector(p0[0] - expand, p1[1] + expand), Vector(), Vector()])
    else:
        cur = Vector(p1[0] + expand - bevx, p0[1] - expand)
        chunk_list.append([cur, Vector(), Vector()])
        prev = cur

        cur = Vector(p1[0] + expand, p0[1] - expand + bevy)
        cp1, cp2 = quadratic_to_cubic(cur, Vector(p1[0] + expand, p0[1] - expand), prev)
        chunk_list[-1][2] = cp2 - prev
        chunk_list.append([cur, cur - cp1, Vector()])
        prev = cur

        cur = Vector(p1[0] + expand, p1[1] + expand - bevy)
        chunk_list.append([cur, Vector(), Vector()])
        prev = cur

        cur = Vector(p1[0] + expand - bevx, p1[1] + expand)
        cp1, cp2 = quadratic_to_cubic(cur, Vector(p1[0] + expand, p1[1] + expand), prev)
        chunk_list[-1][2] = cp2 - prev
        chunk_list.append([cur, cur - cp1, Vector()])
        prev = cur

        cur = Vector(p0[0] - expand + bevx, p1[1] + expand)
        chunk_list.append([cur, Vector(), Vector()])
        prev = cur

        cur = Vector(p0[0] - expand, p1[1] + expand - bevy)
        cp1, cp2 = quadratic_to_cubic(cur, Vector(p0[0] - expand, p1[1] + expand), prev)
        chunk_list[-1][2] = cp2 - prev
        chunk_list.append([cur, cur - cp1, Vector()])
        prev = cur

        cur = Vector(p0[0] - expand, p0[1] - expand + bevy)
        chunk_list.append([cur, Vector(), Vector()])
        prev = cur

        cur = Vector(p0[0] - expand + bevx, p0[1] - expand)
        cp1, cp2 = quadratic_to_cubic(cur, Vector(p0[0] - expand, p0[1] - expand), prev)
        chunk_list[-1][2] = cp2 - prev
        chunk_list.append([cur, cur - cp1, Vector()])
        prev = cur

    add(chunk_list, st_val, [0, 0])
