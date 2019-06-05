"""
Implements all the functions required for generating value for properties
related to layers in Synfig
"""

import sys
import settings
sys.path.append("../")


def gen_properties_value(lottie, val, index, animated, expression):
    """
    Generates the dictionary corresponding to properties/value.json in lottie
    documentation

    Args:
        lottie   (dict)                : Value of property is stored in lottie format
        val      (list | float)        : actual value of parameter
        index    (int)                 : Index/Count of value
        animated (int)                 : Holds if this property is animated or not
        expression (str)               : purpose of this is not clearly stated in lottie

    Returns:
        (None)
    """
    lottie["k"] = val
    lottie["ix"] = index
    lottie["a"] = animated
    if expression != settings.NO_INFO:
        lottie["x"] = expression
