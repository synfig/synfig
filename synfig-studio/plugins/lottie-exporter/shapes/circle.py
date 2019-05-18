"""
Will store all functions needed to generate the circle layer in lottie
"""
import sys
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")

def gen_shapes_circle(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/ellipse.json where ellipse
    will always be considered as circle
    """
    index = Count()
    lottie["ty"] = "el"     # Type: star
    lottie["p"] = {}        # Position of star
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of circle
    lottie["ix"] = idx      # setting the index

    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "origin":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                                               child[0],
                                                               index.inc())
                else:
                    x_val, y_val = 0, 0
                    if is_animate == 0:
                        x_val = float(child[0][0].text) * settings.PIX_PER_UNIT
                        y_val = float(child[0][1].text) * settings.PIX_PER_UNIT
                    else:
                        x_val = float(child[0][0][0][0].text) * settings.PIX_PER_UNIT
                        y_val = float(child[0][0][0][1].text) * settings.PIX_PER_UNIT
                    gen_properties_value(lottie["p"],
                                         change_axis(x_val, y_val),
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)

            # This will be exported as size of ellipse in lottie format
            elif child.attrib["name"] == "radius":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    gen_value_Keyframed(lottie["s"], child[0], index.inc())
                else:
                    radius = 0             # default value for radius
                    if is_animate == 0:
                        radius = float(child[0].attrib["value"])
                    else:
                        radius = float(child[0][0][0].attrib["value"])

                    radius_pix = int(settings.PIX_PER_UNIT) * radius
                    gen_properties_value(lottie["s"],
                                         [radius_pix, radius_pix],
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
