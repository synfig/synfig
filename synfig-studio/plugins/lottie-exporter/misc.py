"""
helpers.py
Some helper functions will be written here
"""
import settings

class Count:
    """
    Class to keep count of variable
    """
    def __init__(self):
        self.idx = -1
    def inc(self):
        """
        This method increases the count by 1 and returns the new count
        """
        self.idx += 1
        return self.idx

def calculate_pixels_per_unit():
    """ 
    Gives the value of 1 unit in terms of pixels according to the canvas defined
    """
    image_width = float(settings.lottie_format["w"])
    image_area_width = settings.view_box_canvas["val"][2] - settings.view_box_canvas["val"][0]
    settings.PIX_PER_UNIT = image_width / image_area_width
    return settings.PIX_PER_UNIT

def change_axis(x_val, y_val):
    """
    Convert synfig axis coordinates into lottie format
    x_val and y_val should be in pixels
    """
    x_val, y_val = float(x_val), float(y_val)
    x_val, y_val = x_val + settings.lottie_format["w"]/2, -y_val + settings.lottie_format["h"]/2
    return [int(x_val), int(y_val)]

def parse_position(animated, i):
    """
    To convert the synfig coordinates from units(initially a string) to pixels
    """
    pos = [float(animated[i][0][0].text),
           float(animated[i][0][1].text)]
    pos = [settings.PIX_PER_UNIT*x for x in pos]
    return pos

def get_angle(theta):
    """
    Converts the .sif angle into lottie angle
    .sif uses positive x-axis as the start point and goes anticlockwise
    lottie uses positive y-axis as the start point and goes clockwise
    """
    theta = int(theta)
    theta = theta % 360
    if theta < 90:
        theta = 90 - theta
    else:
        theta = 450 - theta
    theta = theta % 360
    return theta
