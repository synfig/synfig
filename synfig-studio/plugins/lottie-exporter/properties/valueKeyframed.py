"""
Fill this
"""
import sys
import settings
sys.path.append("../")
from properties.timeAdjust import time_adjust
from properties.valueKeyframe import gen_value_Keyframe

def gen_value_Keyframed(lottie, animated, idx):
    """
    Generates the dictionary corresponding to properties/valueKeyframed.json in
    lottie documentation
    """
    lottie["ix"] = idx
    lottie["a"] = 1
    lottie["k"] = []
    for i in range(len(animated) - 1):
        lottie["k"].append({})
        gen_value_Keyframe(lottie["k"], animated, i)
    last_waypoint_time = float(animated[-1].attrib["time"][:-1]) * settings.lottie_format["fr"]
    lottie["k"].append({})
    lottie["k"][-1]["t"] = last_waypoint_time

    if  "h" in lottie["k"][-2].keys():
        lottie["k"][-1]["h"] = 1
        lottie["k"][-1]["s"] = lottie["k"][-2]["e"]

    time_adjust(lottie, animated)
