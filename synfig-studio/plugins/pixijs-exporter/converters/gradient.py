"""
gradient.py — Generate PixiJS gradient fills
PixiJS v8 uses FillGradient for linear gradients.
"""


def gen_linear_gradient_js(name, x0, y0, x1, y1, color_stops, canvas_w, canvas_h):
    lines = [f"  const {name} = new Graphics();"]
    lines.append(f"  {name}.rect(0, 0, {canvas_w}, {canvas_h});")
    lines.append(f"  const {name}_grad = new FillGradient({{")
    lines.append(f"    type: 'linear',")
    lines.append(f"    x0: {x0}, y0: {y0}, x1: {x1}, y1: {y1},")
    stops_str = ", ".join(
        f"{{ offset: {s[0]}, color: '{_hex_to_css(s[1])}', alpha: {s[2]} }}"
        for s in color_stops
    )
    lines.append(f"    colorStops: [{stops_str}],")
    lines.append(f"  }});")
    lines.append(f"  {name}.fill({name}_grad);")
    lines.append(f"  app.stage.addChild({name});")
    return "\n".join(lines) + "\n"


def gen_radial_gradient_js(name, cx, cy, radius, color_stops):
    lines = [f"  const {name} = new Graphics();"]
    lines.append(f"  const {name}_grad = new FillGradient({{")
    lines.append(f"    type: 'radial',")
    lines.append(f"    x0: {cx}, y0: {cy}, r0: 0,")
    lines.append(f"    x1: {cx}, y1: {cy}, r1: {radius},")
    stops_str = ", ".join(
        f"{{ offset: {s[0]}, color: '{_hex_to_css(s[1])}', alpha: {s[2]} }}"
        for s in color_stops
    )
    lines.append(f"    colorStops: [{stops_str}],")
    lines.append(f"  }});")
    lines.append(f"  {name}.circle({cx}, {cy}, {radius});")
    lines.append(f"  {name}.fill({name}_grad);")
    lines.append(f"  app.stage.addChild({name});")
    return "\n".join(lines) + "\n"


def _hex_to_css(hex_str):
    return "#" + hex_str[2:]
