"""
Fill this
"""
import sys
sys.path.append("../")
import settings
from misc import Count
from properties.value import gen_properties_value

def gen_helpers_transform(lottie, layer):
    """
    Generates the dictionary corresponding to helpers/transform.json
    """
    index = Count()
    lottie["o"] = {}    # opacity/Amount
    lottie["r"] = {}    # Rotation of the layer
    lottie["p"] = {}    # Position of the layer
    lottie["a"] = {}    # Anchor point of the layer
    lottie["s"] = {}    # Scale of the layer
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "amount":
                val = int(settings.OPACITY_CONSTANT * float(child[0].attrib["value"]))
                gen_properties_value(
                    lottie["o"], val, index.inc(), settings.DEFAULT_ANIMATED,
                    settings.NO_INFO)
            elif child.attrib["name"] == "origin":
                if child[0].tag == "vector":
                    x_val = float(child[0][0].text) * settings.PIX_PER_UNIT
                    y_val = float(child[0][1].text) * settings.PIX_PER_UNIT
                    gen_properties_value(lottie["p"], change_axis(x_val, y_val),
                                         index.inc(), settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
                else:
                    gen_properties_value(lottie["p"], [0, 0],
                                         index.inc(), settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)

    gen_properties_value(
        lottie["r"],
        settings.DEFAULT_ROTATION,
        index.inc(),
        settings.DEFAULT_ANIMATED,
        settings.NO_INFO)
    gen_properties_value(
        lottie["a"], [
            0, 0, 0], index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)
    gen_properties_value(
        lottie["s"], [
            100, 100, 100], index.inc(), settings.DEFAULT_ANIMATED, settings.NO_INFO)

