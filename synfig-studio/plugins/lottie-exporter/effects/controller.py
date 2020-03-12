"""
This module will store the expression controllers of lottie/AE
"""

import sys
import settings
from common.Count import Count
from effects.slider import gen_effects_slider
from effects.point import gen_effects_point
sys.path.append("../")


def gen_effects_controller(lottie, value, anim_type):
    """
    Generates the dictionary correspondingt to effects/controller.json
    """
    lottie["ty"] = settings.EFFECTS_CONTROLLER  # Effect type
    idx = settings.controller_count.inc()
    lottie["nm"] = "Controller" + str(idx)
    lottie["ix"] = idx

    lottie["ef"] = []
    lottie["ef"].append({})
    if anim_type in {"vector", "group_layer_scale", "stretch_layer_scale", "circle_radius"}:
        gen_effects_point(lottie["ef"][-1], value, idx)
    else:
        gen_effects_slider(lottie["ef"][-1], value, idx)
