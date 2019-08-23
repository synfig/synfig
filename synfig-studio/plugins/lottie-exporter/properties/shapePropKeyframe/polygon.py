# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of polygon layer
in Lottie format
"""

import sys
from common.Vector import Vector
from common.Bline import Bline
from common.Param import Param
from properties.shapePropKeyframe.helper import insert_dict_at
sys.path.append("../../")


def gen_dynamic_list_polygon(lottie, dynamic_list):
    """
    Generates the bline corresponding to polygon layer

    Args:
        lottie (dict) : Lottie format polygon layer will be stored here
        dynamic_list (common.Param.Param) : Synfig format points of polygon

    Returns:
        (None)
    """
    ################## SECTION 1 ################
    # Inserting the waypoints if not animated, finding the first and last frame
    # Calculating the path after this
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1
    dynamic_list = Bline(dynamic_list[0], dynamic_list)

    for entry in dynamic_list.get_entry_list():
        pos = entry["vector"]
        pos.update_frame_window(window)

        z = Param(pos.getparent(), pos.getparent().getparent())
        z.animate("vector", True)
        entry["vector"] = z

    layer = dynamic_list.get_layer().get_layer()
    origin = layer.get_param("origin")

    # Animating the origin
    origin.update_frame_window(window)
    origin.animate("vector")

    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################ END OF SECTION 1 ##############

    ################ SECTION 2 #####################
    # Generating values for all the frames in the window
    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)

        for entry in dynamic_list.get_entry_list():
            pos_cur = entry["vector"].get_value(fr)
            pos_next = entry["vector"].get_value(fr + 1)

            tangent1_cur, tangent2_cur = Vector(0, 0), Vector(0, 0)
            tangent1_next, tangent2_next = Vector(0, 0), Vector(0, 0)

            # Adding origin to each vertex
            origin_cur = origin.get_value(fr)
            origin_next = origin.get_value(fr + 1)
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
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr
