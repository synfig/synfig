from converters.gradient import gen_linear_gradient_js, gen_radial_gradient_js

def test_linear_gradient():
    stops = [(0.0, "0xff0000", 1.0), (1.0, "0x0000ff", 1.0)]
    code = gen_linear_gradient_js("lg1", 0, 0, 200, 0, stops, 480, 270)
    assert "FillGradient" in code
    assert "addColorStop" in code or "colorStops" in code

def test_radial_gradient():
    stops = [(0.0, "0xffffff", 1.0), (1.0, "0x000000", 1.0)]
    code = gen_radial_gradient_js("rg1", 240, 135, 100, stops)
    assert "FillGradient" in code or "radial" in code.lower()

def test_empty_stops():
    code = gen_linear_gradient_js("empty", 0, 0, 100, 0, [], 480, 270)
    assert "FillGradient" in code  # should still produce valid code
