"""
This module will store all the functions required for transformations
corresponding to lottie
"""

import sys
import settings
from misc import Count
from properties.value import gen_properties_value
sys.path.append("../")


def gen_helpers_transform(lottie, layer, pos = [0, 0], anchor = [0, 0, 0]):
    """
    Generates the dictionary corresponding to helpers/transform.json
    """
    index = Count()
    lottie["o"] = {}    # opacity/Amount
    lottie["r"] = {}    # Rotation of the layer
    lottie["p"] = {}    # Position of the layer
    lottie["a"] = {}    # Anchor point of the layer
    lottie["s"] = {}    # Scale of the layer

    # setting the default location
    gen_properties_value(lottie["p"],
                         pos,
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

    # setting the default opacity i.e. 100
    gen_properties_value(lottie["o"],
                         settings.DEFAULT_OPACITY,
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

    # setting the rotation
    gen_properties_value(lottie["r"],
                         settings.DEFAULT_ROTATION,
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

    # setting the anchor point
    gen_properties_value(lottie["a"],
                         anchor,
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)

    # setting the scale
    gen_properties_value(lottie["s"],
                         [100, 100, 100],
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)
