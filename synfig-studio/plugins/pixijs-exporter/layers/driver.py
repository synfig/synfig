"""
driver.py — Main layer dispatch: reads Synfig XML layers and generates PixiJS JS code.
"""
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from converters.color import parse_synfig_color
from converters.transform import synfig_to_pixi_coords
from converters.shapes import gen_circle_js, gen_rectangle_js, gen_star_js, gen_solid_rect_js

SHAPE_SOLID = {"region", "polygon", "advanced_outline", "outline", "circle",
               "rectangle", "filled_rectangle", "star"}
SOLID = {"solid_color", "SolidColor"}
GROUP = {"group", "switch"}

_counter = 0

def _next_name(prefix):
    global _counter
    _counter += 1
    return f"{prefix}_{_counter}"

def _get_param(layer_el, name):
    for p in layer_el:
        if p.tag == "param" and p.attrib.get("name") == name:
            return p
    return None

def _parse_origin(layer_el, canvas_w, canvas_h, ppu):
    param = _get_param(layer_el, "origin")
    if param is not None:
        vec = param.find("vector")
        if vec is not None:
            sx = float(vec.findtext("x", "0"))
            sy = float(vec.findtext("y", "0"))
            return synfig_to_pixi_coords(sx, sy, canvas_w, canvas_h, ppu)
    return canvas_w / 2.0, canvas_h / 2.0

def _parse_color(layer_el):
    param = _get_param(layer_el, "color")
    if param is not None:
        color_el = param.find("color")
        if color_el is not None:
            return parse_synfig_color(color_el)
    return "0xffffff", 1.0

def _parse_real(layer_el, name, default=0.0):
    param = _get_param(layer_el, name)
    if param is not None:
        real_el = param.find("real")
        if real_el is not None:
            return float(real_el.attrib.get("value", str(default)))
    return default

def gen_pixi_layers(root, canvas_w, canvas_h, ppu):
    global _counter
    _counter = 0
    js_parts = []
    layers = [el for el in root if el.tag == "layer"]

    for layer_el in reversed(layers):
        layer_type = layer_el.attrib.get("type", "")
        active = layer_el.attrib.get("active", "true") == "true"
        if not active:
            continue
        amount = _parse_real(layer_el, "amount", 1.0)
        if amount <= 0:
            continue

        if layer_type in ("circle", "simple_circle"):
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            radius = _parse_real(layer_el, "radius", 1.0) * ppu
            fill_hex, alpha = _parse_color(layer_el)
            js_parts.append(gen_circle_js(_next_name("circle"), cx, cy, radius, fill_hex, alpha * amount))

        elif layer_type in ("rectangle", "filled_rectangle"):
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            fill_hex, alpha = _parse_color(layer_el)
            expand = _parse_real(layer_el, "expand", 1.0) * ppu * 2
            js_parts.append(gen_rectangle_js(_next_name("rect"), cx - expand/2, cy - expand/2, expand, expand, fill_hex, alpha * amount))

        elif layer_type == "star":
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            fill_hex, alpha = _parse_color(layer_el)
            r1 = _parse_real(layer_el, "radius1", 1.0) * ppu
            r2 = _parse_real(layer_el, "radius2", 0.5) * ppu
            points = int(_parse_real(layer_el, "points", 5))
            js_parts.append(gen_star_js(_next_name("star"), cx, cy, points, r1, r2, fill_hex, alpha * amount))

        elif layer_type in SOLID:
            fill_hex, alpha = _parse_color(layer_el)
            js_parts.append(gen_solid_rect_js(_next_name("solid"), canvas_w, canvas_h, fill_hex, alpha * amount))

        elif layer_type in GROUP:
            name = _next_name("group")
            js_parts.append(f"  const {name} = new Container();\n")
            js_parts.append(f"  app.stage.addChild({name});\n")
            for child in layer_el:
                if child.tag == "canvas":
                    child_code = gen_pixi_layers(child, canvas_w, canvas_h, ppu)
                    child_code = child_code.replace("app.stage.addChild", f"{name}.addChild")
                    js_parts.append(child_code)

    if not js_parts:
        return "  // No supported layers found\n"
    return "".join(js_parts)
