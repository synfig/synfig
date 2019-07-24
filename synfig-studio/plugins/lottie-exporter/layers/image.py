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
from shapes.rectangle import get_vector_at_frame, to_Synfig_axis
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
import synfig.group as group
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
    group.update_layer(layer)

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

    anchor = settings.DEFAULT_ANCHOR
    rotation = settings.DEFAULT_ROTATION
    opacity = layer.get_param("amount").get()

    gen_helpers_transform(lottie["ks"], st["tl"][0], anchor, st["scale"][0], rotation, opacity[0])


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
    image_scale.animate("image_scale")

    animated_1.gen_path("vector")
    animated_2.gen_path("vector")
    anim1_path = animated_1.get_path()
    anim2_path = animated_2.get_path()

    # Filling the first 2 frames with there original scale values
    fill_image_scale_at_frame(image_scale[0], anim1_path, anim2_path, width, height, 0)
    fill_image_scale_at_frame(image_scale[0], anim1_path, anim2_path, width, height, 1)

    mx_fr = max(get_frame(animated_1[0][-1]), get_frame(animated_2[0][-1]))
    fr = 2
    while fr <= mx_fr:
        new_waypoint = copy.deepcopy(root[0][0])
        time = fr / settings.lottie_format["fr"]
        time = str(time) + "s"
        new_waypoint.attrib["time"] = time
        root[0].append(new_waypoint)
        fill_image_scale_at_frame(image_scale[0], anim1_path, anim2_path, width, height, fr)
        fr += 1
    return image_scale


def fill_image_scale_at_frame(scale_animated, anim1_path, anim2_path, width, height, frame):
    """
    Generates the scale at a given frame according to point1 and point2 in
    comparison with original width and height of image

    Args:
        scale_animated (lxml.etree._Element) : Scale animation in Synfig format
        animated_1     (lxml.etree._Element) : point1 animation in Synfig format
        animated_2     (lxml.etree._Element) : point2 animation in Synfig format
        anim1_path     (dict)                : point1 animation in Lottie format
        anim2_path     (dict)                : point2 animation in Lottie format
        width          (int)                 : Width of original image
        height         (int)                 : Height of original image
        frame          (int)                 : Frame at which scale is to generated

    Returns:
        (None)
    """
    pos1 = get_vector_at_frame(anim1_path, frame)
    pos2 = get_vector_at_frame(anim2_path, frame)
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
