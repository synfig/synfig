# pylint: disable=line-too-long
"""
Will store all the helper functions and modules for generation of shapPropKeyframe file
in Lottie format
"""

import sys
import ast
from lxml import etree
import settings
from misc import change_axis, get_frame, Vector, is_animated, radial_to_tangent
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from synfig.animation import get_vector_at_frame, get_bool_at_frame, gen_dummy_waypoint
sys.path.append("../../")


def append_path(element, parent, element_name, typ="real"):
    """
    Generates a dictionary corresponding to the path followed by that element
    and appends it at the parent which will be needed later
    """
    # Generating the path and store in the lxml element
    element_dict = {}
    if typ == "real":
        gen_value_Keyframed(element_dict, element, 0)
    else:
        gen_properties_multi_dimensional_keyframed(element_dict, element, 0)
    # Store in lxml element
    element_lxml = etree.Element(element_name)
    element_lxml.text = str(element_dict)
    parent.append(element_lxml)


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

    append_path(radius[0], radial_composite, "radius_path")
    append_path(theta[0], radial_composite, "theta_path")


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


def add(side, lottie, origin_cur):
    """
    Does not take care of tangent putting order because the tangents are all
    zero for now according to the code

    Args:
        side (list) : Stores the newly calculated vertex and tangents of outline layer
        lottie (dict) : These vertices will be stored in this dictionary
        origin_cur (list) : Value of origin at specific frame

    Returns:
        (None)
    """
    i = 0
    while i < len(side):
        cubic_to(side[i][0], side[i][1], side[i][2], lottie, origin_cur)
        i += 1


def add_reverse(side, lottie, origin_cur):
    """
    Does not take care of tangents putting order because the tangents are all
    zero for now according to the code:

    Args:
        side (list) : Stores the newly calculated vertex and tangents of outline layer
        lottie (dict) : These vertices will be stored in this dictionary
        origin_cur (list) : Value of origin at specific frame

    Returns:
        (None)
    """
    i = len(side) - 1
    while i >= 0:
        cubic_to(side[i][0], side[i][1], side[i][2], lottie, origin_cur)
        i -= 1


def cubic_to(vec, tan1, tan2, lottie, origin_cur):
    """
    Will have to manipulate the tangents here, but they are not managed as tan1
    and tan2 both are zero always

    Args:
        vec (misc.Vector) : position of the point
        tan1 (misc.Vector) : tangent 1 of the point
        tan2 (misc.Vector) : tangent 2 of the point
        lottie (dict) : Final position and tangents will be stored here
        origin_cur (list) : value of the origin at specific frame

    Returns:
        (None)
    """
    vec *= settings.PIX_PER_UNIT
    tan1 *= settings.PIX_PER_UNIT
    tan2 *= settings.PIX_PER_UNIT
    tan1, tan2 = convert_tangent_to_lottie(3*tan1, 3*tan2)
    pos = change_axis(vec[0], vec[1])
    for i in range(len(pos)):
        pos[i] += origin_cur[i]
    lottie["i"].append(round_to(tan1.get_list()))
    lottie["o"].append(round_to(tan2.get_list()))
    lottie["v"].append(round_to(pos))


def move_to(vec, lottie, origin_cur):
    """
    Don't have to manipulate the tangents because all of them are zero here

    Args:
        vec (misc.Vector) : position of the point
        lottie (dict) : Final position and tangents will be stored here
        origin_cur (list) : value of the origin at specific frame

    Returns:
        (None)
    """
    vec *= settings.PIX_PER_UNIT
    lottie["i"].append([0, 0])
    lottie["o"].append([0, 0])
    pos = change_axis(vec[0], vec[1])
    for i in range(len(pos)):
        pos[i] += origin_cur[i]
    lottie["v"].append(round_to(pos))


def round_to(my_list, decimal_places=3):
    """
    rounds the provided list to the given decimal places and returns it
    """
    ret = []
    for num in my_list:
        ret.append(round(num, decimal_places))
    return ret


def convert_tangent_to_lottie(t1, t2):
    """
    Converts tangent from Synfig format to lottie format

    Args:
        t1 (misc.Vector) : tangent 1 of a point
        t2 (misc.Vector) : tangent 2 of a point

    Returns:
        (misc.Vector) : Converted tangent 1
        (misc.Vector) : Converted tangent 2
    """
    # Convert to Lottie format
    t1 /= 3
    t2 /= 3

    # Synfig and Lottie use different in tangents SEE DOCUMENTATION
    t1 *= -1

    # Important: t1 and t2 have to be relative
    # The y-axis is different in lottie
    t1[1] = -t1[1]
    t2[1] = -t2[1]
    return t1, t2


def insert_dict_at(lottie, idx, fr, loop):
    """
    Inserts dictionary values in the main dictionary, required by shape layer of
    lottie format

    Args:
        lottie (dict) : Shape layer will be stored in this main dictionary
        idx    (int)  : index at which dicionary needs to be stored
        fr     (int)  : frame number
        loop   (bool) : Specifies if the shape is loop or not

    Returns:
        (dict) : Starting dictionary for shape interpolation
        (dict) : Ending dictionary for shape interpolation
    """
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


def quadratic_to_cubic(qp0, qp1, qp2):
    """
    Converts quadratic bezier curve to cubic bezier curve
    """
    cp0 = qp0
    cp3 = qp2
    cp1 = qp0 + 2/3*(qp1 - qp0)
    cp2 = qp2 + 2/3*(qp1 - qp2)
    return cp1, cp2
