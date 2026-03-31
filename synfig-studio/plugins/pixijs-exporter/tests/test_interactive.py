"""Tests for interactive event JS code generation."""
import pytest
from converters.interactive import gen_interactive_js


class TestGenInteractiveJsDefaults:
    """Test default parameters (pointerdown, toggle)."""

    def test_contains_event_mode(self):
        result = gen_interactive_js("sprite1")
        assert "eventMode = 'static'" in result

    def test_contains_cursor_pointer(self):
        result = gen_interactive_js("sprite1")
        assert "cursor = 'pointer'" in result

    def test_contains_pointerdown_event(self):
        result = gen_interactive_js("sprite1")
        assert ".on('pointerdown'" in result

    def test_contains_visible_toggle(self):
        result = gen_interactive_js("sprite1")
        assert ".visible = !sprite1.visible" in result


class TestCustomEventType:
    """Test custom event type."""

    def test_pointerup_event(self):
        result = gen_interactive_js("btn", event_type="pointerup")
        assert ".on('pointerup'" in result

    def test_click_event(self):
        result = gen_interactive_js("btn", event_type="click")
        assert ".on('click'" in result


class TestPlayAction:
    """Test action='play' generates tween code."""

    def test_contains_tween_play(self):
        result = gen_interactive_js("box", action="play")
        assert "_tween.play" in result

    def test_contains_app_ticker(self):
        result = gen_interactive_js("box", action="play")
        assert "app.ticker" in result

    def test_no_visible_toggle(self):
        result = gen_interactive_js("box", action="play")
        assert ".visible = !" not in result


class TestToggleAction:
    """Test action='toggle' explicitly."""

    def test_contains_visible_toggle(self):
        result = gen_interactive_js("item", action="toggle")
        assert "item.visible = !item.visible" in result

    def test_no_tween(self):
        result = gen_interactive_js("item", action="toggle")
        assert "_tween" not in result


class TestTargetName:
    """Test target name appears throughout output."""

    def test_target_in_event_mode(self):
        result = gen_interactive_js("mySprite")
        assert "mySprite.eventMode" in result

    def test_target_in_cursor(self):
        result = gen_interactive_js("mySprite")
        assert "mySprite.cursor" in result

    def test_target_in_on_call(self):
        result = gen_interactive_js("mySprite")
        assert "mySprite.on(" in result


class TestUnknownAction:
    """Unknown action produces no event handler but still sets eventMode and cursor."""

    def test_has_event_mode(self):
        result = gen_interactive_js("obj", action="unknown")
        assert "eventMode = 'static'" in result

    def test_has_cursor(self):
        result = gen_interactive_js("obj", action="unknown")
        assert "cursor = 'pointer'" in result

    def test_no_on_handler(self):
        result = gen_interactive_js("obj", action="unknown")
        assert ".on(" not in result


class TestReturnType:
    """Return value is a string ending with newline."""

    def test_returns_string(self):
        result = gen_interactive_js("x")
        assert isinstance(result, str)

    def test_ends_with_newline(self):
        result = gen_interactive_js("x")
        assert result.endswith("\n")
