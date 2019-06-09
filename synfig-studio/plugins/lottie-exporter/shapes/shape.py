"""
Will store all functions needed to generate the shape(path) layer in lottie
"""

import sys
import settings
from misc import Count
from properties.shape import gen_properties_shape
from properties.shapeKeyframed import gen_properties_shapeKeyframed
sys.path.append("..")


def gen_shapes_shape(lottie, layer, idx):
    index = Count()
    lottie["ty"] = "sh"     # Type: shape
    lottie["ix"] = idx      # Index
    lottie["ks"] = {}
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "bline":
                bline_point = child[0]
                gen_properties_shapeKeyframed(lottie["ks"], bline_point, index.inc())
                #gen_properties_shape(lottie["ks"], bline_point, index.inc())
