"""
Will store all the functions needed to export the translate layer
"""

import sys
from lxml import etree
import copy
import settings
from common.Param import Param
from helpers.transform import gen_helpers_transform
from synfig.animation import print_animation
sys.path.append("..")


def gen_layer_translate(lottie, layer):
    """
    Help generate transform properties of translate layer

    Args:
        lottie (dict) : Transform properties in lottie format
        layer  (common.Layer.Layer) : Transform properties in Synfig format

    Returns:
        (None)
    """

    origin = layer.get_param("origin")
    origin.animate("vector")
    pos = origin

    st = "<param name='anchor'><vector><x>0.00</x><y>0.00</y></vector></param>"
    anchor = etree.fromstring(st)
    anchor = Param(anchor, layer)
    anchor.animate("vector")
    anchor.add_offset()

    if settings.INSIDE_PRECOMP:
        pos.add_offset()
    anchor.animate("vector", True)
    pos.animate("vector", True)

    gen_helpers_transform(lottie, pos, anchor)
