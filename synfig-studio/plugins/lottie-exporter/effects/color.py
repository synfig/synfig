"""
This module will store all the functions required for color property of lottie
"""

import sys
import settings
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
    lottie["ty"] = settings.EFFECTS_COLOR   # Effect type
    lottie["nm"] = "Color"                  # Name
    lottie["ix"] = idx                      # Index
    lottie["v"] = {}                        # Value of color

    color = layer.get_param("color")
    color.animate("color")
    color.fill_path(lottie, "v")
