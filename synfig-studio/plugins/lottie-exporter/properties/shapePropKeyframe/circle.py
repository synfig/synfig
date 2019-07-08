# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of circle layer
in Lottie format
"""

import sys
import math
from misc import Matrix2, Vector
from synfig.animation import to_Synfig_axis, get_vector_at_frame, gen_dummy_waypoint
from properties.valueKeyframed import gen_value_Keyframed
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import add, insert_dict_at, update_child_at_parent, update_frame_window, quadratic_to_cubic
sys.path.append("../../")


def gen_list_circle(lottie, layer):
    """
    Generates a shape layer corresponding to circle layer by manipulating the
    origin and radius of the circle

    Args:
        lottie (dict) : Lottie format circle layer will be stored in this
        layer (lxml.etree._Element) : Synfig format circle layer

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

    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "origin":
                origin = chld
            elif chld.attrib["name"] == "radius":
                radius = chld

    # Animating the origin
    update_frame_window(origin[0], window)
    origin = gen_dummy_waypoint(origin, "param", "vector")
    origin.attrib["name"] = "origin"
    update_child_at_parent(layer, origin, "param", "origin")
    # Generate path for the origin component
    origin_dict = {}
    origin[0].attrib["transform_axis"] = "true"
    gen_properties_multi_dimensional_keyframed(origin_dict, origin[0], 0)

    update_frame_window(radius[0], window)
    radius = gen_dummy_waypoint(radius, "param", "real")
    radius.attrib["name"] = "radius"
    update_child_at_parent(layer, radius, "param", "width")

    # Generate radius for Lottie format
    radius_dict = {}
    gen_value_Keyframed(radius_dict, radius[0], 0)

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)  # This loop needs to be considered somewhere down

        synfig_circle(st_val, origin_dict, radius_dict, fr)
        synfig_circle(en_val, origin_dict, radius_dict, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def synfig_circle(st_val, origin_dict, radius_dict, fr):
    """
    Calculates the points for the circle layer as in Synfig:
    https://github.com/synfig/synfig/blob/678cc3a7b1208fcca18c8b54a29a20576c499927/synfig-core/src/modules/mod_geometry/circle.cpp
    """

    num_splines = 8
    angle = 180.0 / num_splines
    angle *= ((math.pi * 2) / 360)
    k = 1.0 / math.cos(angle)

    radius = abs(to_Synfig_axis(get_vector_at_frame(radius_dict, fr), "real"))
    origin = get_vector_at_frame(origin_dict, fr)

    matrix = Matrix2()
    matrix.set_rotate(angle)

    p0, p1, p2 = Vector(), Vector(), Vector(radius, 0)

    # Setup chunk list
    chunk_list = []
    chunk_list.append([p2, Vector(), Vector()])
    i = 0
    while i < num_splines:
        p0 = p2
        p1 = matrix.get_transformed(p0)
        p2 = matrix.get_transformed(p1)
        cp1, cp2 = quadratic_to_cubic(p2, k*p1, p0)
        cur_tan = p2 - cp1
        chunk_list[-1][2] = cp2 - p0
        chunk_list.append([p2, cur_tan, Vector()])
        i += 1

    add(chunk_list, st_val, origin)
