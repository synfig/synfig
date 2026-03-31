# PixiJS 匯出器 實作計畫

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 建立 Synfig PixiJS 匯出器 plugin，將 .sif 動畫匯出為自包含的 HTML 檔案，使用 PixiJS v8 WebGL 引擎播放。

**Architecture:** 以 Python plugin 形式實作（與現有 lottie-exporter 平行），重用 lottie-exporter 的 `common/` 模組解析 .sif XML，新增 PixiJS 專用的圖層轉換與 JavaScript 程式碼生成邏輯。匯出產物為自包含 HTML（內嵌 PixiJS CDN 引用 + 動畫 JS）。

**Tech Stack:** Python 3, lxml, pytest, PixiJS v8, anime.js (MIT)

**Jira:** Epic FRON-5327, Stories FRON-5328 / FRON-5329 / FRON-5330

---

## Phase 1: 靜態向量匯出 (FRON-5328)

### Task 1: 建立 Plugin 骨架

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/plugin.xml.in`
- Create: `synfig-studio/plugins/pixijs-exporter/pixijs-exporter.py`
- Create: `synfig-studio/plugins/pixijs-exporter/settings.py`
- Modify: `synfig-studio/plugins/CMakeLists.txt`

**Step 1: 建立 plugin.xml.in**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<plugin>
   <_name>Export to PixiJS format</_name>
   <exporter>
       <extension>html</extension>
       <_description>PixiJS WebGL Animation (*.html)</_description>
       <exec stdout="log">pixijs-exporter.py</exec>
   </exporter>
</plugin>
```

**Step 2: 建立 settings.py**

```python
"""
PixiJS exporter global settings and constants
"""
from common.Count import Count

# Canvas defaults
DEFAULT_WIDTH = 480
DEFAULT_HEIGHT = 270
DEFAULT_NAME = "Synfig Animation"
DEFAULT_BG_COLOR = "0x000000"
PIXI_VERSION = "8.4.1"
PIXI_CDN = "https://cdn.jsdelivr.net/npm/pixi.js@8.4.1/dist/pixi.min.mjs"

# Synfig layer type sets (same as lottie-exporter)
SHAPE_LAYER = {"simple_circle", "linear_gradient", "radial_gradient"}
SOLID_LAYER = {"solid_color", "SolidColor"}
SHAPE_SOLID_LAYER = {"region", "polygon", "advanced_outline", "outline", "circle", "rectangle", "filled_rectangle", "star"}
IMAGE_LAYER = {"import"}
GROUP_LAYER = {"group", "switch"}
PRE_COMP_LAYER = {"rotate", "zoom", "translate", "stretch"}
BLUR_LAYER = {"blur"}
TEXT_LAYER = {"text"}
SKELETON_LAYER = {"skeleton"}

# Precision
FLOAT_PRECISION = 3

# Gamma correction
GAMMA = [2.2, 2.2, 2.2]

# Runtime state
PIX_PER_UNIT = 0
LEVEL = 0

def init():
    global lottie_format, view_box_canvas, num_images, file_name
    global layer_count, canvas_count
    lottie_format = {}
    view_box_canvas = {}
    num_images = Count()
    file_name = {}
    layer_count = Count()
    canvas_count = Count()
```

**Step 3: 建立主程式 pixijs-exporter.py（最小可用版本）**

```python
"""
Python plugin to convert .sif format into PixiJS HTML format
input   : FILE_NAME.sif
output  : FILE_NAME.html
"""

import os
import sys
sys.path.append(os.path.abspath(os.path.dirname(__file__)))
# Reuse lottie-exporter's common modules
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lottie-exporter'))

import json
import argparse
from lxml import etree
from common.Canvas import Canvas
from common.misc import calculate_pixels_per_unit
import settings as pixi_settings

def parse_canvas(root):
    """Parse .sif root element and extract canvas metadata."""
    vb = [float(x) for x in root.attrib["view-box"].split()]
    width = int(root.attrib.get("width", pixi_settings.DEFAULT_WIDTH))
    height = int(root.attrib.get("height", pixi_settings.DEFAULT_HEIGHT))
    fps = float(root.attrib.get("fps", "24"))
    return {"width": width, "height": height, "fps": fps, "view_box": vb}

def gen_html(canvas_meta, pixi_js_code):
    """Generate self-contained HTML with PixiJS."""
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
<body>
<script type="module">
import {{ Application, Graphics, Container }} from '{pixi_settings.PIXI_CDN}';

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

    canvas_meta = parse_canvas(root)
    # Phase 1: static shapes only, no animation
    pixi_js_code = "  // TODO: generate layers"
    html = gen_html(canvas_meta, pixi_js_code)

    with open(ns.outfile, 'w', encoding='utf-8') as f:
        f.write(html)

if __name__ == "__main__":
    main()
```

**Step 4: 修改 CMakeLists.txt 註冊新 plugin**

在 `synfig-studio/plugins/CMakeLists.txt` 中加入：

```cmake
add_subdirectory(pixijs-exporter)
```

並建立 `synfig-studio/plugins/pixijs-exporter/CMakeLists.txt`：

```cmake
configure_file(plugin.xml.in plugin.xml @ONLY)

install(
    FILES
        pixijs-exporter.py
        settings.py
        ${CMAKE_CURRENT_BINARY_DIR}/plugin.xml
    DESTINATION share/synfig/plugins/pixijs-exporter
)
```

