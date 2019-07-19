# pylint: disable=line-too-long
"""
Store all functions corresponding to pre composition in lottie
"""

import sys
import settings
from common.misc import set_layer_desc
from common.Layer import Layer
from sources.precomp import add_precomp_asset
from layers.rotate_layer import gen_layer_rotate
from layers.scale_layer import gen_layer_scale
from layers.translate_layer import gen_layer_translate
sys.path.append("..")


def gen_layer_precomp(lottie, cl_layer, idx):
    """
    Generates a pre-composition layer depending upon the layers inside that
    pre-comp
    Here idx represents the position of layer in the .sif file also

    Args:
        lottie (dict) : Will store the pre-comp layer
        layer  (misc.Layer) : Specifies which layer it is
        idx    (int)  : Index of the layer

    Returns:
        (None)
    """
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_PRECOMP_TYPE
    set_layer_desc(cl_layer.get_layer(), settings.LAYER_PRECOMP_NAME + str(idx), lottie)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    if cl_layer.get_type() == "rotate":
        # transform properties will be written inside this now
        gen_layer_rotate(lottie["ks"], cl_layer)
        settings.INSIDE_PRECOMP = True
    elif cl_layer.get_type() == "zoom":
        gen_layer_scale(lottie["ks"], cl_layer)
        settings.INSIDE_PRECOMP = True
    elif cl_layer.get_type() == "translate":
        gen_layer_translate(lottie["ks"], cl_layer)
        settings.INSIDE_PRECOMP = True

    settings.lottie_format["assets"].append({})
    asset = add_precomp_asset(settings.lottie_format["assets"][-1], cl_layer.get_layer().getparent(), idx)
    lottie["refId"] = asset


    lottie["w"] = settings.lottie_format["w"] + settings.ADDITIONAL_PRECOMP_WIDTH # Experimental increase in width and height of precomposition
    lottie["h"] = settings.lottie_format["h"] + settings.ADDITIONAL_PRECOMP_HEIGHT
    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    lottie["bm"] = settings.DEFAULT_BLEND    # Always have the default blend
