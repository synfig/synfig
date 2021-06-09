# pylint: disable=line-too-long
"""
This module contains all the global variables and constants
"""

from common.Count import Count

# Constants
FLOAT_PRECISION = 3
LOTTIE_VERSION = "5.3.4"
IN_POINT = 0
OUT_POINT = 1.00000004073083
DEFAULT_WIDTH = 480
DEFAULT_HEIGHT = 270
DEFAULT_NAME = "Synfig Animation"
DEFAULT_3D = 0
DEFAULT_BLEND = 0
LAYER_TEXT_TYPE = 5
LAYER_TEXT_NAME = "Text Layer"
LAYER_SHAPE_TYPE = 4
LAYER_SHAPE_NAME = "Shape Layer "
LAYER_SOLID_TYPE = 1
LAYER_SOLID_NAME = "Solid Layer "
LAYER_IMAGE_TYPE = 2
LAYER_IMAGE_NAME = "Image Layer "
LAYER_PRECOMP_TYPE = 0
LAYER_PRECOMP_NAME = "Pre Comp Layer "
CANVAS_NAME = "Canvas "
LAYER_DEFAULT_STRETCH = 1
LAYER_DEFAULT_AUTO_ORIENT = 0
OPACITY_CONSTANT = 100
DEFAULT_ANIMATED = 0
NO_INFO = "no_info"
DEFAULT_ROTATION = 0
DEFAULT_OPACITY = 100
DEFAULT_DIRECTION = 1
DEFAULT_POSITION = [0, 0]
DEFAULT_ANCHOR = [0, 0, 0]
DEFAULT_SCALE = [100, 100, 100]
DEFAULT_SKEW = 0
GAMMA = [2.2, 2.2, 2.2]     # Default RGB gamma correction values
PIX_PER_UNIT = 0
SEPARATION_CONSTANT = 42    # Separation constant value used in the animation of horizontal separation
TANGENT_FACTOR = 3.0
IN_TANGENT_X = 0.58
IN_TANGENT_Y = 1
OUT_TANGENT_X = 0.42
OUT_TANGENT_Y = 0
EFFECTS_FILL = 21
EFFECTS_FILL_MASK = 10
EFFECTS_ALL_MASK = 7
EFFECTS_COLOR = 2
EFFECTS_INVERT = 7  # same as All mask, don't know why
EFFECTS_HFEATHER = 0    # horizontal feather
EFFECTS_VFEATHER = 0    # vertical feather
EFFECTS_OPACITY = 0     # Opacity ty = 0
EFFECTS_CONTROLLER = 5
EFFECTS_SLIDER = 0
EFFECTS_POINT = 3
MASK_ADDITIVE = "a"
INSIDE_PRECOMP = False  # specifies if we are inside a precomp or not
ADDITIONAL_PRECOMP_WIDTH = 0
ADDITIONAL_PRECOMP_HEIGHT = 0
NOT_SUPPORTED_TEXT = "Layer '%s' is not supported yet. For more information, contact us on Synfig forums or Github page"
NOT_ACTIVE_TEXT = "Layer '%s' is not active"
EXCLUDE_FROM_RENDERING = "Layer '%s' is excluded from rendering"
SHAPE_LAYER = {"simple_circle", "linear_gradient", "radial_gradient"}
BLUR_LAYER = {"blur"}
SOLID_LAYER = {"SolidColor"}
SHAPE_SOLID_LAYER = {"region", "polygon", "outline", "circle", "rectangle", "filled_rectangle", "star"} 
IMAGE_LAYER = {"import"}
PRE_COMP_LAYER = {"rotate", "zoom", "translate", "stretch"}
GROUP_LAYER = {"group", "switch"}
SKELETON_LAYER = {"skeleton"}
TEXT_LAYER = {"text"}
UNKNOWN_LAYER = "unknown_layer"
CONVERT_METHODS = {"add", "atan2","average", "bone", "bone_link", "bone_root", "composite", "cos", "dotproduct", "exp", "fromint", "linear", "logarithm", "power", "radial_composite", "range", "reciprocal", "scale", "sine", "subtract", "switch", "vectorangle", "vectorlength", "vectorx", "vectory", "weighted_average"}
BONES = {"bone", "bone_root"}
DOT_FLAG = 0 #Used for the two types of dot product -> angle and real
BLUR_TYPE = 29
RANGE_FLAG = 0 #Used for if-else expressions
FONT_STYLES = ["Sans Serif","Times New Roman","Calibria","Arial","Courier","Comic Sans"]
# Some waypoint animated definitions
ANIMATED = 2
SINGLE_WAYPOINT = 1
NOT_ANIMATED = 0
LEVEL = 0 #Indicates the depth of a layer
OUTLINE_FLAG = False #Flag to check for outline as outline needs the newer version of bodymovin.js
WAYPOINTS_LIST = []
WITHOUT_VARIABLE_WIDTH = False
TEXT_LAYER_FLAG = False

def init():
    """
    Initialises the final dictionary corresponding to conversion and
    also the canvas dictionary needed in misc functions

    Args:
        (None)

    Returns:
        (None)
    """
    # Final converted dictionary
    global lottie_format
    lottie_format = {}
    global view_box_canvas
    view_box_canvas = {}
    global num_images
    num_images = Count()
    global file_name
    file_name = {}
    global num_precomp
    num_precomp = Count()
    global OUTLINE_GROW    # outline grow param of group layer
    OUTLINE_GROW = [0]
    global layer_count  # will only count the layers which do not have there desc set
    layer_count = Count()
    global canvas_count # will only count the canvas which do not have any names
    canvas_count = Count()
    global controller_count # counts the slider and point effects controller
    controller_count = Count()
    global blur_dictionary #used to make a dictionary of blur layers
    blur_dictionary = {}
    global non_blur_dictionary #used to make a dictionary of all non blur layers
    non_blur_dictionary = {}
