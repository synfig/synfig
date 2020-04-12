"""
Will store all functions needed to generate the shape(path) layer in lottie
# This is not currently used anywhere because we are using solid_shape.py instead of this module to
support the invert parameter
"""

import sys
import settings
from common.Count import Count
from properties.shapeKeyframed import gen_properties_shapeKeyframed
sys.path.append("..")


def gen_shapes_shape(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/shape.json

    Args:
        lottie (dict)               : The lottie generated shape layer will be stored in it
        layer  (lxml.etree._Element): Synfig format shape(list of bline points) layer
        idx    (int)                : Stores the index of the shape layer

    Returns:
        (None)
    """
    index = Count()
    lottie["ty"] = "sh"     # Type: shape
    lottie["ix"] = idx      # Index
    lottie["ks"] = {}
    gen_properties_shapeKeyframed(lottie["ks"], layer, index.inc())
