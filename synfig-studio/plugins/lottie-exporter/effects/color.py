"""
This module will store all the functions required for color property of lottie
"""

import sys
import settings
from common.misc import is_animated
from common.Count import Count
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("../")


def gen_effects_color(lottie, layer, idx):
    """
    Generates the dictionary corresponding to effects/color.json

    Args:
        lottie (dict)               : Lottie format effects stored in this
        layer  (common.Layer.Layer)  : Synfig format layer
        idx    (int)                : Index/Count of effect

    Returns:
        (None)

    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_COLOR   # Effect type
    lottie["nm"] = "Color"                  # Name
    lottie["ix"] = idx                      # Index
    lottie["v"] = {}                        # Value of color

    color = layer.get_param("color")
    is_animate = is_animated(color[0])
    if is_animate == 2: 
        gen_value_Keyframed(lottie["v"], color[0], index.inc())

    else:
        if is_animate == 0:
            val = color[0]
        else:
            val = color[0][0][0]
        red = float(val[0].text)
        green = float(val[1].text)
        blue = float(val[2].text)
        red, green, blue = red ** (1/settings.GAMMA), green **\
        (1/settings.GAMMA), blue ** (1/ settings.GAMMA)
        alpha = float(val[3].text)
        gen_properties_value(lottie["v"],
                             [red, green, blue, alpha],
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
