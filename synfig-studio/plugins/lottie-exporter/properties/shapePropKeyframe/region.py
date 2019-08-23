# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of region layer
in Lottie format
"""

import sys
import copy
from common.Bline import Bline
from common.Param import Param
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import insert_dict_at, animate_tangents, get_tangent_at_frame, convert_tangent_to_lottie
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
        pos.update_frame_window(window)
        pos.animate("vector", True)

        split_r.update_frame_window(window)
        split_r.animate_without_path("bool")

        split_a.update_frame_window(window)
        split_a.animate_without_path("bool")

        animate_tangents(t1, window)
        animate_tangents(t2, window)

    layer = bline.get_layer().get_layer()
    origin = layer.get_param("origin")

    # Animating the origin
    origin.update_frame_window(window)
    origin.animate("vector")

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
            pos = entry["point"]
            pos_cur, pos_next = pos.get_value(fr), pos.get_value(fr + 1)
            t1 = entry["t1"]
            t2 = entry["t2"]
            split_r = entry["split_radius"]
            split_a = entry["split_angle"]

            tangent1_cur, tangent2_cur = get_tangent_at_frame(t1, t2, split_r, split_a, fr)
            tangent1_next, tangent2_next = get_tangent_at_frame(t1, t2, split_r, split_a, fr + 1)

            tangent1_cur, tangent2_cur = convert_tangent_to_lottie(tangent1_cur, tangent2_cur)
            tangent1_next, tangent2_next = convert_tangent_to_lottie(tangent1_next, tangent2_next)

            # Adding origin to each vertex
            origin_cur, origin_next = origin.get_value(fr), origin.get_value(fr + 1)
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
