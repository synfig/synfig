"""
Will store all functions needed to generate the circle layer in lottie
This will also support the simple_circle layer of Synfig
"""

import sys
import settings
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
    lottie["ty"] = "el"     # Type: circle
    lottie["p"] = {}        # Position of circle
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of circle
    lottie["ix"] = idx      # setting the index

    # Radius
    radius = layer.get_param("radius")
    radius.scale_convert_link(2)        # Lottie format expects diameter of the circle as input
    radius.animate("circle_radius")
    radius.fill_path(lottie, "s")

    # Origin
    origin = layer.get_param("origin", "center")
    origin.animate("vector")
    origin.fill_path(lottie, "p")
