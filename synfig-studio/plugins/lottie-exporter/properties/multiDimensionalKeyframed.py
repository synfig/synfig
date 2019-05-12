"""
Will store all the functions required for generation of
multiDimensionalKeyframed file in lottie
"""
import sys
import settings
from properties.offsetKeyframe import gen_properties_offset_keyframe
from properties.timeAdjust import time_adjust
sys.path.append("..")

def gen_properties_multi_dimensional_keyframed(lottie, animated, idx):
    """
    Generates the dictionary corresponding to
    properties/multiDimensionalKeyframed.json
    """
    lottie["a"] = 1
    lottie["ix"] = idx
    lottie["k"] = []
    for i in range(len(animated) - 1):
        lottie["k"].append({})
        gen_properties_offset_keyframe(lottie["k"], animated, i)
    last_waypoint_time = float(animated[-1].attrib["time"][:-1]) * settings.lottie_format["fr"]
    lottie["k"].append({})
    lottie["k"][-1]["t"] = last_waypoint_time

    if  "h" in lottie["k"][-2].keys():
        lottie["k"][-1]["h"] = 1
        lottie["k"][-1]["s"] = lottie["k"][-2]["e"]

    # Time adjust of the curves
    time_adjust(lottie, animated)
