"""
Implements all the functions required for generating shape for properties
related to layers in Synfig
"""

import sys
from properties.shapeProp import gen_properties_shape_prop
sys.path.append("../")


def gen_properties_shape(lottie, bline_point, idx):
    """
    Will convert bline points to bezier points as required by lottie

    Args:
        lottie      (dict) : Will hold the shape property required by lottie
        bline_point (lxml.etree._Element) : Hold the bline parameter of Synfig format
        idx         (int) : Index/Count of shape property

    Returns:
        (None)
    """
    lottie["ix"] = idx
    lottie["a"] = 0
    lottie["c"] = False     # By default the path is not closed in synfig
    lottie["k"] = {}
    gen_properties_shape_prop(lottie["k"], bline_point)
