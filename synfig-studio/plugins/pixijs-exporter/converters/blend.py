"""
blend.py — Map Synfig blend modes to PixiJS BLEND_MODES
"""

# Synfig blend method integer → PixiJS blendMode string
BLEND_MAP = {
    0: "normal",         # BLEND_COMPOSITE
    1: "normal",         # BLEND_STRAIGHT (no exact match)
    2: "lighten",        # BLEND_BRIGHTEN
    3: "darken",         # BLEND_DARKEN
    6: "multiply",       # BLEND_MULTIPLY
    8: "color",          # BLEND_COLOR
    9: "hue",            # BLEND_HUE
    10: "saturation",    # BLEND_SATURATION
    11: "luminosity",    # BLEND_LUMINANCE
    12: "add",           # BLEND_ADD
    16: "screen",        # BLEND_SCREEN
    17: "hard-light",    # BLEND_HARD_LIGHT
    18: "difference",    # BLEND_DIFFERENCE
    20: "overlay",       # BLEND_OVERLAY
}


def synfig_blend_to_pixi(blend_int):
    """Convert a Synfig blend mode integer to a PixiJS blendMode string."""
    return BLEND_MAP.get(blend_int, "normal")
