"""
PixiJS exporter global settings and constants
"""
from common.Count import Count

DEFAULT_WIDTH = 480
DEFAULT_HEIGHT = 270
DEFAULT_NAME = "Synfig Animation"
DEFAULT_BG_COLOR = "0x000000"
PIXI_VERSION = "8.4.1"
PIXI_CDN = "https://cdn.jsdelivr.net/npm/pixi.js@8.4.1/dist/pixi.min.mjs"

SHAPE_LAYER = {"simple_circle", "linear_gradient", "radial_gradient"}
SOLID_LAYER = {"solid_color", "SolidColor"}
SHAPE_SOLID_LAYER = {"region", "polygon", "advanced_outline", "outline", "circle", "rectangle", "filled_rectangle", "star"}
IMAGE_LAYER = {"import"}
GROUP_LAYER = {"group", "switch"}
PRE_COMP_LAYER = {"rotate", "zoom", "translate", "stretch"}
BLUR_LAYER = {"blur"}
TEXT_LAYER = {"text"}
SKELETON_LAYER = {"skeleton"}

FLOAT_PRECISION = 3
GAMMA = [2.2, 2.2, 2.2]
PIX_PER_UNIT = 0
LEVEL = 0

def init():
    global pixi_format, view_box_canvas, num_images, file_name
    global layer_count, canvas_count
    pixi_format = {}
    view_box_canvas = {}
    num_images = Count()
    file_name = {}
    layer_count = Count()
    canvas_count = Count()