**Step 5: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/ synfig-studio/plugins/CMakeLists.txt
git commit -m "feat: add PixiJS exporter plugin skeleton"
```

---

### Task 2: 建立測試環境

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/tests/__init__.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/conftest.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_skeleton.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/requirements.txt`

**Step 1: 建立 requirements.txt**

```
pytest>=7.0
lxml>=4.0
```

**Step 2: 建立 conftest.py**

```python
import os
import sys
import pytest

EXPORTER_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOTTIE_DIR = os.path.join(os.path.dirname(EXPORTER_DIR), 'lottie-exporter')
sys.path.insert(0, EXPORTER_DIR)
sys.path.insert(0, LOTTIE_DIR)

@pytest.fixture
def fixtures_dir():
    return os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(EXPORTER_DIR))),
                        'synfig-core', 'test', 'fixtures')

@pytest.fixture
def exporter_dir():
    return EXPORTER_DIR
```

**Step 3: 建立冒煙測試**

```python
import os
import settings as pixi_settings

def test_settings_init():
    pixi_settings.init()
    assert pixi_settings.DEFAULT_WIDTH == 480
    assert pixi_settings.DEFAULT_HEIGHT == 270

def test_plugin_xml_exists(exporter_dir):
    assert os.path.isfile(os.path.join(exporter_dir, "plugin.xml.in"))

def test_exporter_importable(exporter_dir):
    assert os.path.isfile(os.path.join(exporter_dir, "pixijs-exporter.py"))
```

**Step 4: 執行測試**

```bash
cd synfig-studio/plugins/pixijs-exporter
pip3 install -r tests/requirements.txt
pytest tests/ -v
```

Expected: 3 tests PASS

**Step 5: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/tests/
git commit -m "test: add pytest skeleton for PixiJS exporter"
```

---

### Task 3: 實作色彩轉換工具

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/__init__.py`
- Create: `synfig-studio/plugins/pixijs-exporter/converters/color.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_color_converter.py`

**Step 1: 寫測試**

```python
from converters.color import synfig_color_to_hex, synfig_color_to_rgba

def test_red_to_hex():
    assert synfig_color_to_hex(1.0, 0.0, 0.0) == "0xff0000"

def test_white_to_hex():
    assert synfig_color_to_hex(1.0, 1.0, 1.0) == "0xffffff"

def test_black_to_hex():
    assert synfig_color_to_hex(0.0, 0.0, 0.0) == "0x000000"

def test_color_with_gamma():
    # Synfig stores colors in linear space, PixiJS expects sRGB
    result = synfig_color_to_hex(0.5, 0.5, 0.5)
    # After gamma correction (2.2), 0.5^(1/2.2) ≈ 0.73 → 0xba
    assert result == "0xbababa"

def test_rgba_string():
    result = synfig_color_to_rgba(1.0, 0.0, 0.0, 0.5)
    assert result == "rgba(255, 0, 0, 0.5)"

def test_clamp_overflow():
    assert synfig_color_to_hex(1.5, -0.1, 0.0) == "0xff0000"
```

**Step 2: 執行測試確認失敗**

```bash
pytest tests/test_color_converter.py -v
```

Expected: FAIL (module not found)

**Step 3: 實作**

```python
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
    """Parse a Synfig XML color element and return (r, g, b, a) as hex + alpha."""
    r = float(color_element.attrib.get("r", "0"))
    g = float(color_element.attrib.get("g", "0"))
    b = float(color_element.attrib.get("b", "0"))
    a = float(color_element.attrib.get("a", "1"))
    return synfig_color_to_hex(r, g, b), round(a, 3)
```

**Step 4: 執行測試確認通過**

```bash
pytest tests/test_color_converter.py -v
```

Expected: 6 tests PASS

**Step 5: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/converters/
git add synfig-studio/plugins/pixijs-exporter/tests/test_color_converter.py
git commit -m "feat: add Synfig-to-PixiJS color converter with gamma correction"
```

---

### Task 4: 實作座標轉換工具

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/transform.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_transform.py`

**Step 1: 寫測試**

Synfig 座標系：原點在中心，Y 軸向上，單位為 unit (60px/unit)。
PixiJS 座標系：原點在左上角，Y 軸向下，單位為 pixel。

```python
from converters.transform import synfig_to_pixi_coords, synfig_to_pixi_angle

def test_center_origin():
    """Synfig (0, 0) should map to canvas center."""
    x, y = synfig_to_pixi_coords(0.0, 0.0, 480, 270, 60.0)
    assert x == 240.0
    assert y == 135.0

def test_top_left():
    """Synfig negative-x, positive-y should map to top-left area."""
    x, y = synfig_to_pixi_coords(-4.0, 2.25, 480, 270, 60.0)
    assert x == 0.0
    assert y == 0.0

def test_bottom_right():
    x, y = synfig_to_pixi_coords(4.0, -2.25, 480, 270, 60.0)
    assert x == 480.0
    assert y == 270.0

def test_angle_conversion():
    """Synfig angle in degrees (counter-clockwise) to PixiJS radians (clockwise)."""
    import math
    assert synfig_to_pixi_angle(0.0) == 0.0
    assert abs(synfig_to_pixi_angle(90.0) - (-math.pi / 2)) < 1e-6

def test_ppu_calculation():
    from converters.transform import calc_pixels_per_unit
    ppu = calc_pixels_per_unit(480, 270, [-4.0, 2.25, 4.0, -2.25])
    assert ppu == 60.0
```

