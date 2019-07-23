"""
Will store all functions needed to generate the circle layer in lottie
This will also support the simple_circle layer of Synfig
"""

import sys
import settings
from properties.value import gen_properties_value
from common.misc import is_animated, change_axis
from common.Count import Count
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")


def gen_shapes_circle(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/ellipse.json where ellipse
    will always be considered as circle

    Args:
        lottie (dict)               : The lottie generated circle layer will be stored in it
        layer  (common.Layer.Layer)  : Synfig format circle layer
        idx    (int)                : Stores the index of the circle layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = "el"     # Type: circle
    lottie["p"] = {}        # Position of circle
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of circle
    lottie["ix"] = idx      # setting the index

    # Origin
    origin = layer.get_param("origin", "center").get()
    is_animate = is_animated(origin[0])
    if is_animate == 2:
        gen_properties_multi_dimensional_keyframed(lottie["p"],
                                                   origin[0],
                                                   index.inc())
    else:
        x_val, y_val = 0, 0
        if is_animate == 0:
            x_val = float(origin[0][0].text) * settings.PIX_PER_UNIT
            y_val = float(origin[0][1].text) * settings.PIX_PER_UNIT
        else:
            x_val = float(origin[0][0][0][0].text) * settings.PIX_PER_UNIT
            y_val = float(origin[0][0][0][1].text) * settings.PIX_PER_UNIT
        gen_properties_value(lottie["p"],
                             change_axis(x_val, y_val),
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    
    # Radius
    radius = layer.get_param("radius").get()
    # This will be exported as size of ellipse in lottie format
    is_animate = is_animated(radius[0])
    if is_animate == 2:
        radius[0].attrib['type'] = "circle_radius"
        gen_value_Keyframed(lottie["s"], radius[0], index.inc())
    else:
        rad = 0             # default value for radius
        if is_animate == 0:
            rad = float(radius[0].attrib["value"])
        else:
            rad = float(radius[0][0][0].attrib["value"])

        radius_pix = int(settings.PIX_PER_UNIT) * rad
        diam = radius_pix * 2
        gen_properties_value(lottie["s"],
                             [diam, diam],
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
