"""
This module will store the expression controllers of lottie/AE
"""

import sys
import copy
import settings
from common.Count import Count
from effects.slider import gen_effects_slider
from effects.point import gen_effects_point
sys.path.append("../")


def gen_effects_controller(lottie, value, anim_type):
    """
    Generates the dictionary correspondingt to effects/controller.json
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_CONTROLLER  # Effect type
    idx = settings.controller_count.inc()
    lottie["nm"] = "Controller" + str(idx)
    lottie["ix"] = idx

    lottie["ef"] = []
    lottie["ef"].append({})
    if anim_type == "vector":
        gen_effects_point(lottie["ef"][-1], value, idx)
    else:
        gen_effects_slider(lottie["ef"][-1], value, idx)
