"""
This module will store all the functions required for all mask property of lottie
"""

import sys
import settings
from misc import Count
from properties.value import gen_properties_value
sys.path.append("../")


def gen_effects_allmask(lottie, layer, idx):
    """
    Generates the dictionary corresponding to effects/allmask.json
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_ALL_MASK        # Effect type
    lottie["nm"] = "All Masks"                      # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)
