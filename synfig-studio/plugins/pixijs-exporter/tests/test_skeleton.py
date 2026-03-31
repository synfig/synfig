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
