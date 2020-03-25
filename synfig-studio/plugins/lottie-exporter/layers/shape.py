"""
Will store all the functions corresponding to shapes in lottie
"""

import sys
import settings
from common.Count import Count
from shapes.star import gen_shapes_star
from shapes.circle import gen_shapes_circle
from shapes.fill import gen_shapes_fill
from shapes.rectangle import gen_shapes_rectangle, gen_custom_rectangle
from shapes.gFill import gen_linear_gradient
from helpers.blendMode import get_blend
from helpers.transform import gen_helpers_transform
sys.path.append("..")


def gen_layer_shape(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/shape.json

    Args:
        lottie (dict)       : Lottie generate shape stored here
        layer  (common.Layer.Layer) : Synfig format shape layer
        idx    (int)        : Stores the index(number of) of shape layer

    Returns:
        (None)
    """
    layer.add_offset()

    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_SHAPE_TYPE
    lottie["nm"] = layer.get_description()
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    gen_helpers_transform(lottie["ks"])

    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    lottie["shapes"] = []   # Shapes to be filled yet
    lottie["shapes"].append({})
    if layer.get_type() == "star":
        gen_shapes_star(lottie["shapes"][0], layer, index.inc())
    elif layer.get_type() in {"circle", "simple_circle"}:
        gen_shapes_circle(lottie["shapes"][0], layer, index.inc())
    elif layer.get_type() in {"filled_rectangle", "rectangle"}:
        gen_shapes_rectangle(lottie["shapes"][0], layer.get_layer(), index.inc())
    elif layer.get_type() in {"linear_gradient"}:
        gen_custom_rectangle(lottie["shapes"][0], index.inc())

    lottie["shapes"].append({})  # For the fill or color
    if layer.get_type() in {"linear_gradient"}:
        gen_linear_gradient(lottie["shapes"][1], layer, index.inc())    # yet to be implemented
    else:
        gen_shapes_fill(lottie["shapes"][1], layer)

    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
