"""
Fill this
"""
import sys
import settings
sys.path.append("../")

def gen_properties_value(lottie, val, index, animated, expression):
    """
    Generates the dictionary corresponding to properties/value.json in lottie
    documentation and properties/multidimensional.json
    """
    lottie["k"] = val
    lottie["ix"] = index
    lottie["a"] = animated
    if expression != settings.NO_INFO:
        lottie["x"] = expression