**Step 2: 實作**

```python
"""
transform.py — Synfig coordinate system to PixiJS coordinate system conversion
"""
import math

def calc_pixels_per_unit(width, height, view_box):
    """Calculate pixels per Synfig unit from canvas dimensions and view box."""
    vb_width = view_box[2] - view_box[0]   # right - left
    return width / vb_width

def synfig_to_pixi_coords(sx, sy, canvas_w, canvas_h, ppu):
    """
    Convert Synfig coords (center origin, Y-up, units) to
    PixiJS coords (top-left origin, Y-down, pixels).
    """
    px = round(sx * ppu + canvas_w / 2.0, 3)
    py = round(-sy * ppu + canvas_h / 2.0, 3)
    return px, py

def synfig_to_pixi_angle(deg):
    """
    Convert Synfig angle (degrees, counter-clockwise) to
    PixiJS rotation (radians, clockwise).
    """
    return -deg * math.pi / 180.0

def synfig_to_pixi_scale(sx, sy):
    """Convert Synfig scale values to PixiJS scale."""
    return round(sx, 4), round(sy, 4)
```

**Step 3: 執行測試**

```bash
pytest tests/test_transform.py -v
```

Expected: 5 tests PASS

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/converters/transform.py
git add synfig-studio/plugins/pixijs-exporter/tests/test_transform.py
git commit -m "feat: add Synfig-to-PixiJS coordinate transform utilities"
```

---

### Task 5: 實作基本形狀圖層轉換

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/shapes.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_shapes.py`

**Step 1: 寫測試**

```python
from converters.shapes import gen_circle_js, gen_rectangle_js, gen_star_js

def test_circle_js():
    code = gen_circle_js("circle1", 240.0, 135.0, 50.0, "0xff0000", 1.0)
    assert "new Graphics()" in code
    assert ".circle(240, 135, 50)" in code
    assert ".fill(" in code
    assert "0xff0000" in code

def test_rectangle_js():
    code = gen_rectangle_js("rect1", 100.0, 100.0, 200.0, 150.0, "0x00ff00", 1.0)
    assert ".rect(100, 100, 200, 150)" in code

def test_star_js():
    code = gen_star_js("star1", 240.0, 135.0, 5, 60.0, 30.0, "0xffff00", 1.0)
    assert ".star(240, 135, 5, 60, 30)" in code

def test_circle_with_alpha():
    code = gen_circle_js("c1", 0, 0, 10, "0xffffff", 0.5)
    assert "alpha" in code or "0.5" in code
```

**Step 2: 實作**

```python
"""
shapes.py — Generate PixiJS Graphics JavaScript for Synfig shape layers
"""

def gen_circle_js(name, cx, cy, radius, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.circle({cx}, {cy}, {radius});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""

def gen_rectangle_js(name, x, y, w, h, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.rect({x}, {y}, {w}, {h});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""

def gen_star_js(name, cx, cy, points, outer_r, inner_r, fill_hex, alpha):
    return f"""  const {name} = new Graphics();
  {name}.star({cx}, {cy}, {points}, {outer_r}, {inner_r});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""

def gen_polygon_js(name, points_list, fill_hex, alpha):
    """points_list: [(x1,y1), (x2,y2), ...]"""
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
    """Full-canvas solid color layer."""
    return f"""  const {name} = new Graphics();
  {name}.rect(0, 0, {w}, {h});
  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});
  app.stage.addChild({name});
"""
```

**Step 3: 執行測試**

```bash
pytest tests/test_shapes.py -v
```

Expected: 4 tests PASS

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/converters/shapes.py
git add synfig-studio/plugins/pixijs-exporter/tests/test_shapes.py
git commit -m "feat: add PixiJS shape generators (circle, rect, star, polygon)"
```

---

### Task 6: 實作 Bezier 路徑轉換

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/path.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_path.py`

**Step 1: 寫測試**

```python
from converters.path import gen_bezier_path_js

def test_simple_path():
    """A simple triangle-like path."""
    vertices = [
        {"x": 0, "y": 0, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
        {"x": 100, "y": 0, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
        {"x": 50, "y": 80, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
    ]
    code = gen_bezier_path_js("path1", vertices, "0xff0000", 1.0, closed=True)
    assert "moveTo(0, 0)" in code
    assert "bezierCurveTo" in code or "lineTo" in code
    assert ".fill(" in code

def test_curved_path():
    """Path with non-zero tangents should produce bezierCurveTo."""
    vertices = [
        {"x": 0, "y": 50, "t1x": -20, "t1y": 0, "t2x": 20, "t2y": 0},
        {"x": 100, "y": 50, "t1x": -20, "t1y": 0, "t2x": 20, "t2y": 0},
    ]
    code = gen_bezier_path_js("curve1", vertices, "0x0000ff", 1.0, closed=False)
    assert "bezierCurveTo" in code

def test_outline_stroke():
    from converters.path import gen_bezier_stroke_js
    vertices = [
        {"x": 0, "y": 0, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
        {"x": 100, "y": 100, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
    ]
    code = gen_bezier_stroke_js("stroke1", vertices, "0x000000", 2.0, 1.0)
    assert ".stroke(" in code
```

