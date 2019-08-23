"""
This module will store all the functions required for feather property of lottie
"""

import sys
import settings
from common.Count import Count
from properties.value import gen_properties_value
sys.path.append("../")


def gen_effects_hfeather(lottie, idx):
    """
    Generates the dictionary corresponding to effects/horizontal feather

    Args:
        lottie (dict)                : Lottie format effects stored in this
        idx    (int)                 : Index/Count of effect

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_HFEATHER        # Effect type
    lottie["nm"] = "Horizontal Feather"             # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)


def gen_effects_vfeather(lottie, idx):
    """
    Generates the dictionary corresponding to effects/vertical feather

    Args:
        lottie (dict)                : Lottie format effects stored in this
        idx    (int)                 : Index/Count of effect

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_VFEATHER        # Effect type
    lottie["nm"] = "Vertical Feather"             # Name
    lottie["ix"] = idx                              # Index
    lottie["v"] = {}                                # value
    gen_properties_value(lottie["v"], 0, index.inc(), 0, settings.NO_INFO)
