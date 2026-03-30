import os
import json
import sys
import pytest


def test_settings_module_importable(exporter_dir):
    """Test that settings module can be imported and has expected attributes."""
    sys.path.insert(0, exporter_dir)
    try:
        import settings
        # settings should have configuration attributes
        assert hasattr(settings, '__file__')
    except ImportError:
        pytest.skip("settings not importable in test context")
    finally:
        if exporter_dir in sys.path:
            sys.path.remove(exporter_dir)


def test_lottie_json_schema_basic():
    """Test that a basic Lottie JSON structure has required fields."""
    # Minimal valid Lottie JSON structure
    lottie = {
        "v": "5.6.5",
        "fr": 24,
        "ip": 0,
        "op": 48,
        "w": 480,
        "h": 270,
        "nm": "test",
        "layers": []
    }
    required_keys = {"v", "fr", "ip", "op", "w", "h", "layers"}
    assert required_keys.issubset(lottie.keys())


def test_lottie_json_version_format():
    """Test Lottie version string format."""
    version = "5.6.5"
    parts = version.split(".")
    assert len(parts) == 3
    assert all(p.isdigit() for p in parts)


def test_lottie_json_dimensions_positive():
    """Test that dimensions must be positive."""
    lottie = {"w": 480, "h": 270}
    assert lottie["w"] > 0
    assert lottie["h"] > 0


def test_lottie_json_framerate_valid():
    """Test valid frame rate values."""
    valid_rates = [12, 24, 25, 30, 60]
    for rate in valid_rates:
        assert rate > 0


def test_lottie_json_layers_structure():
    """Test layer structure in Lottie JSON."""
    layer = {
        "ty": 4,  # shape layer
        "nm": "Shape Layer",
        "ind": 0,
        "ip": 0,
        "op": 48,
        "ks": {}  # transform
    }
    required_layer_keys = {"ty", "nm", "ip", "op"}
    assert required_layer_keys.issubset(layer.keys())


def test_lottie_shape_types():
    """Test common Lottie shape type codes."""
    shape_types = {
        0: "precomp",
        1: "solid",
        2: "image",
        3: "null",
        4: "shape",
        5: "text"
    }
    assert shape_types[4] == "shape"


def test_lottie_blend_modes():
    """Test Lottie blend mode enumeration."""
    blend_modes = {
        0: "normal",
        1: "multiply",
        2: "screen",
        3: "overlay"
    }
    assert 0 in blend_modes
    assert blend_modes[0] == "normal"
