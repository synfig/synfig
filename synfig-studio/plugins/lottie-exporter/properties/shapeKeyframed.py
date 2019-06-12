"""
Will store all the functions required for generation of shapeKeyframed file in
lottie
"""

import sys
from properties.shapePropKeyframe import gen_bline_shapePropKeyframe, gen_dynamic_list_shapePropKeyframe
sys.path.append("../")


def gen_properties_shapeKeyframed(lottie, node, idx):
    """
    Will convert bline points/dynamic_list to bezier points as required by lottie if they are
    animated

    Args:
        lottie      (dict) : Lottie generated shape keyframes will be stored here
        node        (lxml.etree._Element) : Shape/path in Synfig format :- Could be bline_point or dynamic_list
        idx         (int) : Index/Count of shape/path

    Returns:
        (None)
    """
    lottie["ix"] = idx
    lottie["a"] = 1
    lottie["k"] = []
    if node.tag == "bline":
        gen_bline_shapePropKeyframe(lottie["k"], node)
    elif node.tag == "dynamic_list":
        gen_dynamic_list_shapePropKeyframe(lottie["k"], node)
