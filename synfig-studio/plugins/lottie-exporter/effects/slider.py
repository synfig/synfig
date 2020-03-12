"""
This module will store the expression controllers of lottie/AE
"""

import sys
import settings
sys.path.append("../")


def gen_effects_slider(lottie, value, idx):
    """
    Generates the dictionary corresponding to effects/slider.json
    """
    lottie["ty"] = settings.EFFECTS_SLIDER      # Effect type
    lottie["nm"] = "Slider" + str(idx)
    lottie["ix"] = idx
    lottie["v"] = value
