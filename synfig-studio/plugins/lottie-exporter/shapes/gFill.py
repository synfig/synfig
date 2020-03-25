"""
Will have all the functions required for generate the fill(color) in lottie
"""

import sys
from lxml import etree
import settings
from properties.value import gen_properties_value
from properties.valueKeyframed import gen_value_Keyframed
from common.misc import is_animated, real_high_precision
from common.Count import Count
from common.Gradient import Gradient
sys.path.append("..")

def gen_linear_gradient(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/gFill.json

    Args:
    """
    index = Count()
    lottie["ty"] = "gf"
    lottie["r"] = 1    # Don't know it's meaning yet, but works without this also
    lottie["o"] = {}   # Opacity of the gradient layer
    lottie["nm"] = layer.get_description()
    lottie["t"] = 1    # 1 means linear gradient layer
    lottie["s"] = {}   # Starting point of gradient
    lottie["e"] = {}   # Ending point of gradient
    lottie["g"] = {}   # Gradient information is stored here

    # Color Opacity
    opacity = layer.get_param("amount").get()
    is_animate = is_animated(opacity[0])
    if is_animate == 2:
        # Telling the function that this is for opacity
        opacity[0].attrib['type'] = 'opacity'
        gen_value_Keyframed(lottie["o"], opacity[0], index.inc())

    else:
        if is_animate == 0:
            val = float(opacity[0].attrib["value"]) * settings.OPACITY_CONSTANT
        else:
            val = float(opacity[0][0][0].attrib["value"]) * settings.OPACITY_CONSTANT
        gen_properties_value(lottie["o"],
                             val,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)

        # Starting point
        p1 = layer.get_param("p1")
        p1.animate("vector")
        p1.fill_path(lottie, "s")

        # Ending point
        p2 = layer.get_param("p2")
        p2.animate("vector")
        p2.fill_path(lottie, "e")

        # Gradient colors
        lottie["g"]["k"] = {}
        lottie["g"]["ix"] = index.inc()
        gradient = layer.get_param("gradient")
        modify_gradient(gradient)
        gradient.animate("gradient")  # To find the lottie path of the modified gradient
        lottie["g"]["p"] = len(gradient.get()[0][0][0])
        gradient.fill_path(lottie["g"], "k")


def modify_gradient(gradient):
    """
    Finds the number of different positions where the color is defined in a gradient layer, and adds the color
    to each of those position for every waypoint

    Args:
        gradient (common.Param.Param) : Synfig format gradient layer

    Returns:
        (None)
    """
    positions = []
    gradient.animate("gradient")    # This is called initially so as to ensure that each color is inside a waypoint 
    for waypoint in gradient.get()[0]:
        gd = waypoint[0]
        for color in gd:
            positions.append(float(color.attrib["pos"]))
    positions.sort()

    # Synfig uses real_high_precision to differentiate 2 items
    discard = set()
    for i in range(len(positions)):
        for j in range(i+1, len(positions)):
            if abs(positions[i] - positions[j]) < real_high_precision():
                discard.add(j)
    discard_values = [positions[i] for i in discard]

    for val in discard_values:
        positions.remove(val)

    # Now the color is added to all the positions in all the waypoints
    for waypoint in gradient.get()[0]:
        gd = Gradient(waypoint[0])
        add_colors_to_gradient(waypoint, gd, positions)

    
def add_colors_to_gradient(waypoint, gd, positions):
    """
    Deletes previous values of color and introduce new depending upon the positions given

    Args:
        waypoint () : 
        gd () :

    Returns:
        (None)
    """
    st = "<gradient></gradient>"
    new_gradient = etree.fromstring(st)

    # First remove all the colors, easy to remove all first and then add all+some additionals back
    for col in waypoint[0]:
        col.getparent().remove(col)

    # color template
    st = "<color pos='{pos}'><r>{red}</r><g>{green}</g><b>{blue}</b><a>{alpha}</a></color>"

    # Now add all the colors back
    for val in positions:
        color = gd.get_color_at_x(val)
        lxml_col = etree.fromstring(st.format(pos=val, red=color.red, green=color.green, blue=color.blue, alpha=color.alpha))
        waypoint[0].append(lxml_col)
