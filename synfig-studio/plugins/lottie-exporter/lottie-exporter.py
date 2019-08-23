# pylint: disable=line-too-long
"""
Python plugin to convert the .sif format into lottie json format
input   : FILE_NAME.sif
output  : FILE_NAME.json
        : FILE_NAME.html
        : FILE_NAME.log

Supported Layers are mentioned below
"""
import os
import json
import sys
import logging
from lxml import etree
from canvas import gen_canvas
from layers.driver import gen_layers
from common.misc import modify_final_dump
from common.Canvas import Canvas
import settings


def write_to(filename, extension, data):
    """
    Helps in writing data to a specified file name

    Args:
        filename  (str) : Original file name
        extension (str) : original file name needs to be converted to this
        data      (str) : Data that needs to be written

    Returns:
        (str) : changed file name according to the extension specified
    """
    new_name = filename.split(".")
    new_name[-1] = extension
    new_name = ".".join(new_name)
    with open(new_name, "w") as fil:
        fil.write(data)
    return new_name


def parse(file_name):
    """
    Driver function for parsing .sif to lottie(.json) format

    Args:
        file_name (str) : Synfig file name that needs to be parsed to Lottie format

    Returns:
        (str) : File name in json format
    """
    tree = etree.parse(file_name)
    root = tree.getroot()  # canvas
    gen_canvas(settings.lottie_format, root)

    # Storing the file name
    settings.file_name["fn"] = file_name

    # Storing the file directory
    settings.file_name["fd"] = os.path.dirname(file_name)

    # Initialize the logging
    init_logs()

    settings.lottie_format["layers"] = []
    canvas = Canvas(root)
    gen_layers(settings.lottie_format["layers"], canvas, canvas.get_num_layers() - 1)

    lottie_string = json.dumps(modify_final_dump(settings.lottie_format))
    return write_to(file_name, "json", lottie_string)


def gen_html(file_name):
    """
    Generates an HTML file which will allow end user to easily playback
    animation in a web browser

    Args:
        file_name (str) : Stores the HTML file name

    Returns:
        (None)
    """
    bodymovin_path = os.path.join(os.path.dirname(sys.argv[0]), "bodymovin.js")

    with open(bodymovin_path, "r") as f:
        bodymovin_script = f.read()

    html_text = \
"""<html xmlns="http://www.w3.org/1999/xhtml">
<meta charset="UTF-8">
<head>
    <style>
        body{{
            background-color:#fff;
            margin: 0px;
            height: 100%;
            overflow: hidden;
        }}
        #lottie{{
            background-color:#fff;
            width:100%;
            height:100%;
            display:block;
            overflow: hidden;
            transform: translate3d(0,0,0);
            text-align: center;
            opacity: 1;
        }}

    </style>
</head>
<body>

{bodymovin_script}

<div id="lottie"></div>
<script>
    var animationData = {file_name_data};
    var params = {{
        container: document.getElementById('lottie'),
        renderer: 'svg',
        loop: true,
        autoplay: true,
        animationData: animationData
    }};

    var anim;

    anim = lottie.loadAnimation(params);

</script>
</body>
</html>
"""

    write_to(file_name, "html", html_text.format(bodymovin_script=bodymovin_script, file_name_data=json.dumps(modify_final_dump(settings.lottie_format))))


def init_logs():
    """
    Initializes the logger, sets the file name in which the logs will be stored
    and sets the level of the logging(DEBUG | INFO : depending on what is
    specified)
    """
    name = settings.file_name['fn']
    name = name.split(".")
    name[-1] = 'log'
    name = '.'.join(name)
    path = os.path.join(settings.file_name['fd'], name)
    path = os.path.abspath(name)
    logging.basicConfig(filename=path, filemode='w',
                        format='%(name)s - %(levelname)s - %(message)s')
    logging.getLogger().setLevel(logging.DEBUG)


if len(sys.argv) < 2:
    sys.exit()
else:
    settings.init()
    FILE_NAME = sys.argv[1]
    new_file_name = parse(FILE_NAME)
    gen_html(new_file_name)
