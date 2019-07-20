"""
This module will store all the functions required for fill mask property of lottie
"""

import sys
import settings
from common.Count import Count
from properties.value import gen_properties_value
sys.path.append("../")


def gen_effects_fillmask(lottie, idx):
    """
    Generates the dictionary corresponding to effects/fillmask.json

    Args:
        lottie (dict)                : Lottie format effects stored in this
        idx    (int)                 : Index/Count of effect

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_FILL_MASK       # Effect type
    lottie["nm"] = "Fill Mask"                      # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)
