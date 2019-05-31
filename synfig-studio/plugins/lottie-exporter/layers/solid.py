"""
Will store all the functions corresponding to solids in lottie
"""

import sys
import settings
from helpers.transform import gen_helpers_transform
from misc import Count, get_color_hex
from helpers.blendMode import get_blend
from effects.fill import gen_effects_fill
sys.path.append("..")


def gen_layer_solid(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/solid.json
    """
    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_SOLID_TYPE
    lottie["nm"] = settings.LAYER_SOLID_NAME + str(idx)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled
    lottie["ef"] = []   # Stores the effects

    pos = [settings.lottie_format["w"]/2, settings.lottie_format["h"]/2]
    anchor = pos
    gen_helpers_transform(lottie["ks"], layer, pos, anchor)

    lottie["ef"].append({})
    gen_effects_fill(lottie["ef"][-1], layer, index.inc())

    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["sw"] = settings.lottie_format["w"]  # Solid Width
    lottie["sh"] = settings.lottie_format["h"]  # Solid Height
    
    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "color":
                lottie["sc"] = get_color_hex(chld[0])   # Solid Color

    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
    lottie["markers"] = []      # Markers to be filled yet
