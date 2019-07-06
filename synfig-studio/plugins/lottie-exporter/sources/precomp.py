"""
Will store all the functions corresponding to Image Assets in lottie
"""

import sys
import settings
import layers.driver
sys.path.append("..")


def add_precomp_asset(lottie, root, layer_itr):
    """
    Generates a unique ID for precomp asset and returns it to the caller
    function

    Args:
        lottie (dict) : Asset will be stored here in lottie format
        root   (lxml.etree._Element) : Canvas for which layers are to be generated
        layer_itr (int) : Index of the layer in canvas

    Returns:
        (str) : Unique ID of the asset
    """
    lottie["id"] = "precomp_" + str(settings.num_precomp.inc())
    lottie["layers"] = []   # If no layer is added, then might result in an error, keep in mind
    layers.driver.gen_layers(lottie["layers"], root, layer_itr-1)
    return lottie["id"]
