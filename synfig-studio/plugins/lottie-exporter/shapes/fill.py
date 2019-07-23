"""
Will have all the functions required for generate the fill(color) in lottie
"""

import sys
import settings
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
from common.misc import is_animated
from common.Count import Count
from common.Layer import Layer
sys.path.append("..")


def gen_shapes_fill(lottie, layer):
    """
    Generates the dictionary corresponding to shapes/fill.json

    Args:
        lottie (dict)               : The lottie generated fill layer will be stored in it
        layer  (common.Layer.Layer)  : Synfig format fill (can be shape/solid anything, we
                                    only need color and opacity part from it) layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = "fl"     # Type if fill
    lottie["c"] = {}       # Color
    lottie["o"] = {}       # Opacity of the fill layer
    
    # Color
    color = layer.get_param("color").get()
    is_animate = is_animated(color[0])
    if is_animate == 2:
        gen_value_Keyframed(lottie["c"], color[0], index.inc())

    else:
        if is_animate == 0:
            val = color[0]
        else:
            val = color[0][0][0]
        red = float(val[0].text)
        green = float(val[1].text)
        blue = float(val[2].text)
        red, green, blue = red ** (1/settings.GAMMA), green ** (1/settings.GAMMA), blue ** (1/ settings.GAMMA)
        alpha = float(val[3].text)
        gen_properties_value(lottie["c"],
                             [red, green, blue, alpha],
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    # Color Opacity
    opacity = layer.get_param("amount").get()
    is_animate = is_animated(opacity[0])
    if is_animate == 2:
        # Telling the function that this is for opacity
        opacity[0].attrib['type'] = 'opacity'
        gen_value_Keyframed(lottie["o"], opacity[0], index.inc())

    else:
        if is_animate == 0:
            val = float(opacity[0].attrib["value"]) * settings.OPACITY_CONSTANT
        else:
            val = float(opacity[0][0][0].attrib["value"]) * settings.OPACITY_CONSTANT
        gen_properties_value(lottie["o"],
                             val,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
