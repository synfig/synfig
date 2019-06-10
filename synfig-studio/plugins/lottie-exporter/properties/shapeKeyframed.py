"""
Will store all the functions required for generation of shapeKeyframed file in
lottie
"""

import sys
from properties.shapePropKeyframe import gen_properties_shapePropKeyframe
sys.path.append("../")


def gen_properties_shapeKeyframed(lottie, bline_point, idx):
    """
    Will convert bline points to bezier points as required by lottie if they are
    animated

    Args:
        lottie      (dict) : Lottie generated shape keyframes will be stored here
        bline_point (lxml.etree._Element) : Shape/path in Synfig format
        idx         (int) : Index/Count of shape/path

    Returns:
        (None)
    """
    lottie["ix"] = idx
    lottie["a"] = 1
    lottie["k"] = []
    gen_properties_shapePropKeyframe(lottie["k"], bline_point)
