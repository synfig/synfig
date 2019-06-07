"""
"""

import sys
import settings
from properties.shapeProp import gen_properties_shape_prop
sys.path.append("../")


def gen_properties_shape(lottie, bline_point, idx):
    """
    Will convert bline points to bezier points as required by lottie
    """
    lottie["ix"] = idx
    lottie["a"] = 0
    lottie["c"] = False     # By default the path is not closed in synfig 
    lottie["k"] = {}
    gen_properties_shape_prop(lottie["k"], bline_point)
