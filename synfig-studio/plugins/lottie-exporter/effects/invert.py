"""
This module will store all the functions required for invert property of lottie
"""

import sys
import settings
from common.Count import Count
from properties.value import gen_properties_value
sys.path.append("../")


def gen_effects_invert(lottie, idx):
    """
    Generates the dictionary corresponding to effects/invert.json

    Args:
        lottie (dict)                : Lottie format effects stored in this
        idx    (int)                 : Index/Count of effect

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_INVERT          # Effect type
    lottie["nm"] = "Invert"                         # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)
