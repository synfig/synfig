"""
Will store all the functions corresponding to Image Layer in lottie
"""
import sys
import settings
from helpers.transform import gen_helpers_transform
from misc import Count, get_color_hex
from helpers.blendMode import get_blend
from effects.fill import gen_effects_fill
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

    # finding the center of image
    x_val = (float(st["br"][0][0].text) + float(st["tl"][0][0].text)) / 2
    y_val = (float(st["br"][0][1].text) + float(st["tl"][0][1].text)) / 2
    x_val *= settings.PIX_PER_UNIT
    y_val *= settings.PIX_PER_UNIT
    pos = [x_val, y_val]
    anchor = pos
    gen_helpers_transform(lottie["ks"], layer, pos, anchor)


    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
    lottie["markers"] = []      # Markers to be filled yet
