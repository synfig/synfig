"""
text.py — Generate PixiJS Text JS code from Synfig text layer parameters.
"""
import json
import re


def _hex_to_css(hex_str):
    """Convert '0xRRGGBB' to '#RRGGBB' for CSS usage."""
    return "#" + hex_str[2:] if hex_str.startswith("0x") else hex_str


def _sanitize_font_family(family):
    """Allow only safe characters for CSS font-family values."""
    return re.sub(r"[^a-zA-Z0-9\s,\-]", "", family)


def gen_text_js(name, text, x, y, font_family, font_size, fill_hex, alpha):
    """Generate JS code to create a PixiJS Text object.

    Args:
        name: JS variable name
        text: The text string content
        x: X position in pixels
        y: Y position in pixels
        font_family: CSS font family string
        font_size: Font size in pixels
        fill_hex: Fill color as '0xRRGGBB'
        alpha: Opacity (0.0 to 1.0)

    Returns:
        JS code string creating a PixiJS Text object
    """
    css_color = _hex_to_css(fill_hex)
    safe_text = json.dumps(text).replace("</", "<\\/")
    safe_font = _sanitize_font_family(font_family)
    return (
        f"  const {name} = new Text({{\n"
        f"    text: {safe_text},\n"
        f"    style: {{ fontFamily: '{safe_font}', fontSize: {font_size}, fill: '{css_color}' }},\n"
        f"  }});\n"
        f"  {name}.position.set({x}, {y});\n"
        f"  {name}.alpha = {alpha};\n"
        f"  app.stage.addChild({name});\n"
    )
