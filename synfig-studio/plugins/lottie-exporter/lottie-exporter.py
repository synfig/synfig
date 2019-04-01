"""
Python plugin to convert the .sif format into lottie json format
input   : FILE_NAME.sif
output  : FILE_NAME.json

Currently working for only star layers without animations
Partially working for animations
"""
from lxml import etree
import json
import sys

class Count:
    """
    Class to keep count of variable
    """
    def __init__(self):
        self.idx = -1
    def inc(self):
        """
        This method increases the count by 1 and returns the new count
        """
        self.idx += 1
        return self.idx

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
GAMMA = 2.2
PIX_PER_UNIT = 0
TANGENT_FACTOR = 3

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
    calculate_pixels_per_unit()

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

def change_axis(x_val, y_val):
    """
    Convert synfig axis coordinates into lottie format
    x_val and y_val should be in pixels
    """
    x_val, y_val = float(x_val), float(y_val)
    x_val, y_val = x_val + lottie_format["w"]/2, -y_val + lottie_format["h"]/2
    return [int(x_val), int(y_val)]

def gen_helpers_transform(lottie, layer):
    """
    Generates the dictionary corresponding to helpers/transform.json
    """
    index = Count()
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
                    lottie["o"], val, index.inc(), DEFAULT_ANIMATED, NO_INFO)
            elif child.attrib["name"] == "origin":
                if child[0].tag == "vector":
                    x_val = float(child[0][0].text) * PIX_PER_UNIT
                    y_val = float(child[0][1].text) * PIX_PER_UNIT
                    gen_properties_value(lottie["p"], change_axis(x_val, y_val),
                                         index.inc(), DEFAULT_ANIMATED, NO_INFO)
                else:
                    gen_properties_value(lottie["p"], [0, 0],
                                         index.inc(), DEFAULT_ANIMATED, NO_INFO)

    gen_properties_value(
        lottie["r"],
        DEFAULT_ROTATION,
        index.inc(),
        DEFAULT_ANIMATED,
        NO_INFO)
    gen_properties_value(
        lottie["a"], [
            0, 0, 0], index.inc(), DEFAULT_ANIMATED, NO_INFO)
    gen_properties_value(
        lottie["s"], [
            100, 100, 100], index.inc(), DEFAULT_ANIMATED, NO_INFO)

def calculate_pixels_per_unit():
    """
    Gives the value of 1 unit in terms of pixels according to the canvas defined
    """
    image_width = float(lottie_format["w"])
    image_area_width = view_box_canvas["val"][2] - view_box_canvas["val"][0]
    global PIX_PER_UNIT
    PIX_PER_UNIT = image_width / image_area_width
    return PIX_PER_UNIT

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
        theta = 450 - theta
    theta = theta % 360
    return theta

def parse_position(animated, i):
    """
    To convert the synfig coordinates from units(initially a string) to pixels
    """
    pos = [float(animated[i][0][0].text),
           float(animated[i][0][1].text)]
    pos = [PIX_PER_UNIT*x for x in pos]
    return pos

