"""
This module will store all the functions required for fill effects of lottie
"""

import sys
import settings
from common.Count import Count
from common.Layer import Layer
from effects.fillmask import gen_effects_fillmask
from effects.allmask import gen_effects_allmask
from effects.color import gen_effects_color
from effects.invert import gen_effects_invert
from effects.feather import gen_effects_hfeather, gen_effects_vfeather
from effects.opacity import gen_effects_opacity
sys.path.append("../")


def gen_effects_fill(lottie, layer, idx):
    """
    Generates the dictionary corresponding to effects/fill.json

    Args:
        lottie (dict)               : Lottie format layer
        layer  (common.Layer.Layer)  : Synfig format layer
        idx    (int)                : Index/Count of effect

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = settings.EFFECTS_FILL    # Effect type
    lottie["nm"] = "Fill"                   # Name
    lottie["ix"] = idx                      # Index
    lottie["ef"] = []                       # Effect list of properties

    # generating the fill mask, has no use in Synfig. But a necessity for
    # running the .json file
    lottie["ef"].append({})
    gen_effects_fillmask(lottie["ef"][-1], index.inc())

    # generating the all mask property as required by lottie
    lottie["ef"].append({})
    gen_effects_allmask(lottie["ef"][-1], index.inc())

    # generating the color property
    lottie["ef"].append({})
    gen_effects_color(lottie["ef"][-1], layer, index.inc())

    # generating the invert property as required by lottie
    lottie["ef"].append({})
    gen_effects_invert(lottie["ef"][-1], index.inc())

    # generating the horizontal feather as required by lottie
    lottie["ef"].append({})
    gen_effects_hfeather(lottie["ef"][-1], index.inc())

    # generating the vertical feather as required by lottie
    lottie["ef"].append({})
    gen_effects_vfeather(lottie["ef"][-1], index.inc())

    # generating the opacity
    lottie["ef"].append({})
    gen_effects_opacity(lottie["ef"][-1], layer, index.inc())
