"""
Store all functions corresponding to pre composition in lottie
"""

import sys
import settings
from misc import Count
from sources.precomp import add_precomp_asset
from layers.rotate_layer import gen_layer_rotate_layer
sys.path.append("..")


def gen_layer_precomp(lottie, layer, idx):
    """
    Here idx represents the position of layer in the .sif file also
    """
    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_PRECOMP_TYPE
    lottie["nm"] = settings.LAYER_PRECOMP_NAME + str(idx)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    settings.lottie_format["assets"].append({})
    asset = add_precomp_asset(settings.lottie_format["assets"][-1], layer.getparent(), idx)
    lottie["refId"] = asset

    if layer.attrib["type"] == "rotate":
        # transform properties will be written inside this now
        gen_layer_rotate_layer(lottie["ks"], layer)

    lottie["w"] = settings.lottie_format["w"]
    lottie["h"] = settings.lottie_format["h"]
    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    lottie["bm"] = settings.DEFAULT_BLEND    # Always have the default blend 
