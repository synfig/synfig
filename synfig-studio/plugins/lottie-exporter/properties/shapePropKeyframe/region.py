# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of region layer
in Lottie format
"""

import sys
import ast
from synfig.animation import get_vector_at_frame, gen_dummy_waypoint
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.shapePropKeyframe.helper import insert_dict_at, update_frame_window, update_child_at_parent, append_path, animate_radial_composite, get_tangent_at_frame, convert_tangent_to_lottie
sys.path.append("../../")


def gen_bline_region(lottie, bline_point):
    """
    Generates the dictionary corresponding to properties/shapePropKeyframe.json,
    given a bline/spline

    Args:
        lottie     (dict) : Lottie generated keyframes will be stored here for shape/path
        bline_path (lxml.etree._Element) : shape/path store in Synfig format

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

    loop = False
    if "loop" in bline_point.keys():
        val = bline_point.attrib["loop"]
        if val == "false":
            loop = False
        else:
            loop = True

    for entry in bline_point:
        composite = entry[0]
        for child in composite:
            if child.tag == "point":
                pos = child
            elif child.tag == "t1":
                t1 = child
            elif child.tag == "t2":
                t2 = child
            elif child.tag == "split_radius":
                split_r = child
            elif child.tag == "split_angle":
                split_a = child

        # Necassary to update this before inserting new waypoints, as new
        # waypoints might include there on time: 0 seconds
        update_frame_window(pos[0], window)

        # Empty the pos and fill in the new animated pos
        pos = gen_dummy_waypoint(pos, "point", "vector")
        update_child_at_parent(composite, pos, "point")

        update_frame_window(split_r[0], window)
        split_r = gen_dummy_waypoint(split_r, "split_radius", "bool")
        update_child_at_parent(composite, split_r, "split_radius")

        update_frame_window(split_a[0], window)
        split_a = gen_dummy_waypoint(split_a, "split_angle", "bool")
        update_child_at_parent(composite, split_a, "split_angle")

        append_path(pos[0], composite, "point_path", "vector")

        animate_radial_composite(t1[0], window)
        animate_radial_composite(t2[0], window)

    layer = bline_point.getparent().getparent()
    for chld in layer:
        if chld.tag == "param" and chld.attrib["name"] == "origin":
            origin = chld

    # Animating the origin
    update_frame_window(origin[0], window)
    origin_parent = origin.getparent()
    origin = gen_dummy_waypoint(origin, "param", "vector")
    origin.attrib["name"] = "origin"
    update_child_at_parent(origin_parent, origin, "param", "origin")

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

        for entry in bline_point:
            composite = entry[0]
            for child in composite:
                if child.tag == "point_path":
                    dictionary = ast.literal_eval(child.text)
                    pos_cur = get_vector_at_frame(dictionary, fr)
                    pos_next = get_vector_at_frame(dictionary, fr + 1)
                elif child.tag == "t1":
                    t1 = child[0]
                elif child.tag == "t2":
                    t2 = child[0]
                elif child.tag == "split_radius":
                    split_r = child
                elif child.tag == "split_angle":
                    split_a = child

            tangent1_cur, tangent2_cur = get_tangent_at_frame(t1, t2, split_r, split_a, fr)
            tangent1_next, tangent2_next = get_tangent_at_frame(t1, t2, split_r, split_a, fr)

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
