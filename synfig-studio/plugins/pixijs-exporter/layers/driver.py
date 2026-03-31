"""
driver.py — Main layer dispatch: reads Synfig XML layers and generates PixiJS JS code.
"""
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from converters.color import parse_synfig_color
from converters.transform import synfig_to_pixi_coords
from converters.shapes import gen_circle_js, gen_rectangle_js, gen_star_js, gen_solid_rect_js
from converters.animation import waypoints_to_tween_js, gen_tween_setup_js, gen_tween_play_js

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

def _parse_animated_real(param_el, fps):
    """Extract waypoints from an <animated> element inside a param."""
    if param_el is None:
        return None
    animated = param_el.find("animated")
    if animated is None:
        return None
    waypoints = []
    for wp in animated.findall("waypoint"):
        time_str = wp.attrib.get("time", "0s")
        # Parse time string like "0s", "1s", "0.5s", or frame-based "12"
        if time_str.endswith("s"):
            time_sec = float(time_str[:-1])
        else:
            time_sec = float(time_str) / fps

        real_el = wp.find("real")
        if real_el is None:
            continue
        value = float(real_el.attrib.get("value", "0"))

        waypoints.append({
            "time": time_sec,
            "value": value,
            "before": wp.attrib.get("before", "linear"),
            "after": wp.attrib.get("after", "linear"),
            "tension": float(wp.attrib.get("tension", "0")),
            "continuity": float(wp.attrib.get("continuity", "0")),
            "bias": float(wp.attrib.get("bias", "0")),
        })
    return waypoints if waypoints else None

def _parse_animated_origin(param_el, fps, canvas_w, canvas_h, ppu):
    """Extract x/y waypoints from animated origin (vector type)."""
    if param_el is None:
        return None, None
    animated = param_el.find("animated")
    if animated is None:
        return None, None
    x_waypoints = []
    y_waypoints = []
    for wp in animated.findall("waypoint"):
        time_str = wp.attrib.get("time", "0s")
        if time_str.endswith("s"):
            time_sec = float(time_str[:-1])
        else:
            time_sec = float(time_str) / fps

        vec = wp.find("vector")
        if vec is None:
            continue
        sx = float(vec.findtext("x", "0"))
        sy = float(vec.findtext("y", "0"))
        px, py = synfig_to_pixi_coords(sx, sy, canvas_w, canvas_h, ppu)

        base = {
            "time": time_sec,
            "before": wp.attrib.get("before", "linear"),
            "after": wp.attrib.get("after", "linear"),
            "tension": float(wp.attrib.get("tension", "0")),
            "continuity": float(wp.attrib.get("continuity", "0")),
            "bias": float(wp.attrib.get("bias", "0")),
        }
        x_waypoints.append({**base, "value": px})
        y_waypoints.append({**base, "value": py})

    return (x_waypoints if x_waypoints else None,
            y_waypoints if y_waypoints else None)

def _append_origin_tweens(tween_parts, name, layer_el, fps, canvas_w, canvas_h, ppu):
    """Check for animated origin and append tween code. Returns True if tweens added."""
    origin_param = _get_param(layer_el, "origin")
    x_wps, y_wps = _parse_animated_origin(origin_param, fps, canvas_w, canvas_h, ppu)
    if x_wps or y_wps:
        tween_parts.append(gen_tween_setup_js(name))
        if x_wps:
            tween_parts.append(waypoints_to_tween_js(name, "x", x_wps))
        if y_wps:
            tween_parts.append(waypoints_to_tween_js(name, "y", y_wps))
        tween_parts.append(gen_tween_play_js(name, loop=True))
        return True
    return False

def gen_pixi_layers(root, canvas_w, canvas_h, ppu, fps=24):
    global _counter
    _counter = 0
    js_parts = []
    tween_parts = []
    has_animations = False
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
            name = _next_name("circle")
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            radius = _parse_real(layer_el, "radius", 1.0) * ppu
            fill_hex, alpha = _parse_color(layer_el)
            js_parts.append(gen_circle_js(name, cx, cy, radius, fill_hex, alpha * amount))

            if _append_origin_tweens(tween_parts, name, layer_el, fps, canvas_w, canvas_h, ppu):
                has_animations = True

        elif layer_type in ("rectangle", "filled_rectangle"):
            name = _next_name("rect")
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            fill_hex, alpha = _parse_color(layer_el)
            expand = _parse_real(layer_el, "expand", 1.0) * ppu * 2
            js_parts.append(gen_rectangle_js(name, cx - expand/2, cy - expand/2, expand, expand, fill_hex, alpha * amount))

            if _append_origin_tweens(tween_parts, name, layer_el, fps, canvas_w, canvas_h, ppu):
                has_animations = True

        elif layer_type == "star":
            name = _next_name("star")
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            fill_hex, alpha = _parse_color(layer_el)
            r1 = _parse_real(layer_el, "radius1", 1.0) * ppu
            r2 = _parse_real(layer_el, "radius2", 0.5) * ppu
            points = int(_parse_real(layer_el, "points", 5))
            js_parts.append(gen_star_js(name, cx, cy, points, r1, r2, fill_hex, alpha * amount))

            if _append_origin_tweens(tween_parts, name, layer_el, fps, canvas_w, canvas_h, ppu):
                has_animations = True

        elif layer_type in SOLID:
            fill_hex, alpha = _parse_color(layer_el)
            js_parts.append(gen_solid_rect_js(_next_name("solid"), canvas_w, canvas_h, fill_hex, alpha * amount))

        elif layer_type in GROUP:
            name = _next_name("group")
            js_parts.append(f"  const {name} = new Container();\n")
            js_parts.append(f"  app.stage.addChild({name});\n")
            for child in layer_el:
                if child.tag == "canvas":
                    child_code, child_has_anim = gen_pixi_layers(child, canvas_w, canvas_h, ppu, fps)
                    child_code = child_code.replace("app.stage.addChild", f"{name}.addChild")
                    js_parts.append(child_code)
                    if child_has_anim:
                        has_animations = True

    if not js_parts and not tween_parts:
        return ("  // No supported layers found\n", False)

    result = "".join(js_parts)
    if tween_parts:
        result += "\n  // Animation tweens\n"
        result += "".join(tween_parts)
        has_animations = True
    return (result, has_animations)
