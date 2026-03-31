from converters.shapes import gen_circle_js, gen_rectangle_js, gen_star_js, gen_polygon_js, gen_solid_rect_js

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

def test_polygon_js():
    pts = [(0, 0), (100, 0), (50, 80)]
    code = gen_polygon_js("poly1", pts, "0x00ffff", 1.0)
    assert ".poly(" in code

def test_solid_rect_js():
    code = gen_solid_rect_js("bg", 480, 270, "0x333333", 0.8)
    assert ".rect(0, 0, 480, 270)" in code

def test_circle_with_alpha():
    code = gen_circle_js("c1", 0, 0, 10, "0xffffff", 0.5)
    assert "0.5" in code
