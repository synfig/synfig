"""
Module contains all functions required for setting up the blend mode of any
layer
"""

import sys
import settings
sys.path.append("..")


def get_blend(lottie, layer):
    """
    blend_map stores the mapping:
    composite  :  0,
    difference : 18,
    multiply   :  6,
    Hard light : 17,
    Luminance  : 11,
    Saturation : 10,
    Hue        :  9,
    Color      :  8,
    Darken     :  3,
    Brighten   :  2,
    Overlay    : 20,    Does not work perfectly :(
    Screen     : 16

    Args:
        lottie (dict)               : Lottie format layer
        layer  (common.Layer.Layer)  : Synfig format layer

    Returns:
        (None)
    """
    blend_map = {0 : 0, 18 : 10, 6 : 1, 17 : 8, 11 : 15, 10 : 13, 9 : 12, 8 : 14,
                 3 : 4, 2 : 5, 20 : 3, 16 : 2}

    blend = layer.get_param("blend_method").get()
    key = int(blend[0].attrib["value"])
    if key in blend_map.keys():
        lottie["bm"] = blend_map[key]
    else:
        lottie["bm"] = settings.DEFAULT_BLEND
