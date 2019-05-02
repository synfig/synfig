"""
This module converts the canvas to lottie format
"""
import settings
from misc import calculate_pixels_per_unit

def gen_canvas(lottie, root):
    """ 
    Generates the canvas for the lottie format
    It is the outer most dictionary in the lottie json format
    """
    settings.view_box_canvas["val"] = [float(itr)
                              for itr in root.attrib["view-box"].split()]
    if "width" in root.attrib.keys():
        lottie["w"] = int(root.attrib["width"])
    else:
        lottie["w"] = settings.DEFAULT_WIDTH

    if "height" in root.attrib.keys():
        lottie["h"] = int(root.attrib["height"])
    else:
        lottie["h"] = settings.DEFAULT_HEIGHT

    name = settings.DEFAULT_NAME
    for child in root:
        if child.tag == "name":
            name = child.text
            break
    lottie["nm"] = name
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["v"] = settings.LOTTIE_VERSION
    lottie["fr"] = float(root.attrib["fps"])
    lottie["ip"] = float(root.attrib["begin-time"][:-1])
    time = root.attrib["end-time"].split(" ")
    # Adding time in seconds
    lottie["op"] = float(time[0][:-1]) * lottie["fr"]
    # Adding time in frames
    if len(time) > 1:
        lottie["op"] += float(time[1][:-1])
    calculate_pixels_per_unit()
