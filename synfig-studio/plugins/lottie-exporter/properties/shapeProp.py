"""
Implements all the functions required for generating the shapeProperties
required for shape/path layer required by lottie
"""

import sys
import settings
from misc import change_axis, get_vector
sys.path.append("../")


def get_entity_val(elem, elem_type):
    """
    Returns the element value, given it's tag and parent

    Args:
        elem      (lxml.etree._Element) : Node at which the value is to be found
        elem_type (str)                 : Tag of the element, which is searched

    Returns:
        (str) : Element value of the tag
    """
    for child in elem:
        if child.tag == elem_type:
            val = child[0].attrib["value"]
    return val


def set_entity_val(elem, elem_type, val):
    """
    Sets the element value, given it's tag and parent

    Args:
        elem      (lxml.etree._Element) : Node at which the value is to be set
        elem_type (str) : Tag of the element, which is seached
        val       (str) : Value to be set

    Returns:
        (None)
    """
    for child in elem:
        if child.tag == elem_type:
            child[0].attrib["value"] = val


def gen_properties_shape_prop(lottie, bline_point):
    """
    Generates the dictionary corresponding to properties/shapeProp.json in
    lottie documentation

    Args:
        lottie      (dict) : Stores some parts of lottie shape layer
        bline_point (lxml.etree._Element) : Stores the shape/path in Synfig format

    Returns:
        (None)
    """
    # Bezier curve in points. Array of 2 dimensional arrays
    lottie["i"] = []
    # Bezier curve out points. Array of 2 dimensional arrays
    lottie["o"] = []
    # Bezier curve vertices. Array of 2 dimensional arrays
    lottie["v"] = []
    for entry in bline_point:
        composite = entry[0]
        for child in composite:
            if child.tag == "point":
                pos = get_vector(child)
            elif child.tag == "t1":
                # t1 serves as t_in
                t1 = child[0]
            elif child.tag == "t2":
                # t2 serves as t_out
                t2 = child[0]
            elif child.tag == "split_radius":
                split_r = child[0].attrib["value"]
            elif child.tag == "split_angle":
                split_a = child[0].attrib["value"]

        if split_r == "false":  # t2 uses the radius of t1
            r1 = get_entity_val(t1, "radius")
            set_entity_val(t2, "radius", r1)

        if split_a == "false":   # t2 uses the angle of t1
            angle1 = get_entity_val(t1, "theta")
            set_entity_val(t2, "theta", angle1)

        tangent1, tangent2 = get_vector(t1), get_vector(t2)

        # Convert to Lottie format
        tangent1 /= 3
        tangent2 /= 3

        # Lottie and synfig use different in tangents SEE DOCUMENTATION
        tangent1 = -1 * tangent1

        # Important: t1 and t2 have to be relative
        # The y-axis is different in lottie
        tangent1.val2 = -tangent1.val2
        tangent2.val2 = -tangent2.val2

        # Convert the point to Lottie axis
        pos *= settings.PIX_PER_UNIT
        pos = change_axis(pos.val1, pos.val2)

        # Store values in dictionary
        lottie["i"].append([tangent1.val1, tangent1.val2])
        lottie["o"].append([tangent2.val1, tangent2.val2])
        lottie["v"].append(pos)
