"""
Python plugin to convert .sif format into PixiJS HTML format
input   : FILE_NAME.sif
output  : FILE_NAME.html
"""

import os
import sys
_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
sys.path.insert(0, os.path.join(_THIS_DIR, '..', 'lottie-exporter'))
sys.path.insert(0, _THIS_DIR)

import argparse
from lxml import etree
import settings as pixi_settings


def parse_canvas(root):
    """Parse .sif root element and extract canvas metadata."""
    width = int(root.attrib.get("width", pixi_settings.DEFAULT_WIDTH))
    height = int(root.attrib.get("height", pixi_settings.DEFAULT_HEIGHT))
    fps = float(root.attrib.get("fps", "24"))
    view_box_str = root.attrib.get("view-box", "-4 2.25 4 -2.25")
    view_box = [float(x) for x in view_box_str.split()]
    return {"width": width, "height": height, "fps": fps, "view_box": view_box}


def gen_html(canvas_meta, pixi_js_code, has_animations=False):
    """Generate self-contained HTML with PixiJS."""
    tween_script = ""
    if has_animations:
        tween_path = os.path.join(os.path.dirname(__file__), "runtime", "tween.js")
        try:
            with open(tween_path, 'r') as f:
                tween_runtime = f.read()
            tween_script = f"\n<script>\n{tween_runtime}\n</script>"
        except FileNotFoundError:
            raise FileNotFoundError(
                f"Animation tween runtime not found at {tween_path}. "
                "Cannot generate HTML with animations without runtime/tween.js."
            )

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>{pixi_settings.DEFAULT_NAME}</title>
<style>
  body {{ margin: 0; overflow: hidden; background: #000; }}
  canvas {{ display: block; }}
</style>
</head>
<body>{tween_script}
<script type="module">
import {{ Application, Graphics, Container, FillGradient }} from '{pixi_settings.PIXI_CDN}';

(async () => {{
  const app = new Application();
  await app.init({{
    width: {canvas_meta['width']},
    height: {canvas_meta['height']},
    backgroundColor: {pixi_settings.DEFAULT_BG_COLOR},
    antialias: true,
  }});
  document.body.appendChild(app.canvas);

{pixi_js_code}
}})();
</script>
</body>
</html>"""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile")
    parser.add_argument("outfile")
    ns = parser.parse_args()

    pixi_settings.init()
    tree = etree.parse(ns.infile)
    root = tree.getroot()

    from layers.driver import gen_pixi_layers
    from converters.transform import calc_pixels_per_unit

    canvas_meta = parse_canvas(root)
    ppu = calc_pixels_per_unit(canvas_meta['width'], canvas_meta['height'], canvas_meta['view_box'])
    pixi_js_code, has_animations = gen_pixi_layers(
        root, canvas_meta['width'], canvas_meta['height'], ppu,
        fps=canvas_meta['fps']
    )
    html = gen_html(canvas_meta, pixi_js_code, has_animations=has_animations)

    with open(ns.outfile, 'w', encoding='utf-8') as f:
        f.write(html)


if __name__ == "__main__":
    main()
