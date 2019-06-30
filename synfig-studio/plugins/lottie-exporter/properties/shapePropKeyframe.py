# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of shapPropKeyframe file
in Lottie format
"""

import sys
import ast
import copy
import settings
from lxml import etree
from synfig.animation import to_Synfig_axis
from misc import change_axis, get_frame, Vector, Hermite, is_animated, radial_to_tangent
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from synfig.animation import print_animation, get_vector_at_frame, get_bool_at_frame, gen_dummy_waypoint
sys.path.append("../")


def animate_radial_composite(radial_composite, window):
    """
    Animates the radial composite and updates the window of frame if radial
    composite's parameters are already animated
    Also generate the Lottie path and stores in radial_composite

    Args:
        radial_composite (lxml.etree._Element) : Synfig format radial composite-> stores radius and angle
        window           (dict)                : max and min frame of overall animations stored in this

    Returns:
        (None)
    """
    for child in radial_composite:
        if child.tag == "radius":
            radius = child
        elif child.tag == "theta":
            theta = child
    update_frame_window(radius[0], window)
    update_frame_window(theta[0], window)

    radius = gen_dummy_waypoint(radius, "radius", "real")
    theta = gen_dummy_waypoint(theta, "theta", "region_angle")

    # Update the newly computed radius and theta
    update_child_at_parent(radial_composite, radius, "radius")
    update_child_at_parent(radial_composite, theta, "theta")

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
    """
    Given an animation, finds the minimum and maximum frame at which the
    waypoints are located

    Args:
        node    (lxml.etree._Element) : Animation to be searched in
        window  (dict)                : max and min frame will be stored in this

    Returns:
        (None)
    """
    if is_animated(node) == 2:
        for waypoint in node:
            fr = get_frame(waypoint)
            if fr > window["last"]:
                window["last"] = fr
            if fr < window["first"]:
                window["first"] = fr


def update_child_at_parent(parent, new_child, tag, param_name=None):
    """
    Given a node, replaces the child with tag `tag` with new_child

    Args:
        parent    (lxml.etree._Element) : Node whose child needs to be replaced
        new_child (lxml.etree._Element) : Updated child of the parent
        tag       (str)                 : To identify the appropriate child

    Returns:
        (None)
    """
    if not param_name:
        for chld in parent:
            if chld.tag == tag:
                chld.getparent().remove(chld)
    else:
        for chld in parent:
            if chld.tag == tag and chld.attrib["name"] == param_name:
                chld.getparent().remove(chld)
    parent.insert(0, new_child)


def get_tangent_at_frame(t1, t2, split_r, split_a, fr):
    """
    Given a frame, returns the in-tangent and out-tangent at a bline point
    depending on whether split_radius and split_angle is "true"/"false"

    Args:
        t1      (lxml.etree._Element) : Holds Tangent 1/In Tangent
        t2      (lxml.etree._Element) : Holds Tangent 2/Out Tangent
        split_r (lxml.etree._Element) : Holds animation of split radius parameter
        split_a (lxml.etree._Element) : Holds animation of split angle parameter
        fr      (int)                 : Holds the frame value

    Returns:
        (misc.Vector, misc.Vector) : In-tangent and out-tangent at the given frame
    """

    # Get value of split_radius and split_angle at frame
    sp_r = get_bool_at_frame(split_r[0], fr)
    sp_a = get_bool_at_frame(split_a[0], fr)

    # Setting tangent 1
    for chld in t1:
        if chld.tag == "radius_path":
            dictionary = ast.literal_eval(chld.text)
            r1 = get_vector_at_frame(dictionary, fr)
        elif chld.tag == "theta_path":
            dictionary = ast.literal_eval(chld.text)
            a1 = get_vector_at_frame(dictionary, fr)
    x, y = radial_to_tangent(r1, a1)
    tangent1 = Vector(x, y)

    # Setting tangent 2
    for chld in t2:
        if chld.tag == "radius_path":
            dictionary = ast.literal_eval(chld.text)
            r2 = get_vector_at_frame(dictionary, fr)
            if not sp_r:
                # Use t1's radius
                r2 = r1
        elif chld.tag == "theta_path":
            dictionary = ast.literal_eval(chld.text)
            a2 = get_vector_at_frame(dictionary, fr)
            if not sp_a:
                # Use t1's angle
                a2 = a1
    x, y = radial_to_tangent(r2, a2)
    tangent2 = Vector(x, y)
    return tangent1, tangent2


def gen_bline_shapePropKeyframe(lottie, bline_point, origin):
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

        # Generate path for Lottie format
        path_dict = {}
        gen_properties_multi_dimensional_keyframed(path_dict, pos[0], 0)
        # Store in lxml element
        path_lxml = etree.Element("point_path")
        path_lxml.text = str(path_dict)
        composite.append(path_lxml)

        animate_radial_composite(t1[0], window)
        animate_radial_composite(t2[0], window)

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

            # Adding origin to each vertex
            origin_cur = get_vector_at_frame(origin_dict, fr)
            origin_next = get_vector_at_frame(origin_dict, fr + 1)
            for i in range(len(pos_cur)):
                pos_cur[i] += origin_cur[i]
            for i in range(len(pos_next)):
                pos_next[i] += origin_next[i]

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


def gen_dynamic_list_shapePropKeyframe(lottie, dynamic_list, origin):
    """
    Generates the bline corresponding to polygon layer 
    """
    ################## SECTION 1 ################
    # Inserting the waypoints if not animated, finding the first and last frame
    # Calculating the path after this
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1
    count = 0

    for entry in dynamic_list:
        pos = entry
        update_frame_window(pos[0], window)

        new_pos = gen_dummy_waypoint(pos, "entry", "vector")
        pos.getparent().remove(pos)
        dynamic_list.insert(count, new_pos)

        # Generate path for lottie format
        path_dict = {}
        gen_properties_multi_dimensional_keyframed(path_dict, new_pos[0], 0)
        # Store in lxml element
        path_lxml = etree.Element("pos_path")
        path_lxml.text = str(path_dict)
        dynamic_list[count].append(path_lxml)
        count += 1

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

    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################ END OF SECTION 1 ##############

    ################ SECTION 2 #####################
    # Generating values for all the frames in the window
    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)
        
        for entry in dynamic_list:
            # Only two childs, one should be animated, other one is path
            for child in entry:
                if child.tag == "pos_path":
                    dictionary = ast.literal_eval(child.text)
                    pos_cur = get_vector_at_frame(dictionary, fr)
                    pos_next = get_vector_at_frame(dictionary, fr + 1)

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
            st_val["i"].append([tangent1_cur.val1, tangent1_cur.val2])
            st_val["o"].append([tangent2_cur.val1, tangent2_cur.val2])
            st_val["v"].append(pos_cur)
            en_val["i"].append([tangent1_next.val1, tangent1_next.val2])
            en_val["o"].append([tangent2_next.val1, tangent2_next.val2])
            en_val["v"].append(pos_next)
        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def gen_bline_outline(lottie, bline_point, origin):
    """
    Generates the bline corresponding to outline layer by adding some vertices
    to bline and converting it to region layer

    Some parts are common with gen_bline_shapePropKeyframe(), which will be
    placed in a common function latter
    """ 
    ################### SECTION 1 #########################
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
            elif child.tag == "width":
                width = child
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

        # Empty the width and fill in the new animated width
        update_frame_window(width[0], window)
        width = gen_dummy_waypoint(width, "width", "real")
        update_child_at_parent(composite, width, "width")

        update_frame_window(split_r[0], window)
        split_r = gen_dummy_waypoint(split_r, "split_radius", "bool")
        update_child_at_parent(composite, split_r, "split_radius")

        update_frame_window(split_a[0], window)
        split_a = gen_dummy_waypoint(split_a, "split_angle", "bool")
        update_child_at_parent(composite, split_a, "split_angle")

        # Generate path for Lottie format
        path_dict = {}
        gen_properties_multi_dimensional_keyframed(path_dict, pos[0], 0)
        # Store in lxml element
        path_lxml = etree.Element("point_path")
        path_lxml.text = str(path_dict)
        composite.append(path_lxml)

        # Generate width for Lottie format
        width_dict = {}
        gen_value_Keyframed(width_dict, width[0], 0)
        # Store in lxml element
        width_lxml = etree.Element("width_path")
        width_lxml.text = str(width_dict)
        composite.append(width_lxml)

        animate_radial_composite(t1[0], window)
        animate_radial_composite(t2[0], window)


    # Animating the outermost outline width
    layer = bline_point.getparent().getparent()
    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "width":
                outer_width = chld
            elif chld.attrib["name"] == "sharp_cusps":
                sharp_cusps = chld
            elif chld.attrib["name"] == "expand":
                expand = chld
            elif chld.attrib["name"] == "round_tip[0]":
                r_tip0 = chld
            elif chld.attrib["name"] == "round_tip[1]":
                r_tip1 = chld
            elif chld.attrib["name"] == "homogeneous_width":
                homo_width = chld

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

    update_frame_window(outer_width[0], window)
    outer_width = gen_dummy_waypoint(outer_width, "param", "real")
    outer_width.attrib["name"] = "width"
    # Update the layer with this animated outline width
    update_child_at_parent(layer, outer_width, "param", "width")

    # Generate outline width for Lottie format
    # No need to store this dictionary in lxml element, as it will be used in this function and will not be rewritten
    outer_width_dict = {}
    gen_value_Keyframed(outer_width_dict, outer_width[0], 0)

    # Animating the sharp_cusps
    update_frame_window(sharp_cusps[0], window)
    sharp_cusps = gen_dummy_waypoint(sharp_cusps, "param", "bool")
    sharp_cusps.attrib["name"] = "sharp_cusps"

    # Update the layer with this animated outline sharp cusps
    update_child_at_parent(layer, sharp_cusps, "param", "sharp_cusps")

    update_frame_window(expand[0], window)
    expand = gen_dummy_waypoint(expand, "param", "real")
    expand.attrib["name"] = "expand"
    update_child_at_parent(layer, expand, "param", "expand")
    expand_dict = {}
    gen_value_Keyframed(expand_dict, expand[0], 0)

    update_frame_window(r_tip0[0], window)
    r_tip0 = gen_dummy_waypoint(r_tip0, "param", "bool")
    r_tip0.attrib["name"] = "round_tip[0]"
    update_child_at_parent(layer, r_tip0, "param", "round_tip[0]")

    update_frame_window(r_tip1[0], window)
    r_tip1 = gen_dummy_waypoint(r_tip1, "param", "bool")
    r_tip1.attrib["name"] = "round_tip[1]"
    update_child_at_parent(layer, r_tip1, "param", "round_tip[1]")

    update_frame_window(homo_width[0], window)
    homo_width = gen_dummy_waypoint(homo_width, "param", "bool")
    homo_width.attrib["name"] = "homogeneous_width"
    update_child_at_parent(layer, homo_width, "param", "homogeneous_width")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window


    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)  # This loop needs to be considered somewhere down

        synfig_outline(bline_point, st_val, origin_dict, outer_width_dict, sharp_cusps,
                       expand_dict, r_tip0, r_tip1, homo_width, fr)
        synfig_outline(bline_point, en_val, origin_dict, outer_width_dict, sharp_cusps,
                       expand_dict, r_tip0, r_tip1, homo_width, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr


def get_outline_param_at_frame(composite, fr):
    for child in composite:
        if child.tag == "point_path":
            dictionary = ast.literal_eval(child.text)
            pos = get_vector_at_frame(dictionary, fr)
        elif child.tag == "width_path":
            dictionary = ast.literal_eval(child.text)
            width = to_Synfig_axis(get_vector_at_frame(dictionary, fr), "real")
        elif child.tag == "t1":
            t1 = child[0]
        elif child.tag == "t2":
            t2 = child[0]
        elif child.tag == "split_radius":
            split_r = child
        elif child.tag == "split_angle":
            split_a = child

    # Convert pos back to Synfig coordinates
    pos = to_Synfig_axis(pos, "vector")
    pos_ret = Vector(pos[0], pos[1])

    t1, t2 = get_tangent_at_frame(t1, t2, split_r, split_a, fr)
    # Convert to Synfig units
    t1 /= settings.PIX_PER_UNIT
    t2 /= settings.PIX_PER_UNIT

    split_r_val = get_bool_at_frame(split_r[0], fr)
    split_a_val = get_bool_at_frame(split_a[0], fr)

    return pos_ret, width, t1, t2, split_r_val, split_a_val


def synfig_outline(bline_point, st_val, origin_dict, outer_width_dict, sharp_cusps_anim, expand_dict, r_tip0_anim, r_tip1_anim, homo_width_anim, fr):
    """
    calculate for only frame, then in the parent function call for fr + 1
    """

    EPSILON = 0.000000001
    SAMPLES = 50
    CUSP_TANGENT_ADJUST = 0.025
    CUSP_THRESHOLD = 0.40
    SPIKE_AMOUNT = 4
    ROUND_END_FACTOR = 4
    outer_width = to_Synfig_axis(get_vector_at_frame(outer_width_dict, fr), "real")
    expand = to_Synfig_axis(get_vector_at_frame(expand_dict, fr), "real")
    sharp_cusps = get_bool_at_frame(sharp_cusps_anim[0], fr)
    r_tip0 = get_bool_at_frame(r_tip0_anim[0], fr)
    r_tip1 = get_bool_at_frame(r_tip1_anim[0], fr)
    homo_width = get_bool_at_frame(homo_width_anim[0], fr)

    # Setup chunk list
    side_a, side_b = [], []

    # Check if looped
    loop = False
    if "loop" in bline_point.keys() and bline_point.attrib["loop"] == "true":
        loop = True
    
    # Iterators
    end_it = len(bline_point)
    next_it = 0
    if loop:
        iter_it = end_it - 1
    else:
        iter_it = next_it
        next_it += 1

    first_point = bline_point[iter_it][0]
    first_tangent = get_outline_param_at_frame(bline_point[0][0], fr)[3]
    last_tangent = get_outline_param_at_frame(first_point, fr)[2]

    # If we are looped and drawing sharp cusps, we 'll need a value
    # for the incoming tangent. This code fails if we have a "degraded" spline
    # with just one vertex, so we avoid such case.
    if loop and sharp_cusps and last_tangent.is_equal_to(Vector(0, 0)) and len(bline_point) > 1:
        prev_it = iter_it
        prev_it -= 1
        prev_it %= len(bline_point)
        prev_point = bline_point[prev_it][0]
        curve = Hermite(get_outline_param_at_frame(prev_point, fr)[0],
                        get_outline_param_at_frame(first_point, fr)[0],
                        get_outline_param_at_frame(prev_point, fr)[3],
                        get_outline_param_at_frame(first_point, fr)[2])
        last_tangent = curve.derivative(1.0 - CUSP_TANGENT_ADJUST)

    first = not loop
    while next_it != end_it:
        bp1 = bline_point[iter_it][0] 
        bp2 = bline_point[next_it][0]

        # Calculate current vertex and next vertex parameters
        pos_c, width_c, t1_c, t2_c, split_r_c, split_a_c = get_outline_param_at_frame(bp1, fr)
        pos_n, width_n, t1_n, t2_n, split_r_n, split_a_n = get_outline_param_at_frame(bp2, fr)

        # Setup tangents
        prev_t = t1_c
        iter_t = t2_c
        next_t = t1_n

        split_flag = split_a_c or split_r_c

        # If iter_it.t2 == 0 and next.t1 == 0, this is a straight line
        if iter_t.is_equal_to(Vector(0, 0)) and next_t.is_equal_to(Vector(0, 0)):
            iter_t = next_t = pos_n - pos_c

            # If the 2 points are on top of each other, ignore this segment
            # leave 'first' true if was before
            if iter_t.is_equal_to(Vector(0, 0)):
                iter_it = next_it
                iter_it %= len(bline_point)
                next_it += 1
                continue

        # Setup the curve
        curve = Hermite(pos_c, pos_n, iter_t, next_t)

        # Setup width's
        iter_w = width_c * outer_width * 0.5 + expand 
        next_w = width_n * outer_width * 0.5 + expand

        if first:
            first_tangent = curve.derivative(CUSP_TANGENT_ADJUST)

        # Make cusps as necassary
        if not first and \
           sharp_cusps and \
           split_flag and \
           ((not prev_t.is_equal_to(iter_t)) or (iter_t.is_equal_to(Vector(0, 0)))) and \
           (not last_tangent.is_equal_to(Vector(0, 0))):

            curr_tangent = curve.derivative(CUSP_TANGENT_ADJUST) 
            t1 = last_tangent.perp().norm()
            t2 = curr_tangent.perp().norm()

            cross = t1*t2.perp()
            perp = (t1 - t2).mag()
            if cross > CUSP_THRESHOLD:
                p1 = pos_c + t1*iter_w
                p2 = pos_c + t2*iter_w
                side_a.append([line_intersection(p1, last_tangent, p2, curr_tangent), Vector(0, 0), Vector(0, 0)])
            elif cross < -CUSP_THRESHOLD:
                p1 = pos_c - t1*iter_w
                p2 = pos_c - t2*iter_w
                side_b.append([line_intersection(p1, last_tangent, p2, curr_tangent), Vector(0, 0), Vector(0, 0)])
            elif cross > 0.0 and perp > 1.0:
                amount = max(0.0, cross/CUSP_THRESHOLD) * (SPIKE_AMOUNT - 1.0) + 1.0
                side_a.append([pos_c + (t1 + t2).norm()*iter_w*amount, Vector(0, 0), Vector(0, 0)])
            elif cross < 0 and perp > 1:
                amount = max(0.0, -cross/CUSP_THRESHOLD) * (SPIKE_AMOUNT - 1.0) + 1.0
                side_b.append([pos_c - (t1 + t2).norm()*iter_w*amount, Vector(0, 0), Vector(0, 0)])

        # Precalculate positions and coefficients
        length = 0.0
        points = []
        dists = []
        n = 0.0
        itr = 0
        while n < 1.000001:
            points.append(curve.value(n)) 
            if n:
                length += (points[itr] - points[itr-1]).mag()
            dists.append(length)

            n += 1.0/SAMPLES
            itr += 1
        length += (curve.value(1) - points[itr-1]).mag()

        div_length = 1
        if length > EPSILON:
            div_length = 1.0 / length

        # Might not need /3 for the tangents genereated finally - VERY IMPORTANT
        # Make the outline
        pt = curve.derivative(CUSP_TANGENT_ADJUST) / 3
        n = 0.0
        itr = 0
        while n < 1.000001:
            t = curve.derivative(min(max(n, CUSP_TANGENT_ADJUST), 1.0 - CUSP_TANGENT_ADJUST)) / 3 
            d = t.perp().norm()
            k = dists[itr] * div_length
            if not homo_width:
                k = n
            w = (next_w - iter_w)*k + iter_w
            if False and n:
                # create curve
                a = points[itr-1] + d*w
                b = points[itr] + d*w
                tk = (b - a).mag() * div_length
                side_a.append([b, pt*tk, -t*tk])

                a = points[itr-1] - d*w
                b = points[itr] - d*w
                tk = (b - a).mag() * div_length
                side_b.append([b, pt*tk, -t*tk])
            else:
                side_a.append([points[itr] + d*w, Vector(0, 0), Vector(0, 0)])
                side_b.append([points[itr] - d*w, Vector(0, 0), Vector(0, 0)])
            pt = t
            itr += 1
            n += 1.0/SAMPLES

        last_tangent = curve.derivative(1.0 - CUSP_TANGENT_ADJUST)
        side_a.append([curve.value(1.0) + last_tangent.perp().norm()*next_w, Vector(0, 0), Vector(0, 0)])
        side_b.append([curve.value(1.0) - last_tangent.perp().norm()*next_w, Vector(0, 0), Vector(0, 0)])
        first = False

        iter_it = next_it
        iter_it %= len(bline_point)
        next_it += 1

    if len(side_a) < 2 or len(side_b) < 2:
        return

    origin_cur = get_vector_at_frame(origin_dict, fr)
    move_to(side_a[0][0], st_val, origin_cur)

    if loop:
        add(side_a, st_val, origin_cur)
        add_reverse(side_b, st_val, origin_cur)
    else:
        # Insert code for adding end tip
        if r_tip1:
            bp = bline_point[-1][0]
            vertex = get_outline_param_at_frame(bp, fr)[0]
            tangent = last_tangent.norm()
            w = get_outline_param_at_frame(bp, fr)[1] * outer_width * 0.5 + expand

            a = vertex + tangent.perp()*w
            b = vertex - tangent.perp()*w
            p1 = a + tangent*w*(ROUND_END_FACTOR/3.0)
            p2 = b + tangent*w*(ROUND_END_FACTOR/3.0)

            # replace the last point
            side_a[-1] = [a, Vector(0, 0), Vector(0, 0)]
            add(side_a, st_val, origin_cur)
            #add([[b, Vector(0, 0), Vector(0, 0)]], st_val, origin_cur)
            add([[b, p1, p2 - b]], st_val, origin_cur)
        else:
            add(side_a, st_val, origin_cur)

        # Insert code for adding beginning tip
        if r_tip0:
            bp = bline_point[0][0]
            vertex = get_outline_param_at_frame(bp, fr)[0]
            tangent = first_tangent.norm()
            w = get_outline_param_at_frame(bp, fr)[1] * outer_width * 0.5 + expand

            a = vertex - tangent.perp()*w
            b = vertex + tangent.perp()*w
            p1 = a - tangent*w*(ROUND_END_FACTOR/3.0)
            p2 = b - tangent*w*(ROUND_END_FACTOR/3.0)

            # replace the first point
            side_b[0] = [a, Vector(0, 0), Vector(0, 0)]
            add_reverse(side_b, st_val, origin_cur)
            add([[b, p1, p2]], st_val, origin_cur)
        else:
            add_reverse(side_b, st_val, origin_cur)
         

def add(side, lottie, origin_cur):
    """
    Does not take care of tangent putting order because the tangents are all
    zero for now according to the code
    """
    i = 0
    while i + 1 < len(side):
        cubic_to(side[i][0], side[i][2], side[i+1][1], lottie, origin_cur)
        i += 1
    cubic_to(side[i][0], side[i][2], Vector(0, 0), lottie, origin_cur)


def add_reverse(side, lottie, origin_cur):
    """
    Does not take care of tangents putting order because the tangents are all
    zero for now according to the code:
    """
    i = len(side) - 1
    while i > 0:
        cubic_to(side[i][0], side[i][2], side[i-1][1], lottie, origin_cur)
        i -= 1
    cubic_to(side[i][0], side[i][2], Vector(0, 0), lottie, origin_cur)
    """
    for val in reversed(side):
        move_to(val[0], lottie, origin_cur)
    """


def cubic_to(vec, tan1, tan2, lottie, origin_cur):
    """
    Will have to manipulate the tangents here, but they are not managed as tan1
    and tan2 both are zero always
    """
    vec *= settings.PIX_PER_UNIT
    tan1 *= settings.PIX_PER_UNIT
    tan2 *= settings.PIX_PER_UNIT
    lottie["i"].append(tan1.get_list())
    lottie["o"].append(tan2.get_list())
    pos = change_axis(vec.val1, vec.val2)
    for i in range(len(pos)):
        pos[i] += origin_cur[i]
    lottie["v"].append(pos)


def move_to(vec, lottie, origin_cur):
    """
    Don't have to manipulate the tangents because all of them are zero here
    """
    vec *= settings.PIX_PER_UNIT
    lottie["i"].append([0, 0])
    lottie["o"].append([0, 0])
    pos = change_axis(vec.val1, vec.val2)
    for i in range(len(pos)):
        pos[i] += origin_cur[i]
    lottie["v"].append(pos)


def convert_tangent_to_lottie(t1, t2):
    # Convert to Lottie format
    t1 /= 3
    t2 /= 3

    # Synfig and Lottie use different in tangents SEE DOCUMENTATION
    t1 *= -1

    # Important: t1 and t2 have to be relative
    # The y-axis is different in lottie
    t1.val2 = -t1.val2
    t2.val2 = -t2.val2
    return t1, t2


def insert_dict_at(lottie, idx, fr, loop):
    if idx != -1:
        lottie.insert(idx, {})
    else:
        lottie.append({})
    lottie[idx]["i"], lottie[idx]["o"] = {}, {}
    lottie[idx]["i"]["x"] = lottie[idx]["i"]["y"] = 0.5     # Does not matter because frames are adjacent
    lottie[idx]["o"]["x"] = lottie[idx]["o"]["y"] = 0.5     # Does not matter because frames are adjacent
    lottie[-1]["t"] = fr
    lottie[idx]["s"], lottie[idx]["e"] = [], []
    st_val, en_val = lottie[idx]["s"], lottie[idx]["e"]
    
    st_val.append({}), en_val.append({})
    st_val, en_val = st_val[0], en_val[0]
    st_val["i"], st_val["o"], st_val["v"], st_val["c"] = [], [], [], loop
    en_val["i"], en_val["o"], en_val["v"], en_val["c"] = [], [], [], loop
    return st_val, en_val

def line_intersection(p1, t1, p2, t2):
    """
    This function was adapted from what was
    described on http://www.whisqu.se/per/docs/math28.htm
    """
    x0 = p1.val1
    y0 = p1.val2
    
    x1 = p1.val1 + t1.val1
    y1 = p1.val2 + t1.val2

    x2 = p2.val1
    y2 = p2.val2

    x3 = p2.val1 + t2.val1
    y3 = p2.val2 + t2.val2

    near_infinity = 1e+10

    # compute slopes, not the kluge for infinity, however, this will
    # be close enough

    if (x1 - x0) != 0:
        m1 = (y1 - y0) / (x1 - x0)
    else:
        m1 = near_infinity

    if (x3 - x2) != 0:
        m2 = (y3 - y2) / (x3 - x2)
    else:
        near_infinity

    a1 = m1
    a2 = m2
    b1 = -1.0
    b2 = -1.0
    c1 = y0 - m1*x0
    c2 = y2 - m2*x2

    # compute the inverse of the determinate
    det_inv = 1.0 / (a1*b2 - a2*b1)

    # Use kramer's rule to compute intersection
    return Vector((b1*c2 - b2*c1)*det_inv, (a2*c1 - a1*c2)*det_inv)
