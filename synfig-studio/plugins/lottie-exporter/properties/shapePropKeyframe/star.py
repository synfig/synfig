# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of star layer
in Lottie format
"""

import sys
import math
from misc import Vector
from synfig.animation import get_bool_at_frame, to_Synfig_axis, get_vector_at_frame, gen_dummy_waypoint
from properties.valueKeyframed import gen_value_Keyframed
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import add, insert_dict_at, update_child_at_parent, update_frame_window
sys.path.append("../../")


def gen_list_star(lottie, layer):
    """
    Generates a shape layer corresponding to star layer by manipulating the
    parameters of the star

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

    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "origin":
                origin = chld
            elif chld.attrib["name"] == "radius1":
                radius1 = chld
            elif chld.attrib["name"] == "radius2":
                radius2 = chld
            elif chld.attrib["name"] == "angle":
                angle = chld
            elif chld.attrib["name"] == "points":
                points = chld
            elif chld.attrib["name"] == "regular_polygon":
                regular_polygon = chld

    # Animating origin
    update_frame_window(origin[0], window)
    origin = gen_dummy_waypoint(origin, "param", "vector")
    origin.attrib["name"] = "origin"
    update_child_at_parent(layer, origin, "param", "origin")
    # Generate path for the origin component
    origin_dict = {}
    origin[0].attrib["transform_axis"] = "true"
    gen_properties_multi_dimensional_keyframed(origin_dict, origin[0], 0)

    # Animating radius1
    update_frame_window(radius1[0], window)
    radius1 = gen_dummy_waypoint(radius1, "param", "real")
    radius1.attrib["name"] = "radius1"
    update_child_at_parent(layer, radius1, "param", "radius1")
    # Generate expand param for Lottie format
    radius1_dict = {}
    gen_value_Keyframed(radius1_dict, radius1[0], 0)

    # Animating radius2
    update_frame_window(radius2[0], window)
    radius2 = gen_dummy_waypoint(radius2, "param", "real")
    radius2.attrib["name"] = "radius2"
    update_child_at_parent(layer, radius2, "param", "radius2")
    # Generate expand param for Lottie format
    radius2_dict = {}
    gen_value_Keyframed(radius2_dict, radius2[0], 0)

    # Animating angle
    update_frame_window(angle[0], window)
    angle = gen_dummy_waypoint(angle, "param", "angle")
    angle.attrib["name"] = "angle"
    angle[0].attrib["type"] = "star_angle_new"
    update_child_at_parent(layer, angle, "param", "angle")
    # Generate expand param for Lottie format
    angle_dict = {}
    gen_value_Keyframed(angle_dict, angle[0], 0)

    # Animating points
    update_frame_window(points[0], window)
    points = gen_dummy_waypoint(points, "param", "real")
    points.attrib["name"] = "points"
    update_child_at_parent(layer, points, "param", "points")
    # Generate expand param for Lottie format
    points_dict = {}
    gen_value_Keyframed(points_dict, points[0], 0)

    mx_points = get_max_points(points)

    # Animating regular_polygon
    update_frame_window(regular_polygon[0], window)
    regular_polygon = gen_dummy_waypoint(regular_polygon, "param", "bool")
    regular_polygon.attrib["name"] = "regular_polygon"
    update_child_at_parent(layer, regular_polygon, "param", "regular_polygon")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)
        synfig_star(st_val, mx_points, origin_dict, radius1_dict, radius2_dict, angle_dict, points_dict, regular_polygon, fr)
        synfig_star(en_val, mx_points, origin_dict, radius1_dict, radius2_dict, angle_dict, points_dict, regular_polygon, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def get_max_points(points):
    """
    As a shape layer needs to have same number of vertices at each frame, we
    will draw a shape with max number of points. When number of points at a
    frame is less than mx, redundant vertices on the shape will be added
    """
    mx = -1
    for waypoint in points[0]:
        val = float(waypoint[0].attrib["value"])
        mx = max(mx, val)
    return mx


def synfig_star(st_val, mx_points, origin_dict, radius1_dict, radius2_dict, angle_dict, points_dict, regular_polygon_anim, fr):
    """
    Calculates the points for the rectangle layer as in Synfig:
    https://github.com/synfig/synfig/blob/678cc3a7b1208fcca18c8b54a29a20576c499927/synfig-core/src/modules/mod_geometry/star.cpp
    """

    angle = get_vector_at_frame(angle_dict, fr)
    points = int(to_Synfig_axis(get_vector_at_frame(points_dict, fr), "real"))
    radius1 = to_Synfig_axis(get_vector_at_frame(radius1_dict, fr), "real")
    radius2 = to_Synfig_axis(get_vector_at_frame(radius2_dict, fr), "real")
    regular_polygon = get_bool_at_frame(regular_polygon_anim[0], fr)
    origin_cur = get_vector_at_frame(origin_dict, fr)

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
