import math
from converters.transform import synfig_to_pixi_coords, synfig_to_pixi_angle, calc_pixels_per_unit

def test_center_origin():
    x, y = synfig_to_pixi_coords(0.0, 0.0, 480, 270, 60.0)
    assert x == 240.0
    assert y == 135.0

def test_top_left():
    x, y = synfig_to_pixi_coords(-4.0, 2.25, 480, 270, 60.0)
    assert x == 0.0
    assert y == 0.0

def test_bottom_right():
    x, y = synfig_to_pixi_coords(4.0, -2.25, 480, 270, 60.0)
    assert x == 480.0
    assert y == 270.0

def test_angle_conversion():
    assert synfig_to_pixi_angle(0.0) == 0.0
    assert abs(synfig_to_pixi_angle(90.0) - (-math.pi / 2)) < 1e-6

def test_ppu_calculation():
    ppu = calc_pixels_per_unit(480, 270, [-4.0, 2.25, 4.0, -2.25])
    assert ppu == 60.0