**Step 2: 實作**

```python
"""
path.py — Convert Synfig BLine (Bezier spline) to PixiJS Graphics bezier commands
"""

def _has_tangent(v):
    return (abs(v.get("t1x", 0)) > 0.001 or abs(v.get("t1y", 0)) > 0.001 or
            abs(v.get("t2x", 0)) > 0.001 or abs(v.get("t2y", 0)) > 0.001)

def gen_bezier_path_js(name, vertices, fill_hex, alpha, closed=True):
    """
    Generate PixiJS Graphics code for a filled bezier path.
    vertices: list of {"x", "y", "t1x", "t1y", "t2x", "t2y"}
    t1 = incoming tangent, t2 = outgoing tangent (Synfig convention)
    """
    if not vertices:
        return ""

    lines = [f"  const {name} = new Graphics();"]
    v0 = vertices[0]
    lines.append(f"  {name}.moveTo({round(v0['x'], 3)}, {round(v0['y'], 3)});")

    for i in range(1, len(vertices)):
        prev = vertices[i - 1]
        curr = vertices[i]
        # Control point 1: prev position + prev outgoing tangent / 3
        cp1x = round(prev["x"] + prev["t2x"] / 3.0, 3)
        cp1y = round(prev["y"] + prev["t2y"] / 3.0, 3)
        # Control point 2: curr position - curr incoming tangent / 3
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
        lines.append(f"  {name}.bezierCurveTo({cp1x}, {cp1y}, {cp2x}, {cp2y}, {round(curr['x'], 3)}, {round(curr['y'], 3)});")
        lines.append(f"  {name}.closePath();")

    lines.append(f"  {name}.fill({{ color: {fill_hex}, alpha: {alpha} }});")
    lines.append(f"  app.stage.addChild({name});")
    return "\n".join(lines) + "\n"


def gen_bezier_stroke_js(name, vertices, stroke_hex, width, alpha, closed=False):
    """Generate PixiJS Graphics code for a stroked bezier path (outline layer)."""
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
        lines.append(f"  {name}.bezierCurveTo({cp1x}, {cp1y}, {cp2x}, {cp2y}, {round(curr['x'], 3)}, {round(curr['y'], 3)});")

    if closed and len(vertices) > 2:
        lines.append(f"  {name}.closePath();")

    lines.append(f"  {name}.stroke({{ color: {stroke_hex}, width: {width}, alpha: {alpha} }});")
    lines.append(f"  app.stage.addChild({name});")
    return "\n".join(lines) + "\n"
```

**Step 3: 執行測試**

```bash
pytest tests/test_path.py -v
```

Expected: 3 tests PASS

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/converters/path.py
git add synfig-studio/plugins/pixijs-exporter/tests/test_path.py
git commit -m "feat: add Bezier path converter for PixiJS Graphics"
```

---

### Task 7: 實作漸層填充轉換

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/gradient.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_gradient.py`

**Step 1: 寫測試**

```python
from converters.gradient import gen_linear_gradient_js, gen_radial_gradient_js

def test_linear_gradient():
    stops = [(0.0, "0xff0000", 1.0), (1.0, "0x0000ff", 1.0)]
    code = gen_linear_gradient_js("lg1", 0, 0, 200, 0, stops, 480, 270)
    assert "FillGradient" in code
    assert "addColorStop" in code

def test_radial_gradient():
    stops = [(0.0, "0xffffff", 1.0), (1.0, "0x000000", 1.0)]
    code = gen_radial_gradient_js("rg1", 240, 135, 100, stops)
    assert "FillGradient" in code or "radial" in code.lower()
```

**Step 2: 實作**

```python
"""
gradient.py — Generate PixiJS gradient fills
PixiJS v8 uses FillGradient for linear gradients.
Radial gradients require a workaround (render to texture or SVG filter).
"""

def gen_linear_gradient_js(name, x0, y0, x1, y1, color_stops, canvas_w, canvas_h):
    """
    Generate PixiJS linear gradient fill.
    color_stops: [(position, hex_color, alpha), ...]
    """
    lines = [f"  const {name} = new Graphics();"]
    lines.append(f"  {name}.rect(0, 0, {canvas_w}, {canvas_h});")

    # PixiJS v8 FillGradient
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
    """
    Generate PixiJS radial gradient (approximation using multiple concentric circles).
    PixiJS v8 FillGradient supports 'radial' type.
    """
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
    """Convert '0xff0000' to '#ff0000' for CSS."""
    return "#" + hex_str[2:]
```

**Step 3: 執行測試**

```bash
pytest tests/test_gradient.py -v
```

Expected: 2 tests PASS

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/converters/gradient.py
git add synfig-studio/plugins/pixijs-exporter/tests/test_gradient.py
git commit -m "feat: add linear and radial gradient converter for PixiJS"
```

---

### Task 8: 實作圖層驅動器（整合所有轉換器）

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/layers/__init__.py`
- Create: `synfig-studio/plugins/pixijs-exporter/layers/driver.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_driver.py`

**Step 1: 寫測試**

