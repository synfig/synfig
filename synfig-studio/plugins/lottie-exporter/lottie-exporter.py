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
from canvas import gen_canvas
from layers.shape import gen_layer_shape
from misc import Count
import settings

if len(sys.argv) < 2:
    sys.exit()
else:
    settings.init()
    FILE_NAME = sys.argv[1]
    tree = etree.parse(FILE_NAME)
    root = tree.getroot()  # canvas
    gen_canvas(settings.lottie_format, root)

    num_layers = Count()
    settings.lottie_format["layers"] = []
    for child in root:
        if child.tag == "layer":
            if child.attrib["active"] == "false":   # Only render the active layers
                continue
            if child.attrib["type"] != "star":  # Only star conversion
                continue
            settings.lottie_format["layers"].insert(0, {})
            gen_layer_shape(
                settings.lottie_format["layers"][0],
                child,
                num_layers.inc())

    lottie_string = json.dumps(settings.lottie_format)
    # Write the output to the file name with .json extension
    NEW_FILE_NAME = FILE_NAME.split(".")
    # Uncomment this when this file is used as plugin
    # NEW_FILE_NAME = NEW_FILE_NAME[:-2]
    NEW_FILE_NAME[-1] = "json"
    NEW_FILE_NAME = ".".join(NEW_FILE_NAME)
    outputfile_f = open(NEW_FILE_NAME, 'w')
    outputfile_f.write(lottie_string)
    outputfile_f.close()
