"""
Python plugin to convert the .sif format into lottie json format
input   : x.sif
output  : x.json

Currently working for only star layers without animations
"""
import xml.etree.ElementTree as ET
import json
import sys

# Final converted dictionary
lottie_format = {}
view_box_canvas = {}

# Constants
LOTTIE_VERSION = "5.3.4"
IN_POINT = 0
OUT_POINT = 1.00000004073083
DEFAULT_WIDTH = 480
DEFAULT_HEIGHT = 270
DEFAULT_NAME = "Synfig Animation"
DEFAULT_3D = 0
DEFAULT_BLEND = 0
LAYER_SHAPE_TYPE = 4
LAYER_SHAPE_NAME = "Shape Layer "
LAYER_DEFAULT_STRETCH = 1
LAYER_DEFAULT_AUTO_ORIENT = 0
OPACITY_CONSTANT = 100
DEFAULT_ANIMATED = 0
NO_INFO = "no_info"
DEFAULT_ROTATION = 0
DEFAULT_OPACITY = 100

def gen_canvas(lottie, root):
    """
    Generates the canvas for the lottie format
    It is the outer most dictionary in the lottie json format
    """
    view_box_canvas["val"] = [float(itr)
                              for itr in root.attrib["view-box"].split()]
    if "width" in root.attrib.keys():
        lottie["w"] = int(root.attrib["width"])
    else:
        lottie["w"] = DEFAULT_WIDTH

    if "height" in root.attrib.keys():
        lottie["h"] = int(root.attrib["height"])
    else:
        lottie["h"] = DEFAULT_HEIGHT

    name = DEFAULT_NAME
    for child in root:
        if child.tag == "name":
            name = child.text
            break
    lottie["nm"] = name
    lottie["ddd"] = DEFAULT_3D
    lottie["v"] = LOTTIE_VERSION
    lottie["fr"] = float(root.attrib["fps"])
    lottie["ip"] = float(root.attrib["begin-time"][:-1])
    lottie["op"] = float(root.attrib["end-time"][:-1]) * lottie["fr"]

def gen_properties_value(lottie, val, index, animated, expression):
    """
    Generates the dictionary corresponding to properties/value.json in lottie
    documentation and properties/multidimensional.json
    """
    lottie["k"] = val
    lottie["ix"] = index
    lottie["a"] = animated
    if expression != NO_INFO:
        lottie["x"] = expression

def gen_helpers_transform(lottie, layer):
    """
    Generates the dictionary corresponding to helpers/transform.json
    """
    index = 0
    lottie["o"] = {}    # opacity/Amount
    lottie["r"] = {}    # Rotation of the layer
    lottie["p"] = {}    # Position of the layer
    lottie["a"] = {}    # Anchor point of the layer
    lottie["s"] = {}    # Scale of the layer
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "amount":
                val = int(OPACITY_CONSTANT * float(child[0].attrib["value"]))
                gen_properties_value(
                    lottie["o"], val, index, DEFAULT_ANIMATED, NO_INFO)
                index += 1
            elif child.attrib["name"] == "origin":
                x_val = float(child[0][0].text)
                y_val = float(child[0][1].text)
                factor = get_unit()
                x_val = int(factor * x_val) + lottie_format["w"]/2
                y_val = -int(factor * y_val) + lottie_format["h"]/2
                gen_properties_value(lottie["p"], [x_val, y_val, 0], index, DEFAULT_ANIMATED, NO_INFO)
                index += 1

    gen_properties_value(
        lottie["r"],
        DEFAULT_ROTATION,
        index,
        DEFAULT_ANIMATED,
        NO_INFO)
    index += 1
    gen_properties_value(
        lottie["a"], [
            0, 0, 0], index, DEFAULT_ANIMATED, NO_INFO)
    index += 1
    gen_properties_value(
        lottie["s"], [
            100, 100, 100], index, DEFAULT_ANIMATED, NO_INFO)
    index += 1

def get_unit():
    """
    Gives the value of 1 unit in terms of pixels according to the canvas defined
    """
    image_width = float(lottie_format["w"])
    image_area_width = view_box_canvas["val"][2] - view_box_canvas["val"][0]
    factor = image_width / image_area_width
    return factor

def get_angle(theta):
    """
    Converts the .sif angle into lottie angle
    .sif uses positive x-axis as the start point and goes anticlockwise
    lottie uses positive y-axis as the start point and goes clockwise
    """
    theta = int(theta)
    theta = theta % 360
    if theta < 90:
        theta = 90 - theta
    else:
        theta = theta - 90
    return theta

