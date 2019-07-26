"""
Will store all the functions corresponding to shapes in lottie
"""

import sys
import settings
from common.misc import set_layer_desc
from common.Count import Count
from shapes.star import gen_shapes_star
from shapes.circle import gen_shapes_circle
from shapes.fill import gen_shapes_fill
from shapes.rectangle import gen_shapes_rectangle
from helpers.blendMode import get_blend
from helpers.transform import gen_helpers_transform
import synfig.group as group
sys.path.append("..")


def gen_layer_shape(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/shape.json

    Args:
        lottie (dict)               : Lottie generate shape stored here
        layer  (lxml.etree._Element): Synfig format shape layer
        idx    (int)                : Stores the index(number of) of shape layer

    Returns:
        (None)
    """
    group.update_layer(layer)

    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_SHAPE_TYPE
    set_layer_desc(layer, settings.LAYER_SHAPE_NAME + str(idx), lottie)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    gen_helpers_transform(lottie["ks"], layer)

    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["shapes"] = []   # Shapes to be filled yet
    lottie["shapes"].append({})
    if layer.attrib["type"] == "star":
        gen_shapes_star(lottie["shapes"][0], layer, index.inc())
    elif layer.attrib["type"] in {"circle", "simple_circle"}:
        gen_shapes_circle(lottie["shapes"][0], layer, index.inc())
    elif layer.attrib["type"] in {"filled_rectangle", "rectangle"}:
        gen_shapes_rectangle(lottie["shapes"][0], layer, index.inc())

    lottie["shapes"].append({})  # For the fill or color
    gen_shapes_fill(lottie["shapes"][1], layer)

    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
