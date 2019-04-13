"""
Fill this
"""
import sys
sys.path.append("..")
import settings
from properties.offsetKeyframe import gen_properties_offset_keyframe

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
    timeadjust = 0.5
    for i in range(len(animated) - 1):
        if i == 0:
            continue
        time_span_cur = lottie["k"][i+1]["t"] - lottie["k"][i]["t"]
        time_span_prev = lottie["k"][i]["t"] - lottie["k"][i-1]["t"]
        cur_get_after = animated[i].attrib["after"]
        next_get_before = animated[i+1].attrib["before"]

                # prev              iter
        # ANY/CONSTANT ---- ANY/ANY
        # ANY/ANY      ---- CONSTANT/ANY
        if cur_get_after == "constant" or next_get_before == "constant":
            continue
            
        # prev    --- iter        --- next
        # ANY/ANY --- ANY/!LINEAR --- ANY/ANY
        if cur_get_after != "linear":
            for dim in range(len(lottie["k"][i]["to"])):
                lottie["k"][i]["to"][dim] = lottie["k"][i]["to"][dim] *\
                (time_span_cur * (timeadjust + 1)) /\
                (time_span_cur * timeadjust + time_span_prev)

        # iter    --- next        --- after_next
        # ANY/ANY --- !LINEAR/ANY --- ANY/ANY
        if next_get_before != "linear":
            for dim in range(len(lottie["k"][i]["to"])):
                if i + 2 <= len(animated) - 1:
                    time_span_next = lottie["k"][i+2]["t"] - lottie["k"][i+1]["t"]
                    lottie["k"][i]["ti"][dim] = lottie["k"][i]["ti"][dim] *\
                    (time_span_cur * (timeadjust + 1)) /\
                    (time_span_cur * timeadjust + time_span_next)

