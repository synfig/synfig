"""
Implements all the functions required for generating value for properties
related to layers in Synfig
"""

import sys
import settings
from misc import Count
from properties.shapeKeyframed import gen_properties_shapeKeyframed
from properties.value import gen_properties_value
sys.path.append("../")


def gen_mask(lottie, invert, bline_point, idx):
    """
    """
    lottie["inv"] = invert
    lottie["mode"] = settings.MASK_ADDITIVE
    lottie["pt"] = {}
    lottie["o"] = {}    # Opacity
    lottie["x"] = {}    # Expression
    lottie["nm"] = "Mask " + str(idx.idx) 
    index = Count()
    for chld in bline_point.getparent().getparent():
        if chld.tag == "param" and chld.attrib["name"] == "origin":
            origin = chld
    gen_properties_shapeKeyframed(lottie["pt"], bline_point, origin, index.inc())
    gen_properties_value(lottie["o"], settings.DEFAULT_OPACITY, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)
    gen_properties_value(lottie["x"], 0, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)

    convert_non_loop(lottie["pt"])


def convert_non_loop(lottie):
    print("reached")
    for chld in lottie["k"]:
        try:
            char_dict = {"s", "e"}
            for char in char_dict:
                posi = chld[char][0]
                print(posi["c"])
                if posi["c"] == False:
                    posi["o"][-1][0] = posi["o"][-1][1] = 0
                    posi["i"][0][0] = posi["i"][0][1] = 0
                    posi["c"] = True
                print(posi["c"])
        except:
            pass
