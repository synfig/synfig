"""
Fill this
"""
import sys
sys.path.append("..")
import settings
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
from misc import Count, is_animated

def gen_shapes_fill(lottie, layer):
    """
    Generates the dictionary corresponding to shapes/fill.json
    """
    index = Count()
    lottie["ty"] = "fl"     # Type if fill
    lottie["c"] = {}       # Color
    lottie["o"] = {}       # Opacity of the fill layer
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "color":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_value_Keyframed(lottie["c"], child[0], index.inc())

                else:
                    if is_animate == 0:
                        val = child[0]
                    else:
                        val = child[0][0][0]
                    red = float(val[0].text)
                    green = float(val[1].text)
                    blue = float(val[2].text)
                    red, green, blue = red ** (1/settings.GAMMA), green **\
                    (1/settings.GAMMA), blue ** (1/ settings.GAMMA)
                    alpha = float(val[3].text)
                    gen_properties_value(
                        lottie["c"], [
                            red, green, blue, alpha], index.inc(),
                        settings.DEFAULT_ANIMATED, settings.NO_INFO)
            elif child.attrib["name"] == "amount":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    # Telling the function that this is for opacity
                    child[0].attrib['type'] = 'opacity'
                    gen_value_Keyframed(lottie["o"], child[0], index.inc())

                else:
                    if is_animate == 0:
                        val = float(child[0].attrib["value"]) * settings.OPACITY_CONSTANT
                    else:
                        val = float(child[0][0][0].attrib["value"]) * settings.OPACITY_CONSTANT
                    gen_properties_value(lottie["o"], val, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)
                 
