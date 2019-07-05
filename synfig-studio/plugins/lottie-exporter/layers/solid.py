"""
Will store all the functions corresponding to solids in lottie
"""

import sys
import settings
from helpers.transform import gen_helpers_transform
from misc import Count, get_color_hex
from helpers.blendMode import get_blend
from effects.fill import gen_effects_fill
from synfig.group import update_layer, get_additional_width, get_additional_height
sys.path.append("..")


def gen_layer_solid(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/solid.json

    Args:
        lottie (dict)               : Lottie generated solid layer stored here
        layer  (lxml.etree._Element): Synfig format solid layer
        idx    (int)                : Stores the index(number of) of solid layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_SOLID_TYPE
    lottie["nm"] = settings.LAYER_SOLID_NAME + str(idx)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled
    lottie["ef"] = []   # Stores the effects

    pos = [settings.lottie_format["w"]/2 + get_additional_width()/2,
        settings.lottie_format["h"]/2 + get_additional_height()/2]

    anchor = pos
    gen_helpers_transform(lottie["ks"], layer, pos, anchor)

    lottie["ef"].append({})
    gen_effects_fill(lottie["ef"][-1], layer, index.inc())

    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["sw"] = settings.lottie_format["w"] + get_additional_width() # Solid Width
    lottie["sh"] = settings.lottie_format["h"] + get_additional_height() # Solid Height

    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "color":
                lottie["sc"] = get_color_hex(chld[0])   # Solid Color

    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
