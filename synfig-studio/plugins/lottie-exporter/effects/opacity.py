"""
This module will store all the functions required for opacity property of lottie
"""

import sys
import settings
from common.misc import is_animated
from common.Count import Count
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("../")


def gen_effects_opacity(lottie, layer, idx):
    """
    Generates the dictionary corresponding to effects/opacity.json

    Args:
        lottie (dict)               : Lottie format effects stored in this
        layer  (common.Layer.Layer)  : Synfig format layer
        idx    (int)                : Index/Count of effect

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_OPACITY     # Effect type
    lottie["nm"] = "Opacity"                    # Name
    lottie["ix"] = idx                          # Index
    lottie["v"] = {}                            # Value of opacity

    opacity = layer.get_param("amount").get()
    is_animate = is_animated(opacity[0])
    if is_animate == 2:
        # Telling the function that this is for opacity
        opacity[0].attrib['type'] = 'effects_opacity'
        gen_value_Keyframed(lottie["v"], opacity[0], index.inc())

    else:
        if is_animate == 0:
            val = float(opacity[0].attrib["value"])
        else:
            val = float(opacity[0][0][0].attrib["value"])
        gen_properties_value(lottie["v"],
                             val,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
