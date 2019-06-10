"""
"""

import sys
import ast
from lxml import etree
import settings
from misc import get_frame, Vector, change_axis, get_vector, is_animated, radial_to_tangent
from shapes.rectangle import gen_dummy_waypoint, get_vector_at_frame
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from shapes.rectangle import print_animation
sys.path.append("../")


def animate_radial_composite(radial_composite, window):
    """
    Animates the radial composite and updates the window of frame if radial
    composite's parameters are already animated
    Also generate the Lottie path and stores in radial_composite
    """
    for child in radial_composite:
        if child.tag == "radius":
            radius = child
        elif child.tag == "theta":
            theta = child
    update_frame_window(radius[0], window)
    update_frame_window(theta[0], window)

    radius = gen_dummy_waypoint(radius, is_animated(radius[0]), "real")
    theta = gen_dummy_waypoint(theta, is_animated(theta[0]), "region_angle")

    # Empty the radial_composite and fill in new values
    for child in radial_composite:
        child.getparent().remove(child)
    radial_composite.insert(0, radius)
    radial_composite.insert(1, theta)

    # Generating the radial path and store in the lxml element
    radius_dict = {}
    gen_value_Keyframed(radius_dict, radius[0], 0)
    # Store in lxml element
    radius_lxml = etree.Element("radius_path")
    radius_lxml.text = str(radius_dict)
    radial_composite.append(radius_lxml)

    # Generating the theta path and store in the lxml element
    theta_dict = {}
    gen_value_Keyframed(theta_dict, theta[0], 0)
    # Store in lxml element
    theta_lxml = etree.Element("theta_path")
    theta_lxml.text = str(theta_dict)
    radial_composite.append(theta_lxml)


def update_frame_window(node, window):
    if is_animated(node) == 2:
        for waypoint in node:
            fr = get_frame(waypoint)
            if fr > window["last"]:
                window["last"] = fr
            if fr < window["first"]:
                window["first"] = fr


def gen_properties_shapePropKeyframe(lottie, bline_point):
    """
    """
    # Assuming split angle and split radius both are ticked for now

    ################### SECTION 2 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    # AFter that, there path will be calculated in lottie format which can
    # latter be used in get_vector_at_frame() function
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    for entry in bline_point:
        composite = entry[0]
        for child in composite:
            if child.tag == "point":
                pos = child
            elif child.tag == "t1":
                t1 = child
            elif child.tag == "t2":
                t2 = child
                
        # Necassary to update this before inserting new waypoints, as new
        # waypoints might include there on time: 0 seconds
        update_frame_window(pos[0], window)

        # Empty the pos and fill in the new animated pos
        pos = gen_dummy_waypoint(pos, is_animated(pos[0]), "vector")
        for child in composite:
            if child.tag == "point":
                child.getparent().remove(child)
        composite.insert(0, pos)

        # Generate path for Lottie format
        path_dict = {}
        gen_properties_multi_dimensional_keyframed(path_dict, pos[0], 0)
        # Store in lxml element
        path_lxml = etree.Element("point_path")
        path_lxml.text = str(path_dict)
        composite.append(path_lxml)

        animate_radial_composite(t1[0], window)
        animate_radial_composite(t2[0], window)

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window 
    fr = window["first"]
    while fr <= window["last"]:
        lottie.append({})
        lottie[-1]["i"], lottie[-1]["o"] = {}, {}
        lottie[-1]["i"]["x"] = lottie[-1]["i"]["y"] = 0.5   # Does not matter because frames are adjacent
        lottie[-1]["o"]["x"] = lottie[-1]["o"]["y"] = 0.5   # Does not matter because frames are adjacent
        lottie[-1]["t"] = fr
        lottie[-1]["s"], lottie[-1]["e"] = [], []           # Start and end value of the path
        st_val, en_val = lottie[-1]["s"], lottie[-1]["e"]
        st_val.append({})
        en_val.append({})
        st_val, en_val = st_val[0], en_val[0]
        st_val["i"], st_val["o"], st_val["v"], st_val["c"] = [], [], [], False
        en_val["i"], en_val["o"], en_val["v"], en_val["c"] = [], [], [], False
        for entry in bline_point:
            composite = entry[0]
            for child in composite:
                if child.tag == "point_path":
                    dictionary = ast.literal_eval(child.text)
                    pos_cur = get_vector_at_frame(dictionary, fr) 
                    pos_next = get_vector_at_frame(dictionary, fr + 1)
                elif child.tag == "t1":
                    t1 = child[0]
                    for chld in t1:
                        if chld.tag == "radius_path":
                            dictionary = ast.literal_eval(chld.text)
                            r1_cur = get_vector_at_frame(dictionary, fr)
                            r1_next = get_vector_at_frame(dictionary, fr + 1)
                        elif chld.tag == "theta_path":
                            dictionary = ast.literal_eval(chld.text)
                            a1_cur = get_vector_at_frame(dictionary, fr)
                            a1_next = get_vector_at_frame(dictionary, fr + 1)
                    x, y = radial_to_tangent(r1_cur, a1_cur)
                    tangent1_cur = Vector(x, y)
                    x, y = radial_to_tangent(r1_next, a1_next)
                    tangent1_next = Vector(x, y)
                elif child.tag == "t2":
                    t2 = child[0]
                    for chld in t2:
                        if chld.tag == "radius_path":
                            dictionary = ast.literal_eval(chld.text)
                            r2_cur = get_vector_at_frame(dictionary, fr)
                            r2_next = get_vector_at_frame(dictionary, fr + 1)
                        elif chld.tag == "theta_path":
                            dictionary = ast.literal_eval(chld.text)
                            a2_cur = get_vector_at_frame(dictionary, fr)
                            a2_next = get_vector_at_frame(dictionary, fr + 1)
                    x, y = radial_to_tangent(r2_cur, a2_cur)
                    tangent2_cur = Vector(x, y)
                    x, y = radial_to_tangent(r2_next, a2_next)
                    tangent2_next = Vector(x, y)

            # Convert to Lottie format
            tangent1_cur /= 3
            tangent1_next /= 3
            tangent2_cur /= 3
            tangent2_next /= 3

            # Synfig and Lottie use different in tangents SEE DOCUMENTATION
            tangent1_cur *= -1
            tangent1_next *= -1

            # Important: t1 and t2 have to be relative
            # The y-axis is different in lottie
            tangent1_cur.val2 = -tangent1_cur.val2
            tangent2_cur.val2 = -tangent2_cur.val2
            tangent1_next.val2 = -tangent1_next.val2
            tangent2_next.val2 = -tangent2_next.val2

            # Store values in dictionary
            st_val["i"].append([tangent1_cur.val1, tangent1_cur.val2])
            st_val["o"].append([tangent2_cur.val1, tangent2_cur.val2])
            st_val["v"].append(pos_cur)
            en_val["i"].append([tangent1_next.val1, tangent1_next.val2])
            en_val["o"].append([tangent2_next.val1, tangent2_next.val2])
            en_val["v"].append(pos_next)
        fr += 1
        # Setting final time
        lottie.append({})
        lottie[-1]["t"] = fr