```python
from lxml import etree
from layers.driver import gen_pixi_layers

def test_circle_layer():
    """A minimal .sif circle layer XML should produce valid PixiJS JS."""
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="circle" active="true" version="0.2" desc="Circle">
        <param name="z_depth"><real value="0"/></param>
        <param name="amount"><real value="1"/></param>
        <param name="blend_method"><integer value="0"/></param>
        <param name="color"><color r="1" g="0" b="0" a="1"/></param>
        <param name="radius"><real value="0.5"/></param>
        <param name="origin"><vector><x>0</x><y>0</y></vector></param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code = gen_pixi_layers(root, 480, 270, 60.0)
    assert "Graphics" in code
    assert "circle" in code

def test_empty_canvas():
    xml = '<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s"></canvas>'
    root = etree.fromstring(xml)
    code = gen_pixi_layers(root, 480, 270, 60.0)
    assert code.strip() == "" or "// no layers" in code.lower()
```

**Step 2: 實作**

```python
"""
driver.py — Main layer dispatch: reads Synfig XML layers and generates PixiJS JS code.
"""
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from converters.color import synfig_color_to_hex, parse_synfig_color
from converters.transform import synfig_to_pixi_coords
from converters.shapes import gen_circle_js, gen_rectangle_js, gen_star_js, gen_solid_rect_js
from converters.path import gen_bezier_path_js
from converters.gradient import gen_linear_gradient_js, gen_radial_gradient_js

SHAPE_SOLID = {"region", "polygon", "advanced_outline", "outline", "circle",
               "rectangle", "filled_rectangle", "star"}
SOLID = {"solid_color", "SolidColor"}
GRADIENT = {"linear_gradient", "radial_gradient"}
GROUP = {"group", "switch"}

_counter = 0

def _next_name(prefix):
    global _counter
    _counter += 1
    return f"{prefix}_{_counter}"

def _get_param(layer_el, name):
    """Get a param element by name from a Synfig layer XML element."""
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
    """
    Walk the Synfig canvas XML and generate PixiJS JavaScript for each layer.
    Returns a string of JS code to be inserted into the HTML template.
    """
    global _counter
    _counter = 0

    js_parts = []
    layers = [el for el in root if el.tag == "layer"]

    # Synfig renders bottom layer first (last in XML = bottom)
    for layer_el in reversed(layers):
        layer_type = layer_el.attrib.get("type", "")
        active = layer_el.attrib.get("active", "true") == "true"
        if not active:
            continue

        amount = _parse_real(layer_el, "amount", 1.0)
        if amount <= 0:
            continue

        if layer_type == "circle" or layer_type == "simple_circle":
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            radius = _parse_real(layer_el, "radius", 1.0) * ppu
            fill_hex, alpha = _parse_color(layer_el)
            js_parts.append(gen_circle_js(_next_name("circle"), cx, cy, radius, fill_hex, alpha * amount))

        elif layer_type in {"rectangle", "filled_rectangle"}:
            # Synfig rectangles defined by point1 and point2
            cx, cy = _parse_origin(layer_el, canvas_w, canvas_h, ppu)
            fill_hex, alpha = _parse_color(layer_el)
            # Simplified: use origin as center, default size
            w = _parse_real(layer_el, "expand", 1.0) * ppu * 2
            h = w  # approximate
            js_parts.append(gen_rectangle_js(_next_name("rect"), cx - w/2, cy - h/2, w, h, fill_hex, alpha * amount))

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
            # Wrap children in a Container
            name = _next_name("group")
            js_parts.append(f"  const {name} = new Container();\n")
            js_parts.append(f"  app.stage.addChild({name});\n")
            # Recursively process child layers (in <canvas> child element)
            for child in layer_el:
                if child.tag == "canvas":
                    child_code = gen_pixi_layers(child, canvas_w, canvas_h, ppu)
                    # Replace app.stage.addChild with group.addChild
                    child_code = child_code.replace("app.stage.addChild", f"{name}.addChild")
                    js_parts.append(child_code)

        # More layer types will be added in Phase 2 & 3

    if not js_parts:
        return "  // No supported layers found\n"

    return "".join(js_parts)
```

**Step 3: 執行測試**

```bash
pytest tests/test_driver.py -v
```

Expected: 2 tests PASS

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/layers/
git add synfig-studio/plugins/pixijs-exporter/tests/test_driver.py
git commit -m "feat: add layer driver dispatching Synfig layers to PixiJS code"
```

---

### Task 9: 整合主程式並端對端測試

**Files:**
- Modify: `synfig-studio/plugins/pixijs-exporter/pixijs-exporter.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_e2e.py`

**Step 1: 寫端對端測試**

```python
import os
import subprocess
import tempfile

def test_gradient_sif_to_html(fixtures_dir, exporter_dir):
    """Export gradient.sif to HTML and verify output structure."""
    sif_path = os.path.join(fixtures_dir, "gradient.sif")
    if not os.path.isfile(sif_path):
        import pytest
        pytest.skip("gradient.sif fixture not found")

    with tempfile.NamedTemporaryFile(suffix=".html", delete=False) as f:
        out_path = f.name

    try:
        script = os.path.join(exporter_dir, "pixijs-exporter.py")
        result = subprocess.run(
            ["python3", script, sif_path, out_path],
            capture_output=True, text=True, timeout=30
        )
        assert result.returncode == 0, f"Export failed: {result.stderr}"
        assert os.path.isfile(out_path)

        with open(out_path, 'r') as f:
            html = f.read()

        assert "<!DOCTYPE html>" in html
        assert "pixi" in html.lower()
        assert "Application" in html
        assert "Graphics" in html
    finally:
        if os.path.exists(out_path):
            os.unlink(out_path)

def test_basic_shapes_sif_to_html(fixtures_dir, exporter_dir):
    """Export basic_shapes.sif to HTML."""
    sif_path = os.path.join(fixtures_dir, "basic_shapes.sif")
    if not os.path.isfile(sif_path):
        import pytest
        pytest.skip("basic_shapes.sif fixture not found")

    with tempfile.NamedTemporaryFile(suffix=".html", delete=False) as f:
        out_path = f.name

    try:
        script = os.path.join(exporter_dir, "pixijs-exporter.py")
        result = subprocess.run(
            ["python3", script, sif_path, out_path],
            capture_output=True, text=True, timeout=30
        )
        assert result.returncode == 0, f"Export failed: {result.stderr}"

        with open(out_path, 'r') as f:
            html = f.read()

        assert "<!DOCTYPE html>" in html
        assert len(html) > 500  # Not just an empty template
    finally:
        if os.path.exists(out_path):
            os.unlink(out_path)
```

**Step 2: 更新主程式，接入 layer driver**

在 `pixijs-exporter.py` 的 `main()` 中，將 `pixi_js_code = "  // TODO"` 替換為：

```python
    from layers.driver import gen_pixi_layers
    from converters.transform import calc_pixels_per_unit

    vb = [float(x) for x in root.attrib["view-box"].split()]
    ppu = calc_pixels_per_unit(canvas_meta['width'], canvas_meta['height'], vb)
    pixi_js_code = gen_pixi_layers(root, canvas_meta['width'], canvas_meta['height'], ppu)
```

同時更新 `gen_html()` 的 import 行，加入 `FillGradient, Container`：

```python
import {{ Application, Graphics, Container, FillGradient }} from '{pixi_settings.PIXI_CDN}';
```

**Step 3: 執行測試**

```bash
pytest tests/ -v
```

Expected: 所有測試 PASS（e2e 測試若缺 fixture 則 skip）

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/
git commit -m "feat: integrate layer driver into PixiJS exporter, add e2e tests"
```

---

## Phase 2: 補間動畫 (FRON-5329)

### Task 10: 實作 Tween 引擎（輕量自寫版）

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/runtime/tween.js`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_tween_js.py`

**Step 1: 寫 tween.js — 嵌入 HTML 的輕量補間引擎**

```javascript
/**
 * SynfigTween — Lightweight tween engine for PixiJS
 * Supports: linear, ease-in, ease-out, ease-in-out, cubic-bezier, constant
 */
class SynfigTween {
  constructor(target) {
    this.target = target;
    this.keyframes = [];
    this.duration = 0;
    this.loop = false;
    this.playing = false;
    this._startTime = 0;
  }

  addKeyframe(time, props, easing = 'linear') {
    this.keyframes.push({ time, props, easing });
    this.keyframes.sort((a, b) => a.time - b.time);
    this.duration = Math.max(this.duration, time);
    return this;
  }

  play(ticker) {
    this.playing = true;
    this._startTime = performance.now();
    ticker.add(() => this._update());
    return this;
  }

  _update() {
    if (!this.playing) return;
    let elapsed = (performance.now() - this._startTime) / 1000;
    if (this.loop && this.duration > 0) {
      elapsed = elapsed % this.duration;
    } else if (elapsed > this.duration) {
      elapsed = this.duration;
      this.playing = false;
    }
    this._applyAt(elapsed);
  }

  _applyAt(t) {
    if (this.keyframes.length === 0) return;
    // Find surrounding keyframes
    let prev = this.keyframes[0];
    let next = this.keyframes[this.keyframes.length - 1];
    for (let i = 0; i < this.keyframes.length - 1; i++) {
      if (t >= this.keyframes[i].time && t <= this.keyframes[i + 1].time) {
        prev = this.keyframes[i];
        next = this.keyframes[i + 1];
        break;
      }
    }
    const segDuration = next.time - prev.time;
    const progress = segDuration > 0 ? (t - prev.time) / segDuration : 1;
    const easedProgress = SynfigTween.ease(progress, next.easing);

    for (const key of Object.keys(next.props)) {
      const from = prev.props[key] !== undefined ? prev.props[key] : this.target[key];
      const to = next.props[key];
      if (typeof from === 'number' && typeof to === 'number') {
        this.target[key] = from + (to - from) * easedProgress;
      }
    }
  }

  static ease(t, type) {
    switch (type) {
      case 'constant': return 0;
      case 'linear': return t;
      case 'ease-in': return t * t;
      case 'ease-out': return t * (2 - t);
      case 'ease-in-out': return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
      default:
        if (Array.isArray(type) && type.length === 4) {
          return SynfigTween.cubicBezier(type[0], type[1], type[2], type[3], t);
        }
        return t;
    }
  }

  static cubicBezier(x1, y1, x2, y2, t) {
    // Newton-Raphson iteration for cubic bezier
    let x = t;
    for (let i = 0; i < 8; i++) {
      const cx = 3 * x1, bx = 3 * (x2 - x1) - cx, ax = 1 - cx - bx;
      const curveX = ((ax * x + bx) * x + cx) * x;
      const curveXDeriv = (3 * ax * x + 2 * bx) * x + cx;
      if (Math.abs(curveX - t) < 1e-6) break;
      if (Math.abs(curveXDeriv) < 1e-6) break;
      x -= (curveX - t) / curveXDeriv;
    }
    const cy = 3 * y1, by = 3 * (y2 - y1) - cy, ay = 1 - cy - by;
    return ((ay * x + by) * x + cy) * x;
  }
}
```

