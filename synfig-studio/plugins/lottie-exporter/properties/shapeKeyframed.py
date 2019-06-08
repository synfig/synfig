"""
"""

import sys
from properties.shapePropKeyframe import gen_properties_shapePropKeyframe
sys.path.append("../")


def gen_properties_shapeKeyframed(lottie, bline_point, idx):
    """
    Will convert bline points to bezier points as required by lottie if they are
    animated
    """
    lottie["ix"] = idx
    lottie["a"] = 1
    lottie["k"] = []
    gen_properties_shapePropKeyframe(lottie["k"], bline_point)
