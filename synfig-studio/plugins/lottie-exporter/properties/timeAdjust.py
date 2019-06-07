"""
Stores the functions required for adjusting time factor in the neighbouring
waypoints in Synfig
"""

import sys
sys.path.append("../")


def time_adjust(lottie, animated):
    """
    Adjusts the tangents between neighbouring waypoints depending upon the time
    factor between previous waypoints or next waypoints

    Args:
        lottie   (dict)                : Holds bezier curve in Lottie format
        animated (lxml.etree._Element) : Synfig format animation

    Returns:
        (None)
    """
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

        if animated.attrib["type"] == "real":
            if cur_get_after != "linear":
                lottie["k"][i]["o"]["x"][0] *= (time_span_cur * (timeadjust + 1)) /\
                        (time_span_cur * timeadjust + time_span_prev)
                lottie["k"][i]["o"]["y"][0] *= (time_span_cur * (timeadjust + 1)) /\
                        (time_span_cur * timeadjust + time_span_prev)
            if next_get_before != "linear":
                if i + 2 <= len(animated) - 1:
                    time_span_next = lottie["k"][i+2]["t"] - lottie["k"][i+1]["t"]
                    lottie["k"][i]["i"]["x"][0] *= (time_span_cur * (timeadjust + 1)) /\
                            (time_span_cur * timeadjust + time_span_next)

        elif animated.attrib["type"] == "vector":

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