def gen_shapes_star(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/star.json
    """
    index = 0
    lottie["ty"] = "sr"     # Type: star
    lottie["pt"] = {}       # Number of points on the star
    lottie["p"] = {}        # Position of star
    lottie["r"] = {}        # Angle / Star's rotation
    lottie["ir"] = {}       # Inner radius
    lottie["or"] = {}       # Outer radius
    lottie["is"] = {}       # Inner roundness of the star
    lottie["os"] = {}       # Outer roundness of the star
    regular_polygon = "false"
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "regular_polygon":
                regular_polygon = child[0].attrib["value"]
            elif child.attrib["name"] == "points":
                gen_properties_value(lottie["pt"],
                                     int(child[0].attrib["value"]),
                                     index,
                                     DEFAULT_ANIMATED,
                                     NO_INFO)
                index += 1
            elif child.attrib["name"] == "angle":
                theta = get_angle(float(child[0].attrib["value"]))
                gen_properties_value(
                    lottie["r"], theta, index, DEFAULT_ANIMATED, NO_INFO)
                index += 1
            elif child.attrib["name"] == "radius1":
                r_outer = float(child[0].attrib["value"])
                factor = get_unit()
                gen_properties_value(
                    lottie["or"], int(
                        factor * r_outer), index, DEFAULT_ANIMATED, NO_INFO)
            elif child.attrib["name"] == "radius2":
                r_inner = float(child[0].attrib["value"])
                factor = get_unit()
                gen_properties_value(
                    lottie["ir"], int(
                        factor * r_inner), index, DEFAULT_ANIMATED, NO_INFO)

    if regular_polygon == "false":
        lottie["sy"] = 1    # Star Type
    else:
        lottie["sy"] = 2    # Polygon Type
    gen_properties_value(lottie["is"], 0, index, DEFAULT_ANIMATED, NO_INFO)
    index += 1
    gen_properties_value(lottie["os"], 0, index, DEFAULT_ANIMATED, NO_INFO)
    index += 1
    gen_properties_value(lottie["p"],
                         [0, 0],
                         index,
                         DEFAULT_ANIMATED,
                         NO_INFO)

    lottie["ix"] = idx

def gen_shapes_fill(lottie, layer):
    """
    Generates the dictionary corresponding to shapes/fill.json
    """
    index = 0
    lottie["ty"] = "fl"     # Type if fill
    lottie["c"] = {}       # Color
    lottie["o"] = {}       # Opacity of the fill layer
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "color":
                red = child[0][0].text
                green = child[0][1].text
                blue = child[0][2].text
                a_val = child[0][3].text
                gen_properties_value(
                    lottie["c"], [
                        red, green, blue, a_val], index, DEFAULT_ANIMATED, NO_INFO)
                index += 1

    gen_properties_value(
        lottie["o"],
        DEFAULT_OPACITY,
        index,
        DEFAULT_ANIMATED,
        NO_INFO)
    index += 1

def gen_layer_shape(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/shape.json
    """
    index = 0
    lottie["ddd"] = DEFAULT_3D
    lottie["ind"] = idx
    lottie["ty"] = LAYER_SHAPE_TYPE
    lottie["nm"] = LAYER_SHAPE_NAME + str(idx)
    lottie["sr"] = LAYER_DEFAULT_STRETCH
    lottie["ks"] = {}   # Transform properties to be filled
    gen_helpers_transform(lottie["ks"], layer)

    lottie["ao"] = LAYER_DEFAULT_AUTO_ORIENT
    lottie["shapes"] = []   # Shapes to be filled yet
    lottie["shapes"].append({})
    if layer.attrib["type"] == "star":
        gen_shapes_star(lottie["shapes"][0], layer, index)
        index += 1
    lottie["shapes"].append({})  # For the fill or color
    gen_shapes_fill(lottie["shapes"][1], layer)

    lottie["ip"] = lottie_format["ip"]
    lottie["op"] = lottie_format["op"]
    lottie["st"] = 0            # Don't know yet
    lottie["bm"] = DEFAULT_BLEND
    lottie["markers"] = []      # Markers to be filled yet

if len(sys.argv) < 2:
    sys.exit()
else:
    FILE_NAME = sys.argv[1]
    tree = ET.parse(FILE_NAME)
    root = tree.getroot()  # canvas
    gen_canvas(lottie_format, root)

    num_layers = 0
    lottie_format["layers"] = []
    for child in root:
        if child.tag == "layer":
            if child.attrib["type"] != "star":  # Only star conversion
                continue
            lottie_format["layers"].append({})
            gen_layer_shape(
                lottie_format["layers"][num_layers],
                child,
                num_layers)
            num_layers += 1

    lottie_string = json.dumps(lottie_format)
    # Write the output to the file name with .json extension
    NEW_FILE_NAME = FILE_NAME.split(".")[0] + ".json"
    outputfile_f = open(NEW_FILE_NAME, 'w')
    outputfile_f.write(lottie_string)
    outputfile_f.close()
