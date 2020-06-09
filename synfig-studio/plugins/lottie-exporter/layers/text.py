"""
Will store all the functions corresponding to text layer in lottie
"""

import sys
import settings
from common.Count import Count
from synfig.group import get_additional_width,get_additional_height
from helpers.transform import gen_helpers_transform
from helpers.blendMode import get_blend
from common.misc import is_animated
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")

def calc_default_text_properties(layer):
    """
    Generated the lottie dictionary corresponding to properties which won't change in text-animation

    Args:
        layer  (common.Layer.Layer) : Synfig format text layer

    Returns:
        Lottie format default dictionary for text animation properties
    """
    default_dict = {
      "nm": "",
      "s": {
        "t": 0,
        "xe": {
          "a": 0,
          "k": 0,
          "ix": 7
        },
        "ne": {
          "a": 0,
          "k": 0,
          "ix": 8
        },
        "a": {
          "a": 0,
          "k": 100,
          "ix": 4
        },
        "b": 1,
        "rn": 0,
        "sh": 1,
        "r": 1
      }
    }
    default_dict["nm"] = layer.get_description()

    return default_dict
def calc_anchor_alignment(lottie):
    """
    Generates the dictionary corresponding to the anchor alignment properties
    
    Args:
        lottie (dict) : Lottie generated text layer stored here
    """
    lottie["t"]["m"] = {  
                          "g": 1,
                          "a": {
                            "a": 0,
                            "k": [
                              0,
                              0
                            ],
                            "ix": 2
                         }
                       } #Has no corresponding synfig property, so using default values

def gen_layer_text(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/text.json

    Args:
        lottie (dict)       : Lottie generated text layer stored here
        layer  (common.Layer.Layer) : Synfig format text layer
        idx    (int)        : Stores the index(number of) of text layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx+1
    lottie["ty"] = settings.LAYER_TEXT_TYPE
    lottie["nm"] = layer.get_description()
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    lottie["t"] = {}
    calc_anchor_alignment(lottie)
    lottie["t"]["a"] = []

    origin = layer.get_param("origin")
    origin.animate("vector")
    
    anchor = layer.get_param("orient")
    anchor.animate("vector",True)

    scale = layer.get_param("size")
    scale.animate("stretch_layer_scale")
    pos = origin
    pos.animate("vector",True)

    gen_helpers_transform(lottie["ks"], pos, anchor, scale)
    
    lottie["t"]["a"].append(calc_default_text_properties(layer))
    lottie["t"]["a"][0]["a"] = {"fc":{},"o":{}}
    
    color = layer.get_param("color")
    color.animate("color")
    color.fill_path(lottie["t"]["a"][0]["a"],"fc")

    opacity = layer.get_param("amount").get()
    is_animate = is_animated(opacity[0])
    if is_animate == settings.ANIMATED:
        # Telling the function that this is for opacity
        opacity[0].attrib['type'] = 'opacity'
        gen_value_Keyframed(lottie["t"]["a"][0]["a"]["o"], opacity[0], index.inc())

    else:
        if is_animate == settings.NOT_ANIMATED:
            val = float(opacity[0].attrib["value"]) * settings.OPACITY_CONSTANT
        else:
            val = float(opacity[0][0][0].attrib["value"]) * settings.OPACITY_CONSTANT
        gen_properties_value(lottie["t"]["a"][0]["a"]["o"],
                             val,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    lottie["t"]["d"] = {}
    lottie["t"]["p"] = {}
    lottie["t"]["d"]["k"] = []
    default = {
              "s": {
                "s": 60,
                "f": "",
                "t": "",
                "j": 2,
                "tr": 0,
                "lh": 50,
                "ls": 1,
                "fc": [1,1,1,1]
              },
              "t": 0
            }
    
    ax = layer.get_param("text").get()[0].text.split("\n")
    #Line separation is not supported so adding separation only for integer values
    line_separation = layer.get_param("vcompress").get()[0].attrib["value"]
    breaks = "\r"*round(float(line_separation))
    text = breaks.join(ax)
    default["s"]["t"] = text
    lottie["t"]["d"]["k"].append(default)
    get_blend(lottie, layer)