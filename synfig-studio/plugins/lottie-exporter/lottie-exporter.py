"""
Python plugin to convert the .sif format into lottie json format
input   : FILE_NAME.sif
output  : FILE_NAME.json

Currently working for star and circle layers along with animations
"""
import json
import sys
from lxml import etree
from canvas import gen_canvas
from layers.shape import gen_layer_shape
from misc import Count
import settings

def parse(file_name):
    """
    Driver function for parsing .sif to lottie(.json) format
    """
    tree = etree.parse(file_name)
    root = tree.getroot()  # canvas
    gen_canvas(settings.lottie_format, root)

    num_layers = Count()
    settings.lottie_format["layers"] = []
    supported_layers = {"star", "circle", "rectangle"}
    for child in root:
        if child.tag == "layer":
            if child.attrib["active"] == "false":   # Only render the active layers
                continue
            if child.attrib["type"] not in supported_layers:  # Only supported layers
                continue
            settings.lottie_format["layers"].insert(0, {})
            gen_layer_shape(settings.lottie_format["layers"][0],
                            child,
                            num_layers.inc())

    lottie_string = json.dumps(settings.lottie_format)
    # Write the output to the file name with .json extension
    new_file_name = file_name.split(".")
    # Uncomment this when this file is used as plugin
    # new_file_name = new_file_name[:-2]
    new_file_name[-1] = "json"
    new_file_name = ".".join(new_file_name)
    outputfile_f = open(new_file_name, 'w')
    outputfile_f.write(lottie_string)
    outputfile_f.close()

if len(sys.argv) < 2:
    sys.exit()
else:
    settings.init()
    FILE_NAME = sys.argv[1]
    parse(FILE_NAME)
