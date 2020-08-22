# pylint: disable=line-too-long
"""
Will store all the functions required for generation of shapeKeyframed file in
lottie
"""

import sys
from properties.shapePropKeyframe.outline import gen_bline_outline
from properties.shapePropKeyframe.region import gen_bline_region
from properties.shapePropKeyframe.polygon import gen_dynamic_list_polygon
from properties.shapePropKeyframe.circle import gen_list_circle
from properties.shapePropKeyframe.rectangle import gen_list_rectangle
from properties.shapePropKeyframe.star import gen_list_star
from common.Param import Param
from common.Layer import Layer
sys.path.append("../")


def gen_properties_shapeKeyframed(lottie, node, idx):
    """
    Will convert bline points/dynamic_list to bezier points as required by lottie if they are
    animated

    Args:
        lottie (dict) : Lottie generated shape keyframes will be stored here
        node   (common.Layer.Layer | common.Param.Param) : Shape/path in Synfig format :- Could be bline_point or dynamic_list
        idx    (int) : Index/Count of shape/path

    Returns:
        (None)
    """
    lottie["ix"] = idx
    lottie["a"] = 1
    lottie["k"] = []
    if isinstance(node, Layer) and node.get_type() == "circle":
        gen_list_circle(lottie["k"], node)
    elif isinstance(node, Layer) and node.get_type() in {"rectangle", "filled_rectangle"}:
        gen_list_rectangle(lottie["k"], node)
    elif isinstance(node, Layer) and node.get_type() == "star":
        gen_list_star(lottie["k"], node)
    elif isinstance(node, Layer) and node.get_type() in {"linear_gradient", "radial_gradient"}:  # rectangle layer is needed for gradients
        gen_list_rectangle(lottie["k"], node)
    elif isinstance(node, Param) and node.get_layer_type() == "region":
        gen_bline_region(lottie["k"], node)
    elif isinstance(node, Param) and node.get_layer_type() == "polygon":
        gen_dynamic_list_polygon(lottie["k"], node)
    elif isinstance(node, Param) and node.get_layer_type() == "outline":
        gen_bline_outline(lottie["k"], node)
