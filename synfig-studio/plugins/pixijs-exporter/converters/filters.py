"""
filters.py — Generate PixiJS filter JS code from Synfig layer parameters.
"""


def gen_blur_filter_js(target_name, strength_x, strength_y):
    """Generate JS to apply a BlurFilter to a PixiJS display object.

    Args:
        target_name: JS variable name of the target display object
        strength_x: Horizontal blur strength (pixels)
        strength_y: Vertical blur strength (pixels)

    Returns:
        JS code string that applies a BlurFilter
    """
    return (
        f"  {target_name}.filters = [new BlurFilter({{ strengthX: {strength_x}, strengthY: {strength_y} }})];\n"
    )
