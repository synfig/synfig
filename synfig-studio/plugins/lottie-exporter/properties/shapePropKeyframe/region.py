# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of region layer
in Lottie format
"""

import sys
import settings
from common.Bline import Bline
from properties.shapePropKeyframe.helper import insert_dict_at, animate_tangents, get_tangent_at_frame, convert_tangent_to_lottie
from synfig.animation import to_Lottie_axis
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
        width = entry["width"]  # Not needed, but required by Bline class
        # Hence we do not update the window also
        width.animate("real")

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
        synfig_region(bline, st_val, origin, fr)
        synfig_region(bline, en_val, origin, fr + 1)
        fr += 1
    # Setting final time
    lottie.append({})
    lottie[-1]["t"] = fr

def synfig_region(bline, st_val, origin_p, fr):
    """
    Calculates the points for the region layer in Synfig
    """
    bline_list = bline.get_list_at_frame(fr)
    origin_cur = origin_p.get_value(fr)         

    for bline_point in bline_list:
        tangent1 = bline_point.get_tangent1() * settings.PIX_PER_UNIT
        tangent2 = bline_point.get_tangent2() * settings.PIX_PER_UNIT

        tangent1, tangent2 = convert_tangent_to_lottie(tangent1, tangent2)

        # Adding origin to each vertex
        pos = to_Lottie_axis(bline_point.get_vertex().get_list(), "vector")
        for i in range(len(pos)):
            pos[i] += origin_cur[i]

        # Store values in dictionary
        st_val["i"].append(tangent1.get_list())
        st_val["o"].append(tangent2.get_list())
        st_val["v"].append(pos)
