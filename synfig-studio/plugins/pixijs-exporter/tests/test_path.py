from converters.path import gen_bezier_path_js, gen_bezier_stroke_js

def test_simple_path():
    vertices = [
        {"x": 0, "y": 0, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
        {"x": 100, "y": 0, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
        {"x": 50, "y": 80, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
    ]
    code = gen_bezier_path_js("path1", vertices, "0xff0000", 1.0, closed=True)
    assert "moveTo(0, 0)" in code
    assert "bezierCurveTo" in code
    assert ".fill(" in code

def test_curved_path():
    vertices = [
        {"x": 0, "y": 50, "t1x": -20, "t1y": 0, "t2x": 20, "t2y": 0},
        {"x": 100, "y": 50, "t1x": -20, "t1y": 0, "t2x": 20, "t2y": 0},
    ]
    code = gen_bezier_path_js("curve1", vertices, "0x0000ff", 1.0, closed=False)
    assert "bezierCurveTo" in code

def test_outline_stroke():
    vertices = [
        {"x": 0, "y": 0, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
        {"x": 100, "y": 100, "t1x": 0, "t1y": 0, "t2x": 0, "t2y": 0},
    ]
    code = gen_bezier_stroke_js("stroke1", vertices, "0x000000", 2.0, 1.0)
    assert ".stroke(" in code

def test_empty_vertices():
    assert gen_bezier_path_js("empty", [], "0x000000", 1.0) == ""
