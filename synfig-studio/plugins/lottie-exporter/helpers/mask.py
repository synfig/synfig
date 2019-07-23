# pylint: disable=line-too-long
"""
Implements all the functions required for generating a lottie mask
"""

import sys
import settings
from common.Count import Count
from properties.shapeKeyframed import gen_properties_shapeKeyframed
from properties.value import gen_properties_value
sys.path.append("../")


def gen_mask(lottie, invert, bline_point, idx):
    """
    Generates a mask. This was specifically needed to support the invert
    parameter

    Args:
        lottie      (dict) : The final mask will be stored in this dict
        invert      (bool) : Tells if the invert parameter is set or not
        bline_point (common.Layer.Layer | common.Param.Param) : Synfig format shape
        idx         (int)  : Specifies the index of this mask

    Returns:
        (None)
    """
    lottie["inv"] = invert
    lottie["mode"] = settings.MASK_ADDITIVE
    lottie["pt"] = {}
    lottie["o"] = {}    # Opacity
    lottie["x"] = {}    # Expression
    lottie["nm"] = "Mask " + str(idx)
    index = Count()

    gen_properties_shapeKeyframed(lottie["pt"], bline_point, index.inc())
    gen_properties_value(lottie["o"], settings.DEFAULT_OPACITY, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)
    gen_properties_value(lottie["x"], 0, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)

    convert_non_loop(lottie["pt"])


def convert_non_loop(lottie):
    """
    Converts the non loop shapes to looped shapes by changing the in and out
    tangents accordingly

    Args:
        lottie (dict) : The shape to be modified

    Returns:
        (None)
    """
    for chld in lottie["k"]:
        if "s" in chld.keys() and "e" in chld.keys():
            char_dict = {"s", "e"}
            for char in char_dict:
                posi = chld[char][0]
                if not posi["c"]:
                    posi["o"][-1][0] = posi["o"][-1][1] = 0
                    posi["i"][0][0] = posi["i"][0][1] = 0
                    posi["c"] = True
