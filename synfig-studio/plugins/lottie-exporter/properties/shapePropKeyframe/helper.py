# pylint: disable=line-too-long
"""
Will store all the helper functions and modules for generation of shapPropKeyframe file
in Lottie format
"""

import sys
import ast
from lxml import etree
import settings
from common.misc import change_axis, get_frame, is_animated, radial_to_tangent
from common.Vector import Vector
from common.Param import Param
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
from synfig.animation import print_animation, get_vector_at_frame, get_bool_at_frame
sys.path.append("../../")


def animate_tangents(tangent, window):
    """
    Animates the radial composite and updates the window of frame if radial
    composite's parameters are already animated
    Also generate the Lottie path and stores in radial_composite

    Args:
        tangent (common.Param.Param)  : Synfig format radial composite's parent-> stores radius and angle
        window  (dict)                : max and min frame of overall animations stored in this

    Returns:
        (None)
    """
    for child in tangent[0]:
        # Assuming tangent[0] is always radial_composite
        if child.tag == "radius":
            radius = Param(child, tangent[0])
        elif child.tag == "theta":
            theta = Param(child, tangent[0])
    radius.update_frame_window(window)
    theta.update_frame_window(window)

    radius.animate("real")
    theta.animate("region_angle")

    tangent.add_subparam("radius", radius)
    tangent.add_subparam("theta", theta)


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
        t1      (common.Param.Param)  : Holds Tangent 1/In Tangent
        t2      (common.Param.Param)  : Holds Tangent 2/Out Tangent
        split_r (common.Param.Param) : Holds animation of split radius parameter
        split_a (common.Param.Param) : Holds animation of split angle parameter
        fr      (int)                 : Holds the frame value

    Returns:
        (common.Vector.Vector, common.Vector.Vector) : In-tangent and out-tangent at the given frame
    """

    # Get value of split_radius and split_angle at frame
    sp_r = split_r.get_value(fr)
    sp_a = split_a.get_value(fr)

    # Setting tangent 1
    r1 = t1.get_subparam("radius").get_value(fr)
    a1 = t1.get_subparam("theta").get_value(fr)

    x, y = radial_to_tangent(r1, a1)
    tangent1 = Vector(x, y)

    # Setting tangent 2
    r2 = t2.get_subparam("radius").get_value(fr)
    a2 = t2.get_subparam("theta").get_value(fr)

    x, y = radial_to_tangent(r2, a2)
    orig_tang2 = Vector(x, y)

    if not sp_r:
        # Use t1's radius
        r2 = r1
    if not sp_a:
        # Use t1's angle
        a2 = a1

    x, y = radial_to_tangent(r2, a2)
    tangent2 = Vector(x, y)

    if sp_r and (not sp_a):
        if tangent1.mag_squared() == 0:
            tangent2 = orig_tang2

    return tangent1, tangent2


def add(side, lottie, origin_cur, is_rectangle=False):
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
        cubic_to(side[i][0], side[i][1], side[i][2], lottie, origin_cur, is_rectangle)
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


def cubic_to(vec, tan1, tan2, lottie, origin_cur, is_rectangle=False):
    """
    Will have to manipulate the tangents here, but they are not managed as tan1
    and tan2 both are zero always

    Args:
        vec (common.Vector.Vector) : position of the point
        tan1 (common.Vector.Vector) : tangent 1 of the point
        tan2 (common.Vector.Vector) : tangent 2 of the point
        lottie (dict) : Final position and tangents will be stored here
        origin_cur (list) : value of the origin at specific frame

    Returns:
        (None)
    """
    vec *= settings.PIX_PER_UNIT
    tan1 *= settings.PIX_PER_UNIT
    tan2 *= settings.PIX_PER_UNIT
    tan1, tan2 = convert_tangent_to_lottie(3*tan1, 3*tan2)
    pos = change_axis(vec[0], vec[1], not is_rectangle)
    for i in range(len(pos)):
        pos[i] += origin_cur[i]
    lottie["i"].append(tan1.get_list())
    lottie["o"].append(tan2.get_list())
    lottie["v"].append(pos)


def move_to(vec, lottie, origin_cur):
    """
    Don't have to manipulate the tangents because all of them are zero here

    Args:
        vec (common.Vector.Vector) : position of the point
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
    lottie["v"].append(pos)


def convert_tangent_to_lottie(t1, t2):
    """
    Converts tangent from Synfig format to lottie format

    Args:
        t1 (common.Vector.Vector) : tangent 1 of a point
        t2 (common.Vector.Vector) : tangent 2 of a point

    Returns:
        (common.Vector.Vector) : Converted tangent 1
        (common.Vector.Vector) : Converted tangent 2
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

    Args:
        qp0 (common.Vector.Vector) First control point of quadratic bezier
        qp1 (common.Vector.Vector) Second control point of quadratic bezier
        qp2 (common.Vector.Vector) Third control point of quadratic bezier

    Returns:
        (common.Vector.Vector) Second control point of Cubic bezier
        (common.Vector.Vector) Third control point of Cubic bezier
    """
    cp0 = qp0
    cp3 = qp2
    cp1 = qp0 + 2/3*(qp1 - qp0)
    cp2 = qp2 + 2/3*(qp1 - qp2)
    return cp1, cp2
