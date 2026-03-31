"""
color.py — Synfig linear color to PixiJS sRGB conversion
"""
import math

GAMMA = 2.2

def _linear_to_srgb(v):
    """Convert linear color value to sRGB with gamma correction."""
    v = max(0.0, min(1.0, v))
    return int(round(pow(v, 1.0 / GAMMA) * 255))

def synfig_color_to_hex(r, g, b):
    """Convert Synfig linear RGB to PixiJS hex color string."""
    sr = _linear_to_srgb(r)
    sg = _linear_to_srgb(g)
    sb = _linear_to_srgb(b)
    return "0x{:02x}{:02x}{:02x}".format(sr, sg, sb)

def synfig_color_to_rgba(r, g, b, a):
    """Convert Synfig linear RGBA to CSS rgba() string."""
    sr = _linear_to_srgb(r)
    sg = _linear_to_srgb(g)
    sb = _linear_to_srgb(b)
    return "rgba({}, {}, {}, {})".format(sr, sg, sb, round(a, 2))

def parse_synfig_color(color_element):
    """Parse a Synfig XML color element and return (hex, alpha)."""
    r = float(color_element.attrib.get("r", "0"))
    g = float(color_element.attrib.get("g", "0"))
    b = float(color_element.attrib.get("b", "0"))
    a = float(color_element.attrib.get("a", "1"))
    return synfig_color_to_hex(r, g, b), round(a, 3)
