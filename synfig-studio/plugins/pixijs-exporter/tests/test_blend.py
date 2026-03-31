"""Tests for blend mode mapping (Synfig → PixiJS)."""
import pytest
from converters.blend import BLEND_MAP, synfig_blend_to_pixi


# ── Known mappings ──────────────────────────────────────────────────────────

@pytest.mark.parametrize(
    "synfig_int, expected",
    [
        (0, "normal"),        # BLEND_COMPOSITE
        (1, "normal"),        # BLEND_STRAIGHT
        (2, "lighten"),       # BLEND_BRIGHTEN
        (3, "darken"),        # BLEND_DARKEN
        (6, "multiply"),      # BLEND_MULTIPLY
        (8, "color"),         # BLEND_COLOR
        (9, "hue"),           # BLEND_HUE
        (10, "saturation"),   # BLEND_SATURATION
        (11, "luminosity"),   # BLEND_LUMINANCE
        (12, "add"),          # BLEND_ADD
        (16, "screen"),       # BLEND_SCREEN
        (17, "hard-light"),   # BLEND_HARD_LIGHT
        (18, "difference"),   # BLEND_DIFFERENCE
        (20, "overlay"),      # BLEND_OVERLAY
    ],
)
def test_known_blend_mappings(synfig_int, expected):
    """Every documented Synfig blend integer maps to the correct PixiJS string."""
    assert synfig_blend_to_pixi(synfig_int) == expected


# ── Unknown / fallback ──────────────────────────────────────────────────────

@pytest.mark.parametrize("unknown_int", [99, -1, 4, 5, 7, 13, 14, 15, 19, 100, 999])
def test_unknown_blend_returns_normal(unknown_int):
    """Unknown blend mode integers fall back to 'normal'."""
    assert synfig_blend_to_pixi(unknown_int) == "normal"


# ── Edge cases ──────────────────────────────────────────────────────────────

def test_zero_returns_normal():
    """Blend mode 0 (BLEND_COMPOSITE) explicitly returns 'normal'."""
    assert synfig_blend_to_pixi(0) == "normal"


def test_blend_map_is_not_empty():
    """BLEND_MAP must contain entries."""
    assert len(BLEND_MAP) > 0


def test_blend_map_all_values_are_strings():
    """Every value in BLEND_MAP must be a string."""
    for key, value in BLEND_MAP.items():
        assert isinstance(value, str), f"BLEND_MAP[{key}] is {type(value)}, expected str"


def test_blend_map_all_keys_are_ints():
    """Every key in BLEND_MAP must be an integer."""
    for key in BLEND_MAP:
        assert isinstance(key, int), f"Key {key!r} is {type(key)}, expected int"
