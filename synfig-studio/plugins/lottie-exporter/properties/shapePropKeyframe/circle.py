# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of circle layer
in Lottie format
"""

import sys
import math
from common.Matrix2 import Matrix2
from common.Vector import Vector
from common.Layer import Layer
from synfig.animation import print_animation, to_Synfig_axis
from properties.shapePropKeyframe.helper import add, insert_dict_at, update_child_at_parent, quadratic_to_cubic
sys.path.append("../../")


def gen_list_circle(lottie, layer):
    """
    Generates a shape layer corresponding to circle layer by manipulating the
    origin and radius of the circle

    Args:
        lottie (dict) : Lottie format circle layer will be stored in this
        layer  (common.Layer.Layer) : Synfig format circle layer

    Returns:
        (None)
    """
    ################### SECTION 1 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    origin = layer.get_param("origin")
    radius = layer.get_param("radius")

    # Animating the origin
    origin.update_frame_window(window)
    origin.animate("vector")

    radius.update_frame_window(window)
    radius.animate("real")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)  # This loop needs to be considered somewhere down

        synfig_circle(st_val, origin, radius, fr)
        synfig_circle(en_val, origin, radius, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def synfig_circle(st_val, origin_param, radius_param, fr):
    """
    Calculates the points for the circle layer as in Synfig:
    https://github.com/synfig/synfig/blob/678cc3a7b1208fcca18c8b54a29a20576c499927/synfig-core/src/modules/mod_geometry/circle.cpp

    Args:
        st_val (dict) : Lottie format circle is stored in this
        origin (common.Param.Param) : Lottie format origin of circle
        radius (common.Param.Param) : Lottie format radius of circle
        fr (int) : Frame number

    Returns:
        (None)
    """

    num_splines = 8
    angle = 180.0 / num_splines
    angle *= ((math.pi * 2) / 360)
    k = 1.0 / math.cos(angle)

    radius = abs(to_Synfig_axis(radius_param.get_value(fr), "real"))
    origin = origin_param.get_value(fr)

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
