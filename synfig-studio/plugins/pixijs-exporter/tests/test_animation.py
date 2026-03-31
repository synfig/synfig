from converters.animation import waypoints_to_tween_js, interpolation_to_easing, gen_tween_setup_js, gen_tween_play_js

def test_linear_interpolation():
    assert interpolation_to_easing("linear") == "'linear'"

def test_constant_interpolation():
    assert interpolation_to_easing("constant") == "'constant'"

def test_tcb_to_bezier():
    result = interpolation_to_easing("TCB", tension=0, continuity=0, bias=0)
    # TCB(0,0,0) => c1=0.5, c2=0.5 => bezier [0.4995, 0.5, 0.5005, 0.5]
    assert result == "[0.4995, 0.5, 0.5005, 0.5]"

def test_simple_waypoints():
    waypoints = [
        {"time": 0.0, "value": 0, "before": "constant", "after": "linear"},
        {"time": 1.0, "value": 100, "before": "linear", "after": "linear"},
    ]
    code = waypoints_to_tween_js("obj1", "x", waypoints)
    assert "addKeyframe(0.0," in code
    assert "addKeyframe(1.0," in code
    assert "{ x: 0 }" in code
    assert "{ x: 100 }" in code

def test_halt_easing():
    assert interpolation_to_easing("halt") == "'ease-out'"

def test_tween_setup():
    code = gen_tween_setup_js("circle1")
    assert "new SynfigTween(circle1)" in code
    assert "const circle1_tween" in code

def test_tween_play():
    code = gen_tween_play_js("circle1", loop=True)
    assert "circle1_tween.loop = true" in code
    assert "circle1_tween.play(app.ticker)" in code

def test_tween_play_no_loop():
    code = gen_tween_play_js("circle1", loop=False)
    assert "circle1_tween.loop = false" in code

def test_clamped_to_bezier():
    result = interpolation_to_easing("clamped", tension=0, continuity=0, bias=0)
    assert result.startswith("[")
    vals = result.strip("[]").split(", ")
    assert len(vals) == 4

def test_unknown_interpolation_defaults_linear():
    assert interpolation_to_easing("unknown_type") == "'linear'"

def test_waypoints_empty():
    assert waypoints_to_tween_js("obj", "x", []) == ""