def gen_properties_offset_keyframe(lottie, animated, i):
    """
     Generates the dictionary corresponding to properties/offsetKeyFrame.json
    """
    waypoint, next_waypoint = animated[i], animated[i+1]
    cur_get_after, next_get_before = waypoint.attrib["after"], next_waypoint.attrib["before"]
    # Calculate positions of waypoints
    cur_pos = parse_position(animated, i)
    prev_pos = cur_pos
    next_pos = parse_position(animated, i + 1)
    after_next_pos = next_pos
    if i + 2 <= len(animated) - 1:
        after_next_pos = parse_position(animated, i + 2)
    if i - 1 >= 0:
        prev_pos = parse_position(animated, i - 1)

    lottie["i"] = {}    # Time bezier curve, not used in synfig
    lottie["o"] = {}    # Time bezier curve, not used in synfig
    lottie["i"]["x"] = 0.5
    lottie["i"]["y"] = 0.5
    lottie["o"]["x"] = 0.5
    lottie["o"]["y"] = 0.5
    lottie["t"] = float(waypoint.attrib["time"][:-1]) * lottie_format["fr"]
    lottie["s"] = change_axis(cur_pos[0], cur_pos[1])
    lottie["e"] = change_axis(next_pos[0], next_pos[1])
    tens, bias, cont = 0, 0, 0   # default values
    tens1, bias1, cont1 = 0, 0, 0
    if "tension" in waypoint.keys():
        tens = float(waypoint.attrib["tension"])
    if "continuity" in waypoint.keys():
        cont = float(waypoint.attrib["continuity"])
    if "bias" in waypoint.keys():
        bias = float(waypoint.attrib["bias"])
    if "tension" in next_waypoint.keys():
        tens1 = float(next_waypoint.attrib["tension"])
    if "continuity" in next_waypoint.keys():
        cont1 = float(next_waypoint.attrib["continuity"])
    if "bias" in next_waypoint.keys():
        bias1 = float(next_waypoint.attrib["bias"])
    lottie["to"] = []
    lottie["ti"] = []
    for dim in range(len(cur_pos)):
        # iter           next
        # ANY/TCB ------ ANY/ANY 
        if cur_get_after == "auto":
            if i >= 1:
                out_val = ((1 - tens) * (1 + bias) * (1 + cont) *\
                           (cur_pos[dim] - prev_pos[dim]))/2 +\
                           ((1 - tens) * (1 - bias) * (1 - cont) *\
                           (next_pos[dim] - cur_pos[dim]))/2
            else:
                out_val = next_pos[dim] - cur_pos[dim]      # t1 = p2 - p1

        # iter           next
        # ANY/LINEAR --- ANY/ANY
        if cur_get_after == "linear":
            out_val = next_pos[dim] - cur_pos[dim]

        # iter          next
        # ANY/ANY ----- LINEAR/ANY
        if next_get_before == "linear":
            in_val = next_pos[dim] - cur_pos[dim]

        # iter           next           after_next
        # ANY/ANY ------ TCB/ANY ------ ANY/ANY
        if next_get_before == "auto":
            if i + 2 <= len(animated) - 1:
                in_val = ((1 - tens1) * (1 + bias1) * (1 - cont1) *\
                          (next_pos[dim] - cur_pos[dim]))/2 +\
                          ((1 - tens1) * (1 - bias1) * (1 + cont1) *\
                          (after_next_pos[dim] - next_pos[dim]))/2
            else:
                in_val = next_pos[dim] - cur_pos[dim]       # t2 = p2 - p1
        lottie["to"].append(out_val)
        lottie["ti"].append(in_val)

    # Lottie and synfig use different tangents SEE DOCUMENTATION
    lottie["ti"] = [-item for item in lottie["ti"]]

    # Lottie tangent length is larger than synfig
    lottie["ti"] = [item/TANGENT_FACTOR for item in lottie["ti"]]
    lottie["to"] = [item/TANGENT_FACTOR for item in lottie["to"]]

    # IMPORTANT to and ti have to be relative
    # The y-axis is different in lottie
    lottie["ti"][1] = -lottie["ti"][1]
    lottie["to"][1] = -lottie["to"][1]


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
        gen_properties_offset_keyframe(lottie["k"][-1], animated, i)
    last_waypoint_time = float(animated[-1].attrib["time"][:-1]) * lottie_format["fr"]
    lottie["k"].append({})
    lottie["k"][-1]["t"] = last_waypoint_time

    # Time adjust of the curves
    timeadjust = 0.5
    for i in range(len(animated) - 1):
        if i == 0:
            continue
        time_span_cur = lottie["k"][i+1]["t"] - lottie["k"][i]["t"]
        time_span_prev = lottie["k"][i]["t"] - lottie["k"][i-1]["t"]
        for dim in range(len(lottie["k"][i]["to"])):
            lottie["k"][i]["to"][dim] = lottie["k"][i]["to"][dim] *\
            (time_span_cur * (timeadjust + 1)) /\
            (time_span_cur * timeadjust + time_span_prev)

            if i + 2 <= len(animated) - 1:
                time_span_next = lottie["k"][i+2]["t"] - lottie["k"][i+1]["t"]
                lottie["k"][i]["ti"][dim] = lottie["k"][i]["ti"][dim] *\
                (time_span_cur * (timeadjust + 1)) /\
                (time_span_cur * timeadjust + time_span_next)



