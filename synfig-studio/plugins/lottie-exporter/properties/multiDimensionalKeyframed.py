"""
Will store all the functions required for generation of
multiDimensionalKeyframed file in lottie
"""

import sys
import settings
from properties.offsetKeyframe import gen_properties_offset_keyframe
from properties.timeAdjust import time_adjust
from common.misc import get_frame
sys.path.append("..")


def gen_properties_multi_dimensional_keyframed(lottie, animated, idx):
    """
    Generates the dictionary corresponding to
    properties/multiDimensionalKeyframed.json

    Args:
        lottie   (dict)                : Lottie generated keyframes will be stored here
        animated (lxml.etree._Element) : Synfig format animation
        idx      (int)                 : Index/Count of animation

    Returns:
        (None)
    """
    lottie["a"] = 1
    lottie["ix"] = idx
    lottie["k"] = []
    for i in range(len(animated) - 1):
        lottie["k"].append({})
        gen_properties_offset_keyframe(lottie["k"], animated, i)
    last_waypoint_frame = get_frame(animated[-1])
    lottie["k"].append({})
    lottie["k"][-1]["t"] = last_waypoint_frame

    if  "h" in lottie["k"][-2].keys():
        lottie["k"][-1]["h"] = 1
        lottie["k"][-1]["s"] = lottie["k"][-2]["e"]

    # Time adjust of the curves
    time_adjust(lottie, animated)
