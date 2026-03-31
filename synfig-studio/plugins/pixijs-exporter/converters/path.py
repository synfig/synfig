"""
path.py — Convert Synfig BLine (Bezier spline) to PixiJS Graphics bezier commands
"""


def gen_bezier_path_js(name, vertices, fill_hex, alpha, closed=True):
    if not vertices:
        return ""
    lines = [f"  const {name} = new Graphics();"]
    v0 = vertices[0]
    lines.append(f"  {name}.moveTo({round(v0['x'], 3)}, {round(v0['y'], 3)});")
    for i in range(1, len(vertices)):
        prev = vertices[i - 1]
        curr = vertices[i]
        cp1x = round(prev["x"] + prev["t2x"] / 3.0, 3)
        cp1y = round(prev["y"] + prev["t2y"] / 3.0, 3)
        cp2x = round(curr["x"] - curr["t1x"] / 3.0, 3)
        cp2y = round(curr["y"] - curr["t1y"] / 3.0, 3)
        ex = round(curr["x"], 3)
        ey = round(curr["y"], 3)
        lines.append(f"  {name}.bezierCurveTo({cp1x}, {cp1y}, {cp2x}, {cp2y}, {ex}, {ey});")
    if closed and len(vertices) > 2:
        prev = vertices[-1]
        curr = vertices[0]
        cp1x = round(prev["x"] + prev["t2x"] / 3.0, 3)
        cp1y = round(prev["y"] + prev["t2y"] / 3.0, 3)
        cp2x = round(curr["x"] - curr["t1x"] / 3.0, 3)
        cp2y = round(curr["y"] - curr["t1y"] / 3.0, 3)
        lines.append(
            f"  {name}.bezierCurveTo({cp1x}, {cp1y}, {cp2x}, {cp2y}, "
            f"{round(curr['x'], 3)}, {round(curr['y'], 3)});"
        )
        lines.append(f"  {name}.closePath();")
    lines.append(f"  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});")
    lines.append(f"  app.stage.addChild({name});")
    return "\n".join(lines) + "\n"


def gen_bezier_stroke_js(name, vertices, stroke_hex, width, alpha, closed=False):
    if not vertices:
        return ""
    lines = [f"  const {name} = new Graphics();"]
    v0 = vertices[0]
    lines.append(f"  {name}.moveTo({round(v0['x'], 3)}, {round(v0['y'], 3)});")
    for i in range(1, len(vertices)):
        prev = vertices[i - 1]
        curr = vertices[i]
        cp1x = round(prev["x"] + prev["t2x"] / 3.0, 3)
        cp1y = round(prev["y"] + prev["t2y"] / 3.0, 3)
        cp2x = round(curr["x"] - curr["t1x"] / 3.0, 3)
        cp2y = round(curr["y"] - curr["t1y"] / 3.0, 3)
        lines.append(
            f"  {name}.bezierCurveTo({cp1x}, {cp1y}, {cp2x}, {cp2y}, "
            f"{round(curr['x'], 3)}, {round(curr['y'], 3)});"
        )
    if closed and len(vertices) > 2:
        lines.append(f"  {name}.closePath();")
    lines.append(f"  {name}.stroke({{ color: {stroke_hex}, width: {width}, alpha: {alpha} }});")
    lines.append(f"  app.stage.addChild({name});")
    return "\n".join(lines) + "\n"
