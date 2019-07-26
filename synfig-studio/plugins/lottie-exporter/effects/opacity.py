"""
This module will store all the functions required for opacity property of lottie
"""

import sys
import copy
import settings
from common.Count import Count
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

    opacity = layer.get_param("amount")
    opacity.animate("effects_opacity")
    opacity.fill_path(lottie, "v")
