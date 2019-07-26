# pylint: disable=line-too-long
"""
This module will store all the functions required for transformations
corresponding to lottie
"""

import sys
import math
import copy
import settings
from common.Count import Count
from common.misc import is_animated, change_axis
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from synfig.animation import print_animation
sys.path.append("../")


def gen_helpers_transform(lottie, pos=settings.DEFAULT_POSITION, anchor=settings.DEFAULT_ANCHOR, scale=settings.DEFAULT_SCALE, rotation=settings.DEFAULT_ROTATION, opacity=settings.DEFAULT_OPACITY, skew=settings.DEFAULT_SKEW):
    """
    Generates the dictionary corresponding to helpers/transform.json

    Args:
        lottie (dict)                : Lottie format layer
        pos    (:obj: `list | lxml.etree._Element`, optional) : position of layer
        anchor (:obj: `list | lxml.etree._Element`, optional) : anchor point of layer
        scale (:obj: `list | lxml.etree._Element`, optional) : scale of layer
        rotation (:obj: `float | lxml.etree._Element`, optional) : rotation of layer
        opacity (:obj: `float | lxml.etree._Element`, optional) : Opacity of layer

    Returns:
        (None)
    """
    index = Count()
    lottie["o"] = {}    # opacity/Amount
    lottie["r"] = {}    # Rotation of the layer
    lottie["p"] = {}    # Position of the layer
    lottie["a"] = {}    # Anchor point of the layer
    lottie["s"] = {}    # Scale of the layer
    lottie["sk"] = {}   # skew of the layer
    lottie["sa"] = {}   # skew axis of the layer

    # setting the position
    if isinstance(pos, list):
        gen_properties_value(lottie["p"],
                             pos,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    else:
        pos.fill_path(lottie, "p")

    # setting the opacity
    if isinstance(opacity, (float, int)):
        gen_properties_value(lottie["o"],
                             opacity,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    else:
        opacity.fill_path(lottie, "o")

    # setting the rotation
    if isinstance(rotation, (float, int)):
        gen_properties_value(lottie["r"],
                             rotation,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    else:
        rotation.fill_path(lottie, "r")

    # setting the anchor point
    if isinstance(anchor, list):
        gen_properties_value(lottie["a"],
                             anchor,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    else:
        anchor.fill_path(lottie, "a")

    # setting the scale
    if isinstance(scale, list):
        gen_properties_value(lottie["s"],
                             scale,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    # This means scale parameter is animated
    else:
        scale.fill_path(lottie, "s")

    # setting the skew angle
    if isinstance(skew, (float, int)):
        gen_properties_value(lottie["sk"],
                             skew,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
    else:
        skew.fill_path(lottie, "sk")

    # setting the skew axis
    gen_properties_value(lottie["sa"],
                         0,
                         index.inc(),
                         settings.DEFAULT_ANIMATED,
                         settings.NO_INFO)
