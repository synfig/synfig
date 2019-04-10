"""
Fill this
"""
import sys
sys.path.append("..")
import settings
from properties.value import gen_properties_value
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
                red = float(child[0][0].text)
                green = float(child[0][1].text)
                blue = float(child[0][2].text)
                red, green, blue = red ** (1/settings.GAMMA), green **\
                (1/settings.GAMMA), blue ** (1/ settings.GAMMA)
                a_val = child[0][3].text
                gen_properties_value(
                    lottie["c"], [
                        red, green, blue, a_val], index.inc(),
                    settings.DEFAULT_ANIMATED, settings.NO_INFO)

    gen_properties_value(
        lottie["o"],
        settings.DEFAULT_OPACITY,
        index.inc(),
        settings.DEFAULT_ANIMATED,
        settings.NO_INFO)

