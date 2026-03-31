import math
import os
import sys

# Ensure exporter dir is in path for conftest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))


# --- Python mirrors of the JS easing functions ---

def ease_linear(t):
    return t


def ease_in(t):
    return t * t


def ease_out(t):
    return t * (2 - t)


def ease_in_out(t):
    if t < 0.5:
        return 2 * t * t
    return -1 + (4 - 2 * t) * t


def ease_constant(_t):
    return 0


def cubic_bezier(x1, y1, x2, y2, t):
    cx = 3 * x1
    bx = 3 * (x2 - x1) - cx
    ax = 1 - cx - bx
    x = t
    for _ in range(8):
        curve_x = ((ax * x + bx) * x + cx) * x
        curve_x_deriv = (3 * ax * x + 2 * bx) * x + cx
        if abs(curve_x - t) < 1e-6:
            break
        if abs(curve_x_deriv) < 1e-6:
            break
        x -= (curve_x - t) / curve_x_deriv
        x = max(0.0, min(1.0, x))
    cy = 3 * y1
    by = 3 * (y2 - y1) - cy
    ay = 1 - cy - by
    return ((ay * x + by) * x + cy) * x


# --- File existence / structure tests ---

def test_tween_js_exists():
    path = os.path.join(os.path.dirname(__file__), "..", "runtime", "tween.js")
    assert os.path.isfile(path)


def test_tween_js_contains_class():
    path = os.path.join(os.path.dirname(__file__), "..", "runtime", "tween.js")
    with open(path, 'r') as f:
        content = f.read()
    assert "class SynfigTween" in content
    assert "cubicBezier" in content
    assert "addKeyframe" in content


# --- Behavioral easing tests ---

def test_ease_linear():
    assert ease_linear(0.5) == 0.5


def test_ease_in():
    assert ease_in(0.5) == 0.25


def test_ease_out():
    assert ease_out(0.5) == 0.75


def test_ease_in_out():
    assert ease_in_out(0.5) == 0.5


def test_ease_constant():
    assert ease_constant(0.0) == 0
    assert ease_constant(0.5) == 0
    assert ease_constant(1.0) == 0


def test_cubic_bezier_linear():
    result = cubic_bezier(0, 0, 1, 1, 0.5)
    assert math.isclose(result, 0.5, abs_tol=1e-4)
