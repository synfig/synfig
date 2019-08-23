# pylint: disable=line-too-long
"""
Will store all the functions corresponding to Image Layer in lottie
"""

import sys
import copy
from lxml import etree
import settings
from helpers.transform import gen_helpers_transform
from helpers.blendMode import get_blend
from common.misc import is_animated, get_frame
from common.Layer import Layer
from common.Param import Param
from sources.image import add_image_asset
from shapes.rectangle import to_Synfig_axis
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from synfig.animation import print_animation
sys.path.append("..")


def gen_layer_image(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/image.json

    Args:
        lottie (dict)       : Lottie generated image stored here
        layer  (common.Layer.Layer) : Synfig format image layer
        idx    (int)        : Stores the index(number of) of image layer

    Returns:
        (None)
    """
    layer.add_offset()

    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_IMAGE_TYPE
    lottie["nm"] = layer.get_description()
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    settings.lottie_format["assets"].append({})
    st = add_image_asset(settings.lottie_format["assets"][-1], layer)
    asset = settings.lottie_format["assets"][-1]

    # setting class (jpg, png)
    lottie["cl"] = asset["p"].split(".")[-1]

    # setting the reference id
    lottie["refId"] = asset["id"]

    st["tl"].animate("vector")
    st["br"].animate("vector")

    st["scale"] = gen_image_scale(st["tl"], st["br"], asset["w"], asset["h"])
    # Animation of this scale is needed again, as helpers/transform does not do
    # path calculation again
    st["scale"].animate("image_scale")

    anchor = settings.DEFAULT_ANCHOR
    rotation = settings.DEFAULT_ROTATION
    opacity = layer.get_param("amount")
    opacity.animate("opacity")

    gen_helpers_transform(lottie["ks"], st["tl"], anchor, st["scale"], rotation, opacity)


    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT

    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)


def gen_image_scale(animated_1, animated_2, width, height):
    """
    In Synfig, no scale parameter is available for image layer, so it will be
    created here for Lottie conversion

    Args:
        animated_1 (common.Param.Param): point1 animation in Synfig format
        animated_2 (common.Param.Param): point2 animation in Synfig format
        width      (int)                : Width of the original image
        height     (int)                : Height of the original image

    Returns:
        (lxml.etree._Element) : Scale parameter in Synfig format
    """
    st = '<param name="image_scale"><real value="0.0000000000"/></param>'
    root = etree.fromstring(st)
    image_scale = Param(root, None)
    #image_scale.animate("image_scale")
    image_scale.animate_without_path("image_scale")


    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    animated_1.update_frame_window(window)
    animated_2.update_frame_window(window)
    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    fr = window["first"]

    # Filling the first 2 frames with there original scale values
    fill_image_scale_at_frame(image_scale[0], animated_1, animated_2, width, height, fr)
    fill_image_scale_at_frame(image_scale[0], animated_1, animated_2, width, height, fr + 1)
    fr += 2

    while fr <= window["last"]:
        new_waypoint = copy.deepcopy(root[0][0])
        time = fr / settings.lottie_format["fr"]
        time = str(time) + "s"
        new_waypoint.attrib["time"] = time
        root[0].append(new_waypoint)
        fill_image_scale_at_frame(image_scale[0], animated_1, animated_2, width, height, fr)
        fr += 1
    return image_scale


def fill_image_scale_at_frame(scale_animated, animated_1, animated_2, width, height, frame):
    """
    Generates the scale at a given frame according to point1 and point2 in
    comparison with original width and height of image

    Args:
        scale_animated (lxml.etree._Element) : Scale animation in Synfig format
        animated_1     (common.Param.Param) : point1 animation in Synfig format
        animated_2     (common.Param.Param) : point2 animation in Synfig format
        anim1_path     (dict)                : point1 animation in Lottie format
        anim2_path     (dict)                : point2 animation in Lottie format
        width          (int)                 : Width of original image
        height         (int)                 : Height of original image
        frame          (int)                 : Frame at which scale is to generated

    Returns:
        (None)
    """
    pos1 = animated_1.get_value(frame)
    pos2 = animated_2.get_value(frame)
    pos1, pos2 = to_Synfig_axis(pos1, "vector"), to_Synfig_axis(pos2, "vector")
    pos1 = [x * settings.PIX_PER_UNIT for x in pos1]
    pos2 = [x * settings.PIX_PER_UNIT for x in pos2]

    scale_x = (pos2[0] - pos1[0]) * 100 / width
    scale_y = (pos1[1] - pos2[1]) * 100 / height

    # Assumption: all frames till the maximum are present in the animation
    scale_animated[frame][0].attrib["value"] = str(scale_x)
    scale_animated[frame][0].attrib["value2"] = str(scale_y)
    scale_animated[frame].attrib["before"] = "linear"
    scale_animated[frame].attrib["after"] = "linear"
