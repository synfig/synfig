"""
Fill this
"""
import sys
sys.path.append("..")
import settings
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
from misc import Count

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
                if child[0].tag == "animated":
                    gen_value_Keyframed(lottie["c"], child[0], index.inc())
                else:
                    red = float(child[0][0].text)
                    green = float(child[0][1].text)
                    blue = float(child[0][2].text)
                    red, green, blue = red ** (1/settings.GAMMA), green **\
                    (1/settings.GAMMA), blue ** (1/ settings.GAMMA)
                    alpha = float(child[0][3].text)
                    gen_properties_value(
                        lottie["c"], [
                            red, green, blue, alpha], index.inc(),
                        settings.DEFAULT_ANIMATED, settings.NO_INFO)
            elif child.attrib["name"] == "amount":
                if child[0].tag == "animated":
                    # Telling the function that this is for opacity
                    child[0].attrib['type'] = 'opacity'
                    gen_value_Keyframed(lottie["o"], child[0], index.inc())
                else:
                    val = float(child[0].attrib["value"]) * settings.OPACITY_CONSTANT
                    gen_properties_value(lottie["o"], val, index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)
                 
