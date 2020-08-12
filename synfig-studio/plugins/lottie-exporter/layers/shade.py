# pylint: disable=line-too-long
"""
Will store all the functions corresponding to shade Layer in lottie
"""

import settings
import copy
from lxml import etree
from properties.valueKeyframed import gen_value_Keyframed
from common.misc import is_animated
from properties.value import gen_properties_value
from common.Count import Count
from common.Param import Param

def gen_layer_shade(lottie, layer):
    """
    This function will be called for each canvas/composition. Main function to
    generate all the layers

    Args:
        lottie (dict) : Lottie Dictionary for shade layers
        layer  (List) : Dictionary of Synfig format layers

    Returns:
        (None)
    """
    index = Count()
    sub_index = Count()
    lottie["ty"] = 25
    lottie["nm"] = "Drop Shadow"
    lottie["np"] = 8
    lottie["mn"] = "ADBE Drop Shadow"
    lottie["ix"] = 11
    lottie["en"] = 1

    lottie["ef"] = []
    opacity = layer.get_param("amount")
    color = layer.get_param("color")
    origin = layer.get_param("origin")

    color_dict = {}
    opacity_dict = {}
    origin_dict = {}
    direction_dict = {}
    softness_dict = {}
    shadow_dict = {}

    color_dict["ty"] =  2
    color_dict["nm"] =  "Shadow Color"
    color_dict["mn"] =  "ADBE Drop Shadow-0001"
    color_dict["ix"] =  index.inc()
    color_dict["v"]  = {}
    color.animate("color")
    color.fill_path(color_dict,"v")

    opacity_dict["ty"] = 0
    opacity_dict["nm"] = "Opacity"
    opacity_dict["mn"] = "ADBE Drop Shadow-0002"
    opacity_dict["ix"] = index.inc()
    opacity_dict["v"]  = {}
    is_animate = is_animated(opacity[0])
    # Telling the function that this is for opacity
    if is_animate == settings.ANIMATED:
        opacity[0].attrib['type'] = 'opacity'
        gen_value_Keyframed(opacity_dict["v"], opacity[0], sub_index.inc())

    else:
        if is_animate == settings.NOT_ANIMATED:
            val = float(opacity[0].attrib["value"]) * settings.OPACITY_CONSTANT
        else:
            val = float(opacity[0][0][0].attrib["value"]) * settings.OPACITY_CONSTANT
        gen_properties_value(opacity_dict["v"],
                             val,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    direction_dict["ty"] = 0
    direction_dict["nm"] = "Direction"
    direction_dict["mn"] = "ADBE Drop Shadow-0003"
    direction_dict["ix"] = index.inc()
    direction_dict["v"]  = {}

    direction = "<param name='direction'><direction type='vector'></direction></param>"
    val = etree.fromstring(direction)
    direction_value = copy.deepcopy(origin[0])
    val[0].append(direction_value)
    direction_param = Param(val,layer)
    direction_param.animate("angle")
    direction_param.fill_path(direction_dict,"v")

    origin_dict["ty"] =  0
    origin_dict["nm"] =  "Distance"
    origin_dict["mn"] =  "ADBE Drop Shadow-0003"
    origin_dict["ix"] =  index.inc()
    origin_dict["v"] = {}
    
    distance_str = "<param name='origin'><distance type='vector'></distance></param>"
    val = etree.fromstring(distance_str)
    distance_value = copy.deepcopy(origin[0])
    val[0].append(distance_value)
    distance_param = Param(val,layer)
    distance_param.animate("real")
    distance_param.fill_path(origin_dict,"v")

    softness_dict["ty"] = 0,
    softness_dict["nm"] = "Softness",
    softness_dict["mn"] = "ADBE Drop Shadow-0005",
    softness_dict["ix"] = index.inc()
    softness_dict["v"]  = {}
    gen_properties_value(softness_dict["v"],
                         0,
                         sub_index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

    shadow_dict["ty"] =  7,
    shadow_dict["nm"] =  "Shadow Only",
    shadow_dict["mn"] =  "ADBE Drop Shadow-0006",
    shadow_dict["ix"] =  index.inc()
    shadow_dict["v"]  = {}
    gen_properties_value(shadow_dict["v"],
                             0,
                             sub_index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

    lottie["ef"].extend([color_dict,opacity_dict,direction_dict,origin_dict,softness_dict,shadow_dict])
