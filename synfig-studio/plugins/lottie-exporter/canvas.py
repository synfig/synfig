"""
This module converts the canvas to lottie format
"""
import settings
from common.misc import calculate_pixels_per_unit


def calc_time(root, lottie, which):
    """
    Converts the starting time and ending time to lottie format

    Args:
        root   (lxml.etree._Element) : Synfig format animation file
        lottie (dict)                : Lottie format animation file
        which  (str)                 : Differentiates between in time and out time

    Returns:
        (None)
    """
    if which == "ip":
        phase = "begin-time"
    elif which == "op":
        phase = "end-time"
    time = root.attrib[phase].split(" ")
    lottie[which] = 0
    for frame in time:
        # Adding time in hours
        if frame[-1] == "h":
            lottie[which] += float(frame[:-1]) * 60 * 60 * lottie["fr"]
        # Adding time in minutes
        elif frame[-1] == "m":
            lottie[which] += float(frame[:-1]) * 60 * lottie["fr"]
        # Adding time in seconds
        elif frame[-1] == "s":
            lottie[which] += float(frame[:-1]) * lottie["fr"]
        # Adding time in frames
        elif frame[-1] == "f":
            lottie[which] += float(frame[:-1])

    # To support canvas with single frames
    if which == "op":
        lottie[which] += 1

def gen_canvas(lottie, root):
    """
    Generates the canvas for the lottie format
    It is the outer most dictionary in the lottie json format

    Args:
        lottie (dict)               : Lottie format animation file
        root   (lxml.etree._Element): Synfig format animation file

    Returns:
        (None)
    """
    settings.view_box_canvas["val"] = [float(itr) for itr in root.attrib["view-box"].split()]
    if "width" in root.attrib.keys():
        lottie["w"] = int(root.attrib["width"])
    else:
        lottie["w"] = settings.DEFAULT_WIDTH

    if "height" in root.attrib.keys():
        lottie["h"] = int(root.attrib["height"])
    else:
        lottie["h"] = settings.DEFAULT_HEIGHT

    settings.ADDITIONAL_PRECOMP_WIDTH = 4*lottie["w"]
    settings.ADDITIONAL_PRECOMP_HEIGHT = 4*lottie["h"]

    # gamma correction values
    if "gamma-r" in root.attrib.keys():
        settings.GAMMA[0] = float(root.attrib["gamma-r"])
    if "gamma-g" in root.attrib.keys():
        settings.GAMMA[1] = float(root.attrib["gamma-g"])
    if "gamma-b" in root.attrib.keys():
        settings.GAMMA[2] = float(root.attrib["gamma-b"])

    name = settings.DEFAULT_NAME
    for child in root:
        if child.tag == "name":
            name = child.text
            break
    lottie["nm"] = name
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["v"] = settings.LOTTIE_VERSION
    lottie["fr"] = float(root.attrib["fps"])
    lottie["assets"] = []       # Creating array for storing assets
    lottie["markers"] = []      # Creating array for storing markers
    lottie["fonts"] = {"list":[]}        # Creating font element to store font data of a text layer
    calc_time(root, lottie, "ip")
    calc_time(root, lottie, "op")
    calculate_pixels_per_unit()
