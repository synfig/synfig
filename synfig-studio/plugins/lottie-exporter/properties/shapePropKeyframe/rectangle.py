# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of rectangle layer
in Lottie format
"""

import sys
from lxml import etree
from common.misc import approximate_equal
from common.Vector import Vector
from synfig.animation import get_bool_at_frame, to_Synfig_axis, get_vector_at_frame, gen_dummy_waypoint
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
        layer (lxml.etree._Element) : Synfig format rectangle layer

    Returns:
        (None)
    """
    ################### SECTION 1 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    # AFter that, there path will be calculated in lottie format which can
    # latter be used in get_vector_at_frame() function
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1
    bevel_found = False
    expand_found = False

    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "point1":
                point1 = chld
            elif chld.attrib["name"] == "point2":
                point2 = chld
            elif chld.attrib["name"] == "expand":
                expand_found = True
                expand = chld
            elif chld.attrib["name"] == "bevel":
                bevel_found = True
                bevel = chld
            elif chld.attrib["name"] == "bevCircle":
                bevCircle = chld

    if not expand_found:    # Means filled rectangle layer
        st = "<param name='expand'><real value='0.0'/></param>"
        expand = etree.fromstring(st)

    if not bevel_found:     # For rectangle layer in stable version 1.2.2
        st = "<param name='bevel'><real value='0.0'/></param>"
        bevel = etree.fromstring(st)
        st = "<param name='bevCircle'><bool value='false'/></param>"
        bevCircle = etree.fromstring(st)

    # Animating point1
    update_frame_window(point1[0], window)
    point1 = gen_dummy_waypoint(point1, "param", "vector", "point1")
    update_child_at_parent(layer, point1, "param", "point1")
    # Generate path for the point1 component
    p1_dict = {}
    #point1[0].attrib["transform_axis"] = "true"
    gen_properties_multi_dimensional_keyframed(p1_dict, point1[0], 0)

    # Animating point2
    update_frame_window(point2[0], window)
    point2 = gen_dummy_waypoint(point2, "param", "vector", "point2")
    update_child_at_parent(layer, point2, "param", "point2")
    # Generate path for the point2 component
    p2_dict = {}
    gen_properties_multi_dimensional_keyframed(p2_dict, point2[0], 0)

    # Animating expand
    update_frame_window(expand[0], window)
    expand = gen_dummy_waypoint(expand, "param", "real", "expand")
    update_child_at_parent(layer, expand, "param", "expand")
    # Generate expand param for Lottie format
    expand_dict = {}
    gen_value_Keyframed(expand_dict, expand[0], 0)

    # Animating bevel
    update_frame_window(bevel[0], window)
    bevel = gen_dummy_waypoint(bevel, "param", "real", "bevel")
    update_child_at_parent(layer, bevel, "param", "bevel")
    # Generate bevel param for Lottie format
    bevel_dict = {}
    gen_value_Keyframed(bevel_dict, bevel[0], 0)

    # Animating bevCircle
    update_frame_window(bevCircle[0], window)
    bevCircle = gen_dummy_waypoint(bevCircle, "param", "bool", "bevCircle")
    update_child_at_parent(layer, bevCircle, "param", "bevCircle")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)

        synfig_rectangle(st_val, p1_dict, p2_dict, expand_dict, bevel_dict, bevCircle, fr)
        synfig_rectangle(en_val, p1_dict, p2_dict, expand_dict, bevel_dict, bevCircle, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def synfig_rectangle(st_val, p1_dict, p2_dict, expand_dict, bevel_dict, bevCircle, fr):
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

    expand = abs(to_Synfig_axis(get_vector_at_frame(expand_dict, fr), "real"))
    bevel = abs(to_Synfig_axis(get_vector_at_frame(bevel_dict, fr), "real"))
    p0 = to_Synfig_axis(get_vector_at_frame(p1_dict, fr), "vector")
    p0 = Vector(p0[0], p0[1])
    p1 = to_Synfig_axis(get_vector_at_frame(p2_dict, fr), "vector")
    p1 = Vector(p1[0], p1[1])
    if p1[0] < p0[0]:
        p0[0], p1[0] = p1[0], p0[0]
    if p1[1] < p0[1]:
        p0[1], p1[1] = p1[1], p0[1]

    bev_circle = get_bool_at_frame(bevCircle[0], fr)

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
