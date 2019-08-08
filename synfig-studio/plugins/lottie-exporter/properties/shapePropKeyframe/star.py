# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of star layer
in Lottie format
"""

import sys
import math
from common.Vector import Vector
from common.Layer import Layer
from synfig.animation import to_Synfig_axis
from properties.shapePropKeyframe.helper import add, insert_dict_at, update_child_at_parent
sys.path.append("../../")


def gen_list_star(lottie, layer):
    """
    Generates a shape layer corresponding to star layer by manipulating the
    parameters of the star

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

    origin = layer.get_param("origin")
    radius1 = layer.get_param("radius1")
    radius2 = layer.get_param("radius2")
    angle = layer.get_param("angle")
    points = layer.get_param("points")
    regular_polygon = layer.get_param("regular_polygon")

    # Animating origin
    origin.update_frame_window(window)
    # Keeping the transform true here
    #origin.animate("vector", True)
    origin.animate("vector")

    # Animating radius1
    radius1.update_frame_window(window)
    radius1.animate("real")

    # Animating radius2
    radius2.update_frame_window(window)
    radius2.animate("real")

    # Animating angle
    angle.update_frame_window(window)
    angle.animate("star_angle_new")

    # Animating points
    points.update_frame_window(window)
    points.animate("real")

    mx_points = get_max_points(points)

    # Animating regular_polygon
    regular_polygon.update_frame_window(window)
    regular_polygon.animate_without_path("bool")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)
        synfig_star(st_val, mx_points, origin, radius1, radius2, angle, points, regular_polygon, fr)
        synfig_star(en_val, mx_points, origin, radius1, radius2, angle, points, regular_polygon, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def get_max_points(points):
    """
    As a shape layer needs to have same number of vertices at each frame, we
    will draw a shape with max number of points. When number of points at a
    frame is less than mx, redundant vertices on the shape will be added

    NOTE/IMPORTANT: There might come a case when somewhere between two
    waypoints, the number of points is greater than both those 2 waypoints(very
    rare). This case is not handled here

    Args:
        points (lxml.etree._Element) : Animation of points param in Synfig format

    Returns:
        mx (int) : Maximum number of points ever in the animation
    """
    mx = -1
    for waypoint in points[0]:
        val = float(waypoint[0].attrib["value"])
        mx = max(mx, val)
    return mx


def synfig_star(st_val, mx_points, origin_p, radius1_p, radius2_p, angle_p, points_p, regular_polygon_p, fr):
    """
    Calculates the points for the rectangle layer as in Synfig:
    https://github.com/synfig/synfig/blob/678cc3a7b1208fcca18c8b54a29a20576c499927/synfig-core/src/modules/mod_geometry/star.cpp

    Args:
        st_val (dict) : Lottie format star will be stored here
        mx_points (int) : Maximum points ever in star animation
        radius1_p (common.Param.Param) : Lottie format radius1 animation
        radius2_p (common.Param.Param) : Lottie format radius2 animation
        angle_p   (common.Param.Param) : Lottie format angle animation
        points_p  (common.Param.Param) : Lottie format points animation
        regular_polygon_p (common.Param.Param) : Synfig format regularPolygon animation
        fr (int) : Frame number

    Returns:
        (None)
    """

    angle = angle_p.get_value(fr)
    points = int(to_Synfig_axis(points_p.get_value(fr), "real"))
    radius1 = to_Synfig_axis(radius1_p.get_value(fr), "real")
    radius2 = to_Synfig_axis(radius2_p.get_value(fr), "real")
    regular_polygon = regular_polygon_p.get_value(fr)
    origin_cur = origin_p.get_value(fr)

    angle = math.radians(angle)
    dist_between_points = (math.pi * 2) / float(points)
    vector_list = []

    ####
    tot_points = 2*mx_points

    i = 0
    while i < points:
        dist1 = dist_between_points*i + angle
        dist2 = dist_between_points*i + dist_between_points/2 + angle
        vector_list.append(Vector(math.cos(dist1)*radius1, math.sin(dist1)*radius1))
        if not regular_polygon:
            vector_list.append(Vector(math.cos(dist2)*radius2, math.sin(dist2)*radius2))
        else:
            # This condition is needed because in lottie a shape must have equal
            # number of vertices at each frame
            vector_list.append(Vector(math.cos(dist1)*radius1, math.sin(dist1)*radius1))

        tot_points -= 2
        i += 1

    if len(vector_list) < 3:
        # Should throw error
        return

    while tot_points > 0:
        vector_list.append(vector_list[-1])
        tot_points -= 1

    # Setup chunk list
    chunk_list = []
    chunk_list.append([vector_list[0], Vector(), Vector()])
    i = 1
    while i < len(vector_list):
        if math.isnan(vector_list[i][0]) or math.isnan(vector_list[i][1]):
            break
        chunk_list.append([vector_list[i], Vector(), Vector()])
        i += 1

    add(chunk_list, st_val, origin_cur)
