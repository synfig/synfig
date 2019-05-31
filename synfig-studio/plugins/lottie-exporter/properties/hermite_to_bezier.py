"""
Stores all methods necassary for conversion of hermite curve to bezier curve
"""

import sys
sys.path.append("../")


def hermite_to_bezier(lottie, animated):
    """
    Converts a hermite curve into bezier curve
    """
    for i in range(len(animated) - 1):
        cur_get_after = animated[i].attrib["after"]
        next_get_before = animated[i+1].attrib["before"]

        # prev              iter
        # ANY/CONSTANT ---- ANY/ANY
        # ANY/ANY      ---- CONSTANT/ANY
        if cur_get_after == "constant" or next_get_before == "constant":
            continue

        if animated.attrib["type"] == "vector":

            for dim in range(len(lottie["k"][i]["to"])):
                lottie["k"][i]["to"][dim] += lottie["k"][i]["s"][dim]

            for dim in range(len(lottie["k"][i]["ti"])):
                lottie["k"][i]["to"][dim] += lottie["k"][i]["e"][dim]
