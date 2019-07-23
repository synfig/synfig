# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of polygon layer
in Lottie format
"""

import sys
from common.Vector import Vector
from common.Bline import Bline
from common.Param import Param
from synfig.animation import print_animation, get_vector_at_frame, gen_dummy_waypoint
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import append_path, update_frame_window, update_child_at_parent, insert_dict_at
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
    count = 0
    dynamic_list = Bline(dynamic_list[0], dynamic_list)

    for entry in dynamic_list.get_entry_list():
        pos = entry["vector"]
        update_frame_window(pos[0], window)

        new_pos = gen_dummy_waypoint(pos.getparent(), "entry", "vector")
        dynamic_list.set_entry(count, new_pos)

        append_path(new_pos[0], entry, "pos_path", "vector")
        count += 1

    layer = dynamic_list.get_layer()
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

    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################ END OF SECTION 1 ##############

    ################ SECTION 2 #####################
    # Generating values for all the frames in the window
    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)

        for entry in dynamic_list.get_entry_list():
            pos_path_dict = entry["pos_path"]
            pos_cur = get_vector_at_frame(pos_path_dict, fr)
            pos_next = get_vector_at_frame(pos_path_dict, fr + 1)

            tangent1_cur, tangent2_cur = Vector(0, 0), Vector(0, 0)
            tangent1_next, tangent2_next = Vector(0, 0), Vector(0, 0)

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
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr
