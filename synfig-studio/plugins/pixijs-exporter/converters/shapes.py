"""
shapes.py — Generate PixiJS Graphics JavaScript for Synfig shape layers
"""


def _fmt(v):
    """Format a number: drop trailing .0 for clean JS output."""
    if isinstance(v, float) and v == int(v):
        return str(int(v))
    return str(v)


def gen_circle_js(name, cx, cy, radius, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.circle({_fmt(cx)}, {_fmt(cy)}, {_fmt(radius)});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""


def gen_rectangle_js(name, x, y, w, h, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.rect({_fmt(x)}, {_fmt(y)}, {_fmt(w)}, {_fmt(h)});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""


def gen_star_js(name, cx, cy, points, outer_r, inner_r, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.star({_fmt(cx)}, {_fmt(cy)}, {_fmt(points)}, {_fmt(outer_r)}, {_fmt(inner_r)});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""


def gen_polygon_js(name, points_list, fill_hex, alpha):
    flat = []
    for x, y in points_list:
        flat.extend([x, y])
    pts_str = ", ".join(str(round(p, 3)) for p in flat)
    return f"""  const {name} = new Graphics();
  {name}.poly([{pts_str}]);
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""


def gen_solid_rect_js(name, w, h, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.rect(0, 0, {_fmt(w)}, {_fmt(h)});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""
