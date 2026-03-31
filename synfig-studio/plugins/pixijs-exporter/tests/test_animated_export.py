import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from lxml import etree
from layers.driver import gen_pixi_layers

def test_static_circle_no_tween():
    """A static circle should produce no tween code."""
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="circle" active="true" version="0.2" desc="Circle">
        <param name="z_depth"><real value="0"/></param>
        <param name="amount"><real value="1"/></param>
        <param name="color"><color r="1" g="0" b="0" a="1"/></param>
        <param name="radius"><real value="0.5"/></param>
        <param name="origin"><vector><x>0</x><y>0</y></vector></param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0, fps=24)
    assert "SynfigTween" not in code
    assert has_anim is False

def test_animated_origin_produces_tween():
    """A circle with animated origin should produce tween code."""
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="circle" active="true" version="0.2" desc="Circle">
        <param name="amount"><real value="1"/></param>
        <param name="color"><color r="1" g="0" b="0" a="1"/></param>
        <param name="radius"><real value="0.5"/></param>
        <param name="origin">
          <animated type="vector">
            <waypoint time="0s" before="linear" after="linear">
              <vector><x>-2</x><y>0</y></vector>
            </waypoint>
            <waypoint time="2s" before="linear" after="linear">
              <vector><x>2</x><y>0</y></vector>
            </waypoint>
          </animated>
        </param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0, fps=24)
    assert "SynfigTween" in code
    assert "addKeyframe" in code
    assert has_anim is True

def test_animated_origin_values_correct():
    """Verify the tween keyframe values are correct for animated origin."""
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="circle" active="true" version="0.2" desc="Circle">
        <param name="amount"><real value="1"/></param>
        <param name="color"><color r="1" g="0" b="0" a="1"/></param>
        <param name="radius"><real value="0.5"/></param>
        <param name="origin">
          <animated type="vector">
            <waypoint time="0s" before="linear" after="linear">
              <vector><x>0</x><y>0</y></vector>
            </waypoint>
            <waypoint time="1s" before="linear" after="linear">
              <vector><x>2</x><y>-1</y></vector>
            </waypoint>
          </animated>
        </param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0, fps=24)
    # At time 0: origin (0,0) -> pixi (240, 135)
    assert "addKeyframe(0.0" in code
    assert "240.0" in code  # x at time 0 (origin 0,0)
    assert "135.0" in code  # y at time 0
    # At time 1: origin (2,-1) -> pixi (240+120, 135+60) = (360, 195)
    assert "addKeyframe(1.0" in code
    assert "360.0" in code  # x at time 1 (origin 2,-1)
    assert "195.0" in code  # y at time 1
    assert has_anim is True

def test_gen_pixi_layers_backward_compat():
    """gen_pixi_layers without fps should still work (default fps=24)."""
    xml = '<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s"></canvas>'
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0)
    assert has_anim is False

def test_animated_rectangle_origin():
    """A rectangle with animated origin should produce tween code."""
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="rectangle" active="true" version="0.2">
        <param name="amount"><real value="1"/></param>
        <param name="color"><color r="0" g="1" b="0" a="1"/></param>
        <param name="expand"><real value="1.0"/></param>
        <param name="origin">
          <animated type="vector">
            <waypoint time="0s" before="linear" after="linear">
              <vector><x>-1</x><y>0</y></vector>
            </waypoint>
            <waypoint time="1s" before="linear" after="linear">
              <vector><x>1</x><y>0</y></vector>
            </waypoint>
          </animated>
        </param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0, fps=24)
    assert "SynfigTween" in code
    assert has_anim is True

def test_animated_star_origin():
    """A star with animated origin should produce tween code."""
    xml = """<canvas version="1.4" width="480" height="270" view-box="-4 2.25 4 -2.25" fps="24" begin-time="0s" end-time="3s">
      <layer type="star" active="true" version="0.2">
        <param name="amount"><real value="1"/></param>
        <param name="color"><color r="1" g="1" b="0" a="1"/></param>
        <param name="radius1"><real value="1.0"/></param>
        <param name="radius2"><real value="0.5"/></param>
        <param name="points"><real value="5"/></param>
        <param name="origin">
          <animated type="vector">
            <waypoint time="0s" before="linear" after="linear">
              <vector><x>0</x><y>1</y></vector>
            </waypoint>
            <waypoint time="2s" before="linear" after="linear">
              <vector><x>0</x><y>-1</y></vector>
            </waypoint>
          </animated>
        </param>
      </layer>
    </canvas>"""
    root = etree.fromstring(xml)
    code, has_anim = gen_pixi_layers(root, 480, 270, 60.0, fps=24)
    assert "SynfigTween" in code
    assert has_anim is True
