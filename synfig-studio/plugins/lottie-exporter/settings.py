"""
settings.py
This module contains all the global variables and constants
"""

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
TANGENT_FACTOR = 3.0
IN_TANGENT_X = 0.58
IN_TANGENT_Y = 1
OUT_TANGENT_X = 0.42
OUT_TANGENT_Y = 0

def init():
    # Final converted dictionary
    global lottie_format
    lottie_format = {}
    global view_box_canvas
    view_box_canvas = {}