**Step 2: 寫驗證測試（確認 JS 嵌入正確）**

```python
import os

def test_tween_js_exists(exporter_dir):
    path = os.path.join(exporter_dir, "runtime", "tween.js")
    assert os.path.isfile(path)

def test_tween_js_contains_class(exporter_dir):
    path = os.path.join(exporter_dir, "runtime", "tween.js")
    with open(path, 'r') as f:
        content = f.read()
    assert "class SynfigTween" in content
    assert "cubicBezier" in content
    assert "addKeyframe" in content
```

**Step 3: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/runtime/
git add synfig-studio/plugins/pixijs-exporter/tests/test_tween_js.py
git commit -m "feat: add lightweight SynfigTween engine for PixiJS animation"
```

---

### Task 11: 實作 Waypoint → Tween 轉換

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/animation.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_animation.py`

**Step 1: 寫測試**

```python
from converters.animation import waypoints_to_tween_js, interpolation_to_easing

def test_linear_interpolation():
    assert interpolation_to_easing("linear") == "'linear'"

def test_constant_interpolation():
    assert interpolation_to_easing("constant") == "'constant'"

def test_tcb_to_bezier():
    result = interpolation_to_easing("TCB", tension=0, continuity=0, bias=0)
    # Default TCB (0,0,0) should produce a smooth ease
    assert result.startswith("[")  # bezier array

def test_simple_waypoints():
    waypoints = [
        {"time": 0.0, "value": 0, "before": "constant", "after": "linear"},
        {"time": 1.0, "value": 100, "before": "linear", "after": "linear"},
    ]
    code = waypoints_to_tween_js("obj1", "x", waypoints, fps=24)
    assert "addKeyframe" in code
    assert "0" in code  # time 0
    assert "100" in code  # value

def test_halt_easing():
    assert interpolation_to_easing("halt") == "'ease-out'"
```

**Step 2: 實作**

```python
"""
animation.py — Convert Synfig Waypoints to SynfigTween keyframe calls
"""
import math

def interpolation_to_easing(interp_type, tension=0, continuity=0, bias=0):
    """Convert Synfig interpolation type to SynfigTween easing parameter."""
    if interp_type == "linear":
        return "'linear'"
    elif interp_type == "constant":
        return "'constant'"
    elif interp_type == "halt":
        return "'ease-out'"
    elif interp_type in ("TCB", "clamped"):
        # Convert TCB to cubic bezier control points
        # Based on Kochanek-Bartels tangent formula
        c1 = (1.0 - tension) * (1.0 + continuity) * (1.0 + bias) / 2.0
        c2 = (1.0 - tension) * (1.0 - continuity) * (1.0 - bias) / 2.0
        # Map to cubic-bezier approximation
        x1 = round(max(0, min(1, c2 / (c1 + c2 + 0.001))), 4)
        y1 = round(max(0, min(1, c1)), 4)
        x2 = round(max(0, min(1, 1.0 - c1 / (c1 + c2 + 0.001))), 4)
        y2 = round(max(0, min(1, 1.0 - c2)), 4)
        return f"[{x1}, {y1}, {x2}, {y2}]"
    else:
        return "'linear'"

def waypoints_to_tween_js(target_name, prop_name, waypoints, fps=24):
    """
    Generate SynfigTween.addKeyframe() calls from Synfig waypoints.

    waypoints: list of {"time": float(seconds), "value": number,
                        "before": str, "after": str,
                        "tension": float, "continuity": float, "bias": float}
    """
    if not waypoints:
        return ""

    lines = []
    for wp in waypoints:
        time_sec = round(wp["time"], 4)
        value = wp["value"]
        easing = interpolation_to_easing(
            wp.get("after", "linear"),
            wp.get("tension", 0),
            wp.get("continuity", 0),
            wp.get("bias", 0)
        )
        lines.append(
            f"  {target_name}_tween.addKeyframe({time_sec}, "
            f"{{ {prop_name}: {value} }}, {easing});"
        )
    return "\n".join(lines) + "\n"

def gen_tween_setup_js(target_name, loop=True):
    """Generate the SynfigTween instantiation and play call."""
    return (
        f"  const {target_name}_tween = new SynfigTween({target_name});\n"
    )

def gen_tween_play_js(target_name, loop=True):
    """Generate play call (must be after all keyframes are added)."""
    loop_str = "true" if loop else "false"
    return (
        f"  {target_name}_tween.loop = {loop_str};\n"
        f"  {target_name}_tween.play(app.ticker);\n"
    )
```

**Step 3: 執行測試**

```bash
pytest tests/test_animation.py -v
```

Expected: 5 tests PASS

**Step 4: Commit**

```bash
git add synfig-studio/plugins/pixijs-exporter/converters/animation.py
git add synfig-studio/plugins/pixijs-exporter/tests/test_animation.py
git commit -m "feat: add Waypoint-to-Tween converter with TCB→Bezier mapping"
```

---

### Task 12: 整合動畫到 Layer Driver

