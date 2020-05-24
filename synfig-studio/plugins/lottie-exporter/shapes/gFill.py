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


def gen_radial_gradient(lottie, layer, idx):
    """
    Generates the dictionary correspnding to shapes/gFill.json but with radial gradient

    Args:
    """
    index = Count()
    lottie["ty"] = "gf"
    lottie["r"] = 1    # Don't know it's meaning yet, but works without this also
    lottie["o"] = {}   # Opacity of the gradient layer
    lottie["nm"] = layer.get_description()
    lottie["t"] = 2    # 2 means radial gradient layer
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

        # Gradient colors
        lottie["g"]["k"] = {}
        lottie["g"]["ix"] = index.inc()
        gradient = layer.get_param("gradient")
        modify_gradient(gradient)
        gradient.animate("gradient")  # To find the lottie path of the modified gradient
        lottie["g"]["p"] = len(gradient.get()[0][0][0])
        gradient.fill_path(lottie["g"], "k")
        modify_gradient_according_to_latest_version(lottie["g"]["k"])

        # Starting point and Ending points need to be retrieved from center and radius
        center = layer.get_param("center")
        center.animate("vector")
        center.fill_path(lottie, "s")

        radius = layer.get_param("radius")
        radius.animate("real")

        # Ending point will be (start[0] + radius, start[1])
        # Below is just a modification of fill_path function
        expression = "var $bm_rt; $bm_rt = {expr}"
        x_expr = "sum(" + center.expression + "[0], " + radius.expression + ")"
        y_expr = center.expression + "[1]"
        expr = "[" + x_expr + ", " + y_expr + "]"
        expression = expression.format(expr=expr)
        gen_properties_value(lottie["e"],
                             [1, 1],
                             0,
                             0,
                             expression)
        if "ef" not in center.get_layer().get_lottie_layer().keys():
                center.get_layer().get_lottie_layer()["ef"] = []
        # If center has any expression controllers, then they would have been pushed earlier by fill_path, hence no need
        # center.get_layer().get_lottie_layer()["ef"].extend(center.expression_controllers)
        center.get_layer().get_lottie_layer()["ef"].extend(radius.expression_controllers)



def gen_linear_gradient(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/gFill.json but with linear gradient

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
        modify_gradient_according_to_latest_version(lottie["g"]["k"])


def modify_gradient_according_to_latest_version(lottie):
    """
    Modifies the gradient's "g" property to put an "s" value at each keyframe instead of having "s" & "e"
    both.
    Reference: https://github.com/airbnb/lottie-web/issues/2055#issuecomment-605455945

    Args:
        lottie (dict) : Lottie format gradient colors

    Returns:
        (None)
    """
    for i in range(len(lottie["k"]) - 1, -1, -1):
        if i != 0:
            lottie["k"][i]["s"] = lottie["k"][i-1]["e"]
        # temp = lottie["k"][i].pop("e", "No key found")


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