def gen_shapes_star(lottie, layer, idx):
    """
    Generates the dictionary corresponding to shapes/star.json
    """
    index = Count()
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
                                     index.inc(),
                                     DEFAULT_ANIMATED,
                                     NO_INFO)
            elif child.attrib["name"] == "angle":
                theta = get_angle(float(child[0].attrib["value"]))
                gen_properties_value(
                    lottie["r"], theta, index.inc(), DEFAULT_ANIMATED, NO_INFO)
            elif child.attrib["name"] == "radius1":
                r_outer = float(child[0].attrib["value"])
                gen_properties_value(
                    lottie["or"], int(
                        PIX_PER_UNIT * r_outer), index.inc(), DEFAULT_ANIMATED, NO_INFO)
            elif child.attrib["name"] == "radius2":
                r_inner = float(child[0].attrib["value"])
                gen_properties_value(
                    lottie["ir"], int(
                        PIX_PER_UNIT * r_inner), index.inc(), DEFAULT_ANIMATED, NO_INFO)
            elif child.attrib["name"] == "origin":
                if child[0].tag == "animated":
                    gen_properties_multi_dimensional_keyframed(lottie["p"],
                                                             child[0], index.inc())
                else:
                    gen_properties_value(lottie["p"], [0, 0],
                                         index.inc(), DEFAULT_ANIMATED, NO_INFO)

    if regular_polygon == "false":
        lottie["sy"] = 1    # Star Type
    else:
        lottie["sy"] = 2    # Polygon Type
    gen_properties_value(lottie["is"], 0, index.inc(), DEFAULT_ANIMATED, NO_INFO)
    gen_properties_value(lottie["os"], 0, index.inc(), DEFAULT_ANIMATED, NO_INFO)
    lottie["ix"] = idx

def gen_shapes_fill(lottie, layer):
    """
    Generates the dictionary corresponding to shapes/fill.json
    """
    index = Count()
    lottie["ty"] = "fl"     # Type if fill
    lottie["c"] = {}       # Color
    lottie["o"] = {}       # Opacity of the fill layer
    for child in layer:
        if child.tag == "param":
            if child.attrib["name"] == "color":
                red = float(child[0][0].text)
                green = float(child[0][1].text)
                blue = float(child[0][2].text)
                red, green, blue = red ** (1/GAMMA), green ** (1/GAMMA), blue ** (1/ GAMMA)
                a_val = child[0][3].text
                gen_properties_value(
                    lottie["c"], [
                        red, green, blue, a_val], index.inc(), DEFAULT_ANIMATED, NO_INFO)

    gen_properties_value(
        lottie["o"],
        DEFAULT_OPACITY,
        index.inc(),
        DEFAULT_ANIMATED,
        NO_INFO)

def gen_layer_shape(lottie, layer, idx):
    """
    Generates the dictionary corresponding to layers/shape.json
    """
    index = Count()
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
        gen_shapes_star(lottie["shapes"][0], layer, index.inc())
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
    tree = etree.parse(FILE_NAME)
    root = tree.getroot()  # canvas
    gen_canvas(lottie_format, root)

    num_layers = Count()
    lottie_format["layers"] = []
    for child in root:
        if child.tag == "layer":
            if child.attrib["type"] != "star":  # Only star conversion
                continue
            lottie_format["layers"].insert(0, {})
            gen_layer_shape(
                lottie_format["layers"][0],
                child,
                num_layers.inc())

    lottie_string = json.dumps(lottie_format)
    # Write the output to the file name with .json extension
    NEW_FILE_NAME = FILE_NAME.split(".")
    # Comment this when this file is not used as plugin
    NEW_FILE_NAME = NEW_FILE_NAME[:-2]
    NEW_FILE_NAME[-1] = "json"
    NEW_FILE_NAME = ".".join(NEW_FILE_NAME)
    outputfile_f = open(NEW_FILE_NAME, 'w')
    outputfile_f.write(lottie_string)
    outputfile_f.close()