**Files:**
- Modify: `synfig-studio/plugins/pixijs-exporter/layers/driver.py`
- Modify: `synfig-studio/plugins/pixijs-exporter/pixijs-exporter.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_animated_export.py`

**Step 1: 更新 driver.py 偵測 animated 參數**

在 `gen_pixi_layers()` 中，對每個支援的參數檢查是否有 `<animated>` 子元素。若有，生成 SynfigTween keyframe 代碼。

重點邏輯：
- 解析 `<animated type="...">` 下的 `<waypoint>` 元素
- 提取 time, value, before/after interpolation, TCB 參數
- 呼叫 `waypoints_to_tween_js()` 生成 JS
- 在 HTML 模板中嵌入 `tween.js` runtime

**Step 2: 更新 gen_html() 嵌入 tween.js**

```python
def gen_html(canvas_meta, pixi_js_code):
    # Read tween.js runtime
    tween_path = os.path.join(os.path.dirname(__file__), "runtime", "tween.js")
    with open(tween_path, 'r') as f:
        tween_runtime = f.read()

    # Insert tween_runtime before pixi_js_code in the <script> block
```

**Step 3: 寫測試驗證動畫匯出**

```python
def test_animated_export(fixtures_dir, exporter_dir):
    """Export animation_keyframes.sif and verify tween code is present."""
    sif_path = os.path.join(fixtures_dir, "animation_keyframes.sif")
    if not os.path.isfile(sif_path):
        import pytest
        pytest.skip("animation_keyframes.sif not found")
    # ... (similar to e2e test pattern)
    assert "SynfigTween" in html
    assert "addKeyframe" in html
```

**Step 4: Commit**

```bash
git commit -m "feat: integrate animation tween engine into PixiJS exporter"
```

---

## Phase 3: 進階功能 (FRON-5330)

### Task 13: 實作混合模式映射

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/blend.py`
- Create: `synfig-studio/plugins/pixijs-exporter/tests/test_blend.py`

**Step 1: 實作映射表**

```python
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
    return BLEND_MAP.get(blend_int, "normal")
```

**Step 2: 測試、Commit**

---

### Task 14: 實作 Blur 濾鏡

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/filters.py`

**實作：**

```python
def gen_blur_filter_js(target_name, strength_x, strength_y):
    return (
        f"  {target_name}.filters = [new BlurFilter({{ strengthX: {strength_x}, strengthY: {strength_y} }})];\n"
    )
```

HTML import 加入 `BlurFilter`。

---

### Task 15: 實作 Text 圖層

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/text.py`

**實作：**

```python
def gen_text_js(name, text, x, y, font_family, font_size, fill_hex, alpha):
    return f"""  const {name} = new Text({{
    text: {repr(text)},
    style: {{ fontFamily: '{font_family}', fontSize: {font_size}, fill: '{_hex_to_css(fill_hex)}' }},
  }});
  {name}.position.set({x}, {y});
  {name}.alpha = {alpha};
  app.stage.addChild({name});
"""
```

HTML import 加入 `Text`。

---

### Task 16: 實作 Image 嵌入

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/image.py`

**實作：**

圖片嵌入使用 base64 inline data URI：

```python
import base64

def gen_image_js(name, image_path, x, y, w, h):
    with open(image_path, 'rb') as f:
        data = base64.b64encode(f.read()).decode()
    ext = image_path.rsplit('.', 1)[-1].lower()
    mime = {'png': 'image/png', 'jpg': 'image/jpeg', 'jpeg': 'image/jpeg'}.get(ext, 'image/png')
    return f"""  const {name}_tex = await Assets.load('data:{mime};base64,{data}');
  const {name} = new Sprite({name}_tex);
  {name}.position.set({x}, {y});
  {name}.width = {w};
  {name}.height = {h};
  app.stage.addChild({name});
"""
```

HTML import 加入 `Sprite, Assets`。

---

### Task 17: 互動事件模板

**Files:**
- Create: `synfig-studio/plugins/pixijs-exporter/converters/interactive.py`

**實作：**

為匯出的物件加入可選的互動事件：

```python
def gen_interactive_js(target_name, event_type="pointerdown", action="toggle"):
    lines = [f"  {target_name}.eventMode = 'static';"]
    lines.append(f"  {target_name}.cursor = 'pointer';")
    if action == "toggle":
        lines.append(f"  {target_name}.on('{event_type}', () => {{")
        lines.append(f"    {target_name}.visible = !{target_name}.visible;")
        lines.append(f"  }});")
    elif action == "play":
        lines.append(f"  {target_name}.on('{event_type}', () => {{")
        lines.append(f"    if ({target_name}_tween) {target_name}_tween.play(app.ticker);")
        lines.append(f"  }});")
    return "\n".join(lines) + "\n"
```

---

### Task 18: 最終整合與驗證

**Step 1: 執行所有測試**

```bash
cd synfig-studio/plugins/pixijs-exporter
pytest tests/ -v
```

**Step 2: 手動驗證**

用測試 fixture 匯出 HTML，在瀏覽器中確認：
- gradient.sif → 漸層正確
- basic_shapes.sif → 形狀正確
- animation_keyframes.sif → 動畫播放正確

**Step 3: Commit & Push**

```bash
git add -A
git commit -m "feat: complete PixiJS exporter with animation, filters, text, and interactivity"
git push fork feat/pixijs-exporter
```
