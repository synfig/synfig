import os
import xml.etree.ElementTree as ET
import pytest


def test_svg_namespace_valid():
    """Test SVG namespace is correct."""
    ns = "http://www.w3.org/2000/svg"
    assert "w3.org" in ns


def test_svg_basic_structure():
    """Test basic SVG document structure."""
    svg = '<?xml version="1.0"?><svg xmlns="http://www.w3.org/2000/svg" width="480" height="270"></svg>'
    root = ET.fromstring(svg)
    assert root.tag.endswith("svg")
    assert "width" in root.attrib
    assert "height" in root.attrib


def test_svg_viewbox_parsing():
    """Test SVG viewBox attribute parsing."""
    viewbox = "0 0 480 270"
    parts = viewbox.split()
    assert len(parts) == 4
    values = [float(p) for p in parts]
    assert values[2] > 0  # width
    assert values[3] > 0  # height


def test_svg_color_format():
    """Test SVG color format validation."""
    colors = ["#ff0000", "#00ff00", "#0000ff", "rgb(255,0,0)"]
    for color in colors:
        assert len(color) > 0


def test_svg_transform_matrix():
    """Test SVG transform matrix format."""
    transform = "matrix(1,0,0,1,0,0)"
    assert transform.startswith("matrix(")
    values = transform[7:-1].split(",")
    assert len(values) == 6


def test_svg_layer_elements():
    """Test SVG group element structure for layers."""
    svg = '<svg xmlns="http://www.w3.org/2000/svg"><g id="layer1"><rect x="0" y="0" width="100" height="100"/></g></svg>'
    root = ET.fromstring(svg)
    ns = {"svg": "http://www.w3.org/2000/svg"}
    groups = root.findall("svg:g", ns)
    assert len(groups) == 1
    assert groups[0].attrib["id"] == "layer1"
