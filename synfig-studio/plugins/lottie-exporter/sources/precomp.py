"""
Will store all the functions corresponding to Image Assets in lottie
"""

import os
import sys
import settings
import layers.driver
sys.path.append("..")


def add_precomp_asset(lottie, root, layer_itr):
    """
    """
    lottie["id"] = "precomp_" + str(settings.num_precomp.inc()) 
    lottie["layers"] = []   # If no layer is added, then might result in an error, keep in mind
    layers.driver.gen_layers(lottie["layers"], root, layer_itr-1)
    return lottie["id"]
