"""
Will store all the functions corresponding to Image Assets in lottie
"""

import os
import sys
import struct
import imghdr
import shutil
import settings
sys.path.append("..")


def get_image_size(fname):
    '''
    https://stackoverflow.com/questions/8032642/how-to-obtain-image-size-using-standard-python-class-without-using-external-lib
    Determine the image type of fhandle and return its size.
    from draco

    Args:
        fname (str) : File name

    Returns:
        (int, int) : width and height of image file is returned
        (None)     : If some exception occurs while calculating
    '''
    with open(fname, 'rb') as fhandle:
        head = fhandle.read(24)
        if len(head) != 24:
            return
        if imghdr.what(fname) == 'png':
            check = struct.unpack('>i', head[4:8])[0]
            if check != 0x0d0a1a0a:
                return
            width, height = struct.unpack('>ii', head[16:24])
        elif imghdr.what(fname) == 'gif':
            width, height = struct.unpack('<HH', head[6:10])
        elif imghdr.what(fname) == 'jpeg':
            try:
                fhandle.seek(0) # Read 0xff next
                size = 2
                ftype = 0
                while not 0xc0 <= ftype <= 0xcf:
                    fhandle.seek(size, 1)
                    byte = fhandle.read(1)
                    while ord(byte) == 0xff:
                        byte = fhandle.read(1)
                    ftype = ord(byte)
                    size = struct.unpack('>H', fhandle.read(2))[0] - 2
                # We are at a SOFn block
                fhandle.seek(1, 1)  # Skip `precision' byte.
                height, width = struct.unpack('>HH', fhandle.read(4))
            except Exception: #IGNORE:W0703
                return
        else:
            return
        return width, height


def add_image_asset(lottie, layer):
    """
    Generates the dictionary corresponding to sources/image.json
    Returns: st required in calling function

    Args:
        lottie (dict)                : Lottie layer
        layer  (commong.Layer.Layer) : Synfig layer

    Returns:
        (dict) : Stores address of parameters: "tl", "br", "filename"
    """
    lottie["id"] = "image_" + str(settings.num_images.inc())
    st = {}     # Store the address of children

    st["tl"] = layer.get_param("tl")
    st["br"] = layer.get_param("br")
    st["filename"] = layer.get_param("filename")

    file_path = os.path.join(settings.file_name["fd"], st["filename"][0].text)
    file_path = os.path.abspath(file_path)
    width, height = get_image_size(file_path)
    lottie["w"] = width

    lottie["h"] = height

    images_dir = os.path.join(settings.file_name["fd"], "images")
    images_dir = os.path.abspath(images_dir)
    if not os.path.isdir(images_dir):   # Create images directory if not present
        try:
            os.mkdir(images_dir)
        except OSError:
            print("Creation of the directory %s failed" % images_dir)

    # copy original image to images directory
    src = file_path
    head, tail = os.path.split(file_path)
    new_image_path = os.path.join(images_dir, tail)

    # using shutil to make a copy of the image
    shutil.copy(src, new_image_path)

    # copy meta-data of the file
    shutil.copystat(src, new_image_path)

    lottie["u"] = "images/"
    lottie["p"] = tail
    return st
