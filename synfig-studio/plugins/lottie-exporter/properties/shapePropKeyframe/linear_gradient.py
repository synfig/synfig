# pylint: disable=line-too-long
"""
Will store all the functions and modules for generation of rectangle layer
required for linear gradient layer in Lottie format
# NOTE: we need rectangle for linear gradient because lottie format does not
support "gradient ramp" effect yet, when it is supported, we can get rid of
this module
"""

import sys
import settings
from lxml import etree
from common.misc import approximate_equal
from common.Vector import Vector
from synfig.animation import to_Synfig_axis
from properties.shapePropKeyframe.helper import add, insert_dict_at, quadratic_to_cubic
from properties.shapePropKeyframe.rectangle import synfig_rectangle
sys.path.append("../../")


def gen_list_rectangle_for_linear_gradient(lottie, layer):
    """
    Generates a shape layer corresponding to rectangle layer required by linear gradient
    by manipulating the parameters of the rectangle

    Args:
        lottie (dict) : Lottie format linear gradient layer will be stored in this
        layer  (common.Layer.Layer) : Synfig format rectangle layer

    Returns:
        (None)
    """
    ################### SECTION 1 #########################
    # Inserting waypoints if not animated and finding the first and last frame
    window = {}
    window["first"] = sys.maxsize
    window["last"] = -1

    # Generate point1 and point2 to fill the whole canvas
    st = "<param name='point1'><vector><x>{x}</x><y>{y}</y></vector></param>"
    st = st.format(x=settings.view_box_canvas['val'][0], y=settings.view_box_canvas['val'][1])
    point1 = etree.fromstring(st)
    point1 = layer.add_param("point1", point1)
    point1.add_offset()

    st = "<param name='point2'><vector><x>{x}</x><y>{y}</y></vector></param>"
    st = st.format(x=settings.view_box_canvas['val'][2], y=settings.view_box_canvas['val'][3])
    point2 = etree.fromstring(st)
    point2 = layer.add_param("point2", point2)
    point2.add_offset()

    # Generate expand for this rectangle
    st = "<param name='expand'><real value='0.0'/></param>"
    expand = etree.fromstring(st)
    expand = layer.add_param("expand", expand)

    # Generate bevel for this rectangle
    st = "<param name='bevel'><real value='0.0'/></param>"
    bevel = etree.fromstring(st)
    bevel = layer.add_param("bevel", bevel)
    st = "<param name='bevCircle'><bool value='false'/></param>"
    bevCircle = etree.fromstring(st)
    bevCircle = layer.add_param("bevCircle", bevCircle)

    # Animating point1
    point1.update_frame_window(window)
    point1.animate("vector")

    # Animating point2
    point2.update_frame_window(window)
    point2.animate("vector")

    # Animating expand
    expand.update_frame_window(window)
    expand.animate("real")

    # Animating bevel
    bevel.update_frame_window(window)
    bevel.animate("real")

    # Animating bevCircle
    bevCircle.update_frame_window(window)
    bevCircle.animate_without_path("bool")

    # Minimizing the window size
    if window["first"] == sys.maxsize and window["last"] == -1:
        window["first"] = window["last"] = 0
    ################# END OF SECTION 1 ###################

    ################ SECTION 2 ###########################
    # Generating values for all the frames in the window

    fr = window["first"]
    while fr <= window["last"]:
        st_val, en_val = insert_dict_at(lottie, -1, fr, False)

        synfig_rectangle(st_val, point1, point2, expand, bevel, bevCircle, fr)
        synfig_rectangle(en_val, point1, point2, expand, bevel, bevCircle, fr + 1)

        fr += 1
    # Setting the final time
    lottie.append({})
    lottie[-1]["t"] = fr
