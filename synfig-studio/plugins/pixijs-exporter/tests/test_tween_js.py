import os
import sys

# Ensure exporter dir is in path for conftest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

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
