from converters.color import synfig_color_to_hex, synfig_color_to_rgba

def test_red_to_hex():
    assert synfig_color_to_hex(1.0, 0.0, 0.0) == "0xff0000"

def test_white_to_hex():
    assert synfig_color_to_hex(1.0, 1.0, 1.0) == "0xffffff"

def test_black_to_hex():
    assert synfig_color_to_hex(0.0, 0.0, 0.0) == "0x000000"

def test_color_with_gamma():
    result = synfig_color_to_hex(0.5, 0.5, 0.5)
    # After gamma correction (2.2), 0.5^(1/2.2) ≈ 0.7297 → 0xba (186)
    assert result == "0xbababa"

def test_rgba_string():
    result = synfig_color_to_rgba(1.0, 0.0, 0.0, 0.5)
    assert result == "rgba(255, 0, 0, 0.5)"

def test_clamp_overflow():
    assert synfig_color_to_hex(1.5, -0.1, 0.0) == "0xff0000"
