"""
settings.py
This module contains all the global variables and constants
"""

from misc import Count

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
LAYER_SOLID_TYPE = 1
LAYER_SOLID_NAME = "Solid Layer "
LAYER_IMAGE_TYPE = 2
LAYER_IMAGE_NAME = "Image Layer "
LAYER_DEFAULT_STRETCH = 1
LAYER_DEFAULT_AUTO_ORIENT = 0
OPACITY_CONSTANT = 100
DEFAULT_ANIMATED = 0
NO_INFO = "no_info"
DEFAULT_ROTATION = 0
DEFAULT_OPACITY = 100
DEFAULT_DIRECTION = 1
GAMMA = 2.2
PIX_PER_UNIT = 0
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
MASK_ADDITIVE = "a"


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
