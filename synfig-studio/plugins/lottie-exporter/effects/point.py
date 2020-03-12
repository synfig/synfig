"""
Store all functions necessary for effect/point.json
"""

import sys
import settings
sys.path.append("../")


def gen_effects_point(lottie, value, idx):
    """
    Generates the dictionary corresponding to effects/point.json
    """
    lottie["ty"] = settings.EFFECTS_POINT      # Effect type
    lottie["nm"] = "Point" + str(idx)
    lottie["ix"] = idx
    lottie["v"] = value
