"""
transform.py — Synfig coordinate system to PixiJS coordinate system conversion
"""
import math

def calc_pixels_per_unit(width, height, view_box):
    """Calculate pixels per Synfig unit from canvas dimensions and view box."""
    vb_width = view_box[2] - view_box[0]
    return width / vb_width

def synfig_to_pixi_coords(sx, sy, canvas_w, canvas_h, ppu):
    """
    Convert Synfig coords (center origin, Y-up, units) to
    PixiJS coords (top-left origin, Y-down, pixels).
    """
    px = round(sx * ppu + canvas_w / 2.0, 3)
    py = round(-sy * ppu + canvas_h / 2.0, 3)
    return px, py

def synfig_to_pixi_angle(deg):
    """
    Convert Synfig angle (degrees, counter-clockwise) to
    PixiJS rotation (radians, clockwise).
    """
    return -deg * math.pi / 180.0

def synfig_to_pixi_scale(sx, sy):
    """Convert Synfig scale values to PixiJS scale."""
    return round(sx, 4), round(sy, 4)
