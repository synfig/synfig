import os


def test_lottie_exporter_main_exists(exporter_dir):
    """Verify that the main lottie-exporter.py script exists."""
    assert os.path.isfile(os.path.join(exporter_dir, "lottie-exporter.py"))


def test_canvas_module_exists(exporter_dir):
    """Verify canvas.py module exists."""
    assert os.path.isfile(os.path.join(exporter_dir, "canvas.py"))


def test_settings_module_exists(exporter_dir):
    """Verify settings.py module exists."""
    assert os.path.isfile(os.path.join(exporter_dir, "settings.py"))


def test_properties_module_exists(exporter_dir):
    """Verify properties directory exists with Python files."""
    props_dir = os.path.join(exporter_dir, "properties")
    assert os.path.isdir(props_dir)


def test_layers_module_exists(exporter_dir):
    """Verify layers directory exists."""
    layers_dir = os.path.join(exporter_dir, "layers")
    assert os.path.isdir(layers_dir)


def test_shapes_module_exists(exporter_dir):
    """Verify shapes directory exists."""
    shapes_dir = os.path.join(exporter_dir, "shapes")
    assert os.path.isdir(shapes_dir)


def test_common_module_exists(exporter_dir):
    """Verify common directory exists."""
    common_dir = os.path.join(exporter_dir, "common")
    assert os.path.isdir(common_dir)


def test_effects_module_exists(exporter_dir):
    """Verify effects directory exists."""
    effects_dir = os.path.join(exporter_dir, "effects")
    assert os.path.isdir(effects_dir)
