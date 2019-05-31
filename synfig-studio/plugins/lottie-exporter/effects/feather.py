"""
This module will store all the functions required for feather property of lottie
"""

import sys
import settings
from misc import Count
from properties.value import gen_properties_value
sys.path.append("../")


def gen_effects_hfeather(lottie, layer, idx):
    """
    Generates the dictionary corresponding to effects/horizontal feather
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_HFEATHER        # Effect type
    lottie["nm"] = "Horizontal Feather"             # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)


def gen_effects_vfeather(lottie, layer, idx):
    """
    Generates the dictionary corresponding to effects/vertical feather
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_VFEATHER        # Effect type
    lottie["nm"] = "Vertical Feather"             # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)
