"""
Will store all functions needed to generate the rectangle layer in lottie
"""
import sys
import settings
from properties.value import gen_properties_value
from misc import Count, is_animated, change_axis
from properties.multiDimensionalKeyframed import gen_properties_multi_dimensional_keyframed
from properties.valueKeyframed import gen_value_Keyframed
sys.path.append("..")

def gen_shapes_rectangle(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/rect.json
    """
    index = Count()
    lottie["ty"] = "rc"     # Type: rectangle
    lottie["p"] = {}        # Position of rectangle
    lottie["d"] = settings.DEFAULT_DIRECTION
    lottie["s"] = {}        # Size of rectangle
    lottie["ix"] = idx      # setting the index
    lottie["r"] = {}        # Rounded corners of rectangle
    points = {}

    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "point1":
                points["1"] = child # Store address of child here

            elif child.attrib["name"] == "point2":
                points["2"] = child # Store address of child here

            elif child.attrib["name"] == "bevel":
                is_animate = is_animated(child[0])
                if is_animate == 2:
                    #gen_value_Keyframed(lottie["r"], child[0], index.inc())
                    pass
                else:
                    bevel = 0
                    if is_animate == 0:
                        bevel = float(child[0].attrib["value"])
                    else:
                        bevel = float(child[0][0][0].attrib["value"])
                    bevel *= settings.PIX_PER_UNIT
                    gen_properties_value(lottie["r"],
                                         bevel,
                                         index.inc(),
                                         settings.DEFAULT_ANIMATED,
                                         settings.NO_INFO)
    p1_animate = is_animated(points["1"])
    p2_animate = is_animated(points["2"])

    if p1_animate in {0, 1} and p2_animate in {0, 1}:
        child = points["1"]
        if p1_animate == 0:
            x1_val = float(child[0][0].text) * settings.PIX_PER_UNIT
            y1_val = float(child[0][1].text) * settings.PIX_PER_UNIT
        else:
            x1_val = float(child[0][0][0][0].text) * settings.PIX_PER_UNIT
            y1_val = float(child[0][0][0][1].text) * settings.PIX_PER_UNIT

        child = points["2"]
        if p2_animate == 0:
            x2_val = float(child[0][0].text) * settings.PIX_PER_UNIT
            y2_val = float(child[0][1].text) * settings.PIX_PER_UNIT
        else:
            x2_val = float(child[0][0][0][0].text) * settings.PIX_PER_UNIT
            y2_val = float(child[0][0][0][1].text) * settings.PIX_PER_UNIT

        x_val = (x1_val + x2_val) / 2
        y_val = (y1_val + y2_val) / 2
        print(x_val, y_val)
        gen_properties_value(lottie["p"],
                            change_axis(x_val, y_val),
                            index.inc(),
                            settings.DEFAULT_ANIMATED,
                            settings.NO_INFO)

        size = [abs(x2_val - x1_val), abs(y2_val - y1_val)]
        gen_properties_value(lottie["s"],
                             size,
                             index.inc(),
                             settings.DEFAULT_ANIMATED,
                             settings.NO_INFO)
