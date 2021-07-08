# pylint: disable=line-too-long
"""
Will store all the functions related to modify and play with rectangle
layer in the Synfig format
"""

import sys
import settings
from lxml import etree
sys.path.append("..")


def gen_dummy_rectangle(layer):
    """
    Generates point1 and point2 required for rectangle generation in synfig format

    Args:
        layer (common.Layer.Layer) : Synfig format layer (for now this can only be linear gradient layer)

    Returns:
        (None)
    """
    # Generate point1 and point2 to fill the whole canvas
    st = "<param name='point1'><vector><x>{x}</x><y>{y}</y></vector></param>"
    st = st.format(x=10*settings.view_box_canvas['val'][0], y=10*settings.view_box_canvas['val'][1])
    point1 = etree.fromstring(st)
    layer.add_param("point1", point1)

    st = "<param name='point2'><vector><x>{x}</x><y>{y}</y></vector></param>"
    st = st.format(x=10*settings.view_box_canvas['val'][2], y=10*settings.view_box_canvas['val'][3])
    point2 = etree.fromstring(st)
    layer.add_param("point2", point2)
