"""
Will store all the functions corresponding to Image Assets in lottie
"""
import sys
import settings
sys.path.append("..")

def add_image_asset(lottie, layer):
    """
    Generates the dictionary corresponding to sources/image.json
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

    lottie["w"] = abs(float(st["tl"][0][0].text) - float(st["br"][0][0].text)) 
    lottie["w"] *= settings.PIX_PER_UNIT

    lottie["h"] = abs(float(st["tl"][0][1].text) - float(st["br"][0][1].text))
    lottie["h"] *= settings.PIX_PER_UNIT

    # Later can copy the images into a new folder: images/ for the lottie format
    path = st["filename"][0].text.split("/")
    lottie["p"] = path[-1]
    path = path[:-1]
    path = "/".join(path)
    path = path + "/"       # This `/` is very important
    lottie["u"] = path
