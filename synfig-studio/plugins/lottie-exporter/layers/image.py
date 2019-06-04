"""
Will store all the functions corresponding to Image Layer in lottie
"""

import sys
import settings
from helpers.transform import gen_helpers_transform
from misc import Count, change_axis, get_vector
from helpers.blendMode import get_blend
from sources.image import add_image_asset
sys.path.append("..")


def gen_layer_image(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/image.json
    """
    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_IMAGE_TYPE
    lottie["nm"] = settings.LAYER_IMAGE_NAME + str(idx)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    settings.lottie_format["assets"].append({})
    st = add_image_asset(settings.lottie_format["assets"][-1], layer)
    asset = settings.lottie_format["assets"][-1]

    # setting class (jpg, png)
    lottie["cl"] = asset["p"].split(".")[-1] 

    # setting the reference id
    lottie["refId"] = asset["id"]

    # finding the top left of the image
    pos1 = get_vector(st["tl"])
    pos2 = get_vector(st["br"])
    pos1 *= settings.PIX_PER_UNIT
    pos2 *= settings.PIX_PER_UNIT

    scale_x = abs(pos1.val1 - pos2.val1) / asset["w"]
    scale_y = abs(pos1.val2 - pos2.val2) / asset["h"]
    scale_x *= 100
    scale_y *= 100
    scale = [scale_x, scale_y]

    pos1 = change_axis(pos1.val1, pos1.val2)
    anchor = [0, 0, 0]

    gen_helpers_transform(lottie["ks"], layer, pos1, anchor, scale)


    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
    lottie["markers"] = []      # Markers to be filled yet
