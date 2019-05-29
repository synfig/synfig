"""
Will store all the functions corresponding to Image Assets in lottie
"""
import sys
import struct
import imghdr
import settings
sys.path.append("..")

def get_image_size(fname):
    '''
    https://stackoverflow.com/questions/8032642/how-to-obtain-image-size-using-standard-python-class-without-using-external-lib
    Determine the image type of fhandle and return its size.
    from draco'''
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
    """
    lottie["id"] = "image_" + str(settings.num_images.inc())
    st = {}     # Store the address of children
    
    for chld in layer:
        if chld.tag == "param":
            if chld.attrib["name"] == "tl":
                st["tl"] = chld
            elif chld.attrib["name"] == "br":
                st["br"] = chld
            elif chld.attrib["name"] == "filename":
                st["filename"] = chld

    width, height = get_image_size(st["filename"][0].text)
    lottie["w"] = width 

    lottie["h"] = height

    # Later can copy the images into a new folder: images/ for the lottie format
    path = st["filename"][0].text.split("/")
    lottie["p"] = path[-1]
    path = path[:-1]
    path = "/".join(path)
    path = path + "/"       # This `/` is very important
    lottie["u"] = path
    return st
