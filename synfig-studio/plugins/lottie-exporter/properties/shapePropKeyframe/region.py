# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of region layer
in Lottie format
"""

import sys
import ast
from common.Bline import Bline
from common.Param import Param
from synfig.animation import get_vector_at_frame, gen_dummy_waypoint
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import insert_dict_at, update_frame_window, update_child_at_parent, append_path, animate_tangents, get_tangent_at_frame, convert_tangent_to_lottie
sys.path.append("../../")


def gen_bline_region(lottie, bline_point):
    """
    Generates the dictionary corresponding to properties/shapePropKeyframe.json,
    given a bline/spline

    Args:
        lottie     (dict) : Lottie generated keyframes will be stored here for shape/path
        bline_path (common.Param.Param) : shape/path store in Synfig format

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

    bline = Bline(bline_point[0], bline_point)
    loop = bline.get_loop()

    for entry in bline.get_entry_list():
        pos = entry["point"]
        t1 = entry["t1"]
        t2 = entry["t2"]
        split_r = entry["split_radius"]
        split_a = entry["split_angle"]

        # Necassary to update this before inserting new waypoints, as new
        # waypoints might include there on time: 0 seconds
        update_frame_window(pos[0], window)

        # Empty the pos and fill in the new animated pos
        pos = gen_dummy_waypoint(pos.get(), "point", "vector")
        update_child_at_parent(entry["composite"], pos, "point")
        entry["point"] = Param(pos, entry["composite"])

        update_frame_window(split_r[0], window)
        split_r = gen_dummy_waypoint(split_r.get(), "split_radius", "bool")
        update_child_at_parent(entry["composite"], split_r, "split_radius")
        entry["split_radius"] = Param(split_r, entry["composite"])

        update_frame_window(split_a[0], window)
        split_a = gen_dummy_waypoint(split_a.get(), "split_angle", "bool")
        update_child_at_parent(entry["composite"], split_a, "split_angle")
        entry["split_angle"] = Param(split_a, entry["composite"])

        append_path(pos[0], entry, "point_path", "vector")

        animate_tangents(t1, window)
        animate_tangents(t2, window)

    layer = bline.get_layer()
    origin = layer.get_param("origin")

    # Animating the origin
    update_frame_window(origin[0], window)
    origin_parent = origin.getparent()
    origin = gen_dummy_waypoint(origin.get(), "param", "vector", "origin")
    update_child_at_parent(origin_parent.get_layer(), origin, "param", "origin")

    # Generate path for the origin component
    origin_dict = {}
    origin[0].attrib["transform_axis"] = "true"
    gen_properties_multi_dimensional_keyframed(origin_dict, origin[0], 0)

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window
    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, loop)

        for entry in bline.get_entry_list():
            point_path_dict = entry["point_path"]
            pos_cur = get_vector_at_frame(point_path_dict, fr)
            pos_next = get_vector_at_frame(point_path_dict, fr + 1)
            t1 = entry["t1"]
            t2 = entry["t2"]
            split_r = entry["split_radius"]
            split_a = entry["split_angle"]

            tangent1_cur, tangent2_cur = get_tangent_at_frame(t1, t2, split_r, split_a, fr)
            tangent1_next, tangent2_next = get_tangent_at_frame(t1, t2, split_r, split_a, fr + 1)

            tangent1_cur, tangent2_cur = convert_tangent_to_lottie(tangent1_cur, tangent2_cur)
            tangent1_next, tangent2_next = convert_tangent_to_lottie(tangent1_next, tangent2_next)

            # Adding origin to each vertex
            origin_cur = get_vector_at_frame(origin_dict, fr)
            origin_next = get_vector_at_frame(origin_dict, fr + 1)
            for i in range(len(pos_cur)):
                pos_cur[i] += origin_cur[i]
            for i in range(len(pos_next)):
                pos_next[i] += origin_next[i]

            # Store values in dictionary
            st_val["i"].append(tangent1_cur.get_list())
            st_val["o"].append(tangent2_cur.get_list())
            st_val["v"].append(pos_cur)
            en_val["i"].append(tangent1_next.get_list())
            en_val["o"].append(tangent2_next.get_list())
            en_val["v"].append(pos_next)
        fr += 1
    # Setting final time
    lottie.append({})
    lottie[-1]["t"] = fr
