"""
Will store all the functions corresponding to Image Layer in lottie
"""

import sys
import copy
from lxml import etree
import settings
from helpers.transform import gen_helpers_transform
from misc import Count, change_axis, get_vector, is_animated, get_frame
from helpers.blendMode import get_blend
from sources.image import add_image_asset
from shapes.rectangle import gen_dummy_waypoint, print_animation, get_vector_at_frame
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
sys.path.append("..")


def gen_layer_image(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/image.json
    """
    index = Count()
    lottie["ddd"] = settings.DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = settings.LAYER_IMAGE_TYPE
    lottie["nm"] = settings.LAYER_IMAGE_NAME + str(idx)
    lottie["sr"] = settings.LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled

    settings.lottie_format["assets"].append({})
    st = add_image_asset(settings.lottie_format["assets"][-1], layer)
    asset = settings.lottie_format["assets"][-1]

    # setting class (jpg, png)
    lottie["cl"] = asset["p"].split(".")[-1] 

    # setting the reference id
    lottie["refId"] = asset["id"]

    pos1_animate = is_animated(st["tl"][0])
    pos2_animate = is_animated(st["br"][0])
    # If pos1 is not animated
    if pos1_animate in {0, 1}:
        st["tl"] = gen_dummy_waypoint(st["tl"], pos1_animate, "vector")
    # If pos2 is not animated
    if pos2_animate in {0, 1}:
        st["br"] = gen_dummy_waypoint(st["br"], pos2_animate, "vector")

    st["scale"] = gen_image_scale(st["tl"][0], st["br"][0], asset["w"], asset["h"])
    anchor = [0, 0, 0]

    gen_helpers_transform(lottie["ks"], layer, st["tl"][0], anchor, st["scale"][0])


    lottie["ao"] = settings.LAYER_DEFAULT_AUTO_ORIENT
    
    lottie["ip"] = settings.lottie_format["ip"]
    lottie["op"] = settings.lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    get_blend(lottie, layer)
    lottie["markers"] = []      # Markers to be filled yet


def gen_image_scale(animated_1, animated_2, width, height):
    """
    In Synfig, no scale parameter is available for image layer, so it will be
    created here for Lottie conversion
    """ 
    st = '<param name="image_scale"><real value="0.0000000000"/></param>'
    root = etree.fromstring(st)
    is_animate = is_animated(root)
    root = gen_dummy_waypoint(root, is_animate, "image_scale")

    anim1_path, anim2_path = {}, {}
    gen_properties_multi_dimensional_keyframed(anim1_path, animated_1, 0)
    gen_properties_multi_dimensional_keyframed(anim2_path, animated_2, 0)

    # Filling the first 2 frames with there original scale values
    fill_image_scale_at_frame(root[0], animated_1, animated_2, anim1_path, anim2_path, width, height, 0)
    fill_image_scale_at_frame(root[0], animated_1, animated_2, anim1_path, anim2_path, width, height, 1)

    mx_fr = max(get_frame(animated_1[-1]), get_frame(animated_2[-1]))
    fr = 2
    while fr <= mx_fr:
        new_waypoint = copy.deepcopy(root[0][0])
        time = fr / settings.lottie_format["fr"]
        time = str(time) + "s"
        new_waypoint.attrib["time"] = time
        root[0].append(new_waypoint)
        fill_image_scale_at_frame(root[0], animated_1, animated_2, anim1_path, anim2_path, width, height, fr)
        fr += 1
    return root


def fill_image_scale_at_frame(scale_animated, animated_1, animated_2, anim1_path, anim2_path, width, height, frame):
    """
    
    """
    pos1 = get_vector_at_frame(anim1_path, frame)
    pos2 = get_vector_at_frame(anim2_path, frame)
    
    scale_x = abs(pos1[0] - pos2[0]) * 100 / width
    scale_y = abs(pos1[1] - pos2[1]) * 100 / height

    # Assumption: all frames till the maximum are present in the animation
    scale_animated[frame][0].attrib["value"] = str(scale_x)
    scale_animated[frame][0].attrib["value2"] = str(scale_y)
