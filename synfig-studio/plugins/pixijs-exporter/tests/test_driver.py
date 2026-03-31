from lxml import etree
from layers.driver import gen_pixi_layers

def test_circle_layer():
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="circle" active="true" version="0.2" desc="Circle">
        <param name="z_depth"><real value="0"/></param>
        <param name="amount"><real value="1"/></param>
        <param name="blend_method"><integer value="0"/></param>
        <param name="color"><color r="1" g="0" b="0" a="1"/></param>
        <param name="radius"><real value="0.5"/></param>
        <param name="origin"><vector><x>0</x><y>0</y></vector></param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0)
    assert "Graphics" in code
    assert "circle" in code
    assert has_anim is False

def test_solid_color_layer():
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="solid_color" active="true" version="0.2" desc="Solid">
        <param name="color"><color r="0" g="0.5" b="1" a="1"/></param>
        <param name="amount"><real value="1"/></param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0)
    assert "rect(0, 0, 480, 270)" in code
    assert has_anim is False

def test_empty_canvas():
    xml = '<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s"></canvas>'
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0)
    assert "no supported layers" in code.lower() or code.strip() == ""
    assert has_anim is False

def test_inactive_layer_skipped():
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="circle" active="false" version="0.2">
        <param name="color"><color r="1" g="0" b="0" a="1"/></param>
        <param name="radius"><real value="0.5"/></param>
        <param name="origin"><vector><x>0</x><y>0</y></vector></param>
        <param name="amount"><real value="1"/></param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0)
    assert "circle" not in code.lower() or "no supported" in code.lower()
    assert has_anim is False
