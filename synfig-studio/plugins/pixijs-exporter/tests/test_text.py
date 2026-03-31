from converters.text import gen_text_js, _hex_to_css


def test_hex_to_css_converts_0x_prefix():
    assert _hex_to_css("0xRRGGBB") == "#RRGGBB"
    assert _hex_to_css("0xff0000") == "#ff0000"
    assert _hex_to_css("0x00ff00") == "#00ff00"


def test_hex_to_css_passthrough_non_0x():
    assert _hex_to_css("#ff0000") == "#ff0000"
    assert _hex_to_css("red") == "red"
    assert _hex_to_css("rgb(0,0,0)") == "rgb(0,0,0)"


def test_gen_text_js_contains_new_text():
    code = gen_text_js("label1", "Hello", 100, 200, "Arial", 24, "0xff0000", 1.0)
    assert "new Text(" in code


def test_gen_text_js_contains_text_content():
    code = gen_text_js("label1", "Hello World", 0, 0, "Arial", 16, "0x000000", 1.0)
    assert "Hello World" in code


def test_gen_text_js_contains_font_family():
    code = gen_text_js("t", "x", 0, 0, "Courier New", 12, "0x000000", 1.0)
    assert "Courier New" in code


def test_gen_text_js_contains_font_size():
    code = gen_text_js("t", "x", 0, 0, "Arial", 32, "0x000000", 1.0)
    assert "32" in code


def test_gen_text_js_fill_color_as_css():
    code = gen_text_js("t", "x", 0, 0, "Arial", 16, "0xaabbcc", 1.0)
    assert "#aabbcc" in code
    assert "0xaabbcc" not in code


def test_gen_text_js_position():
    code = gen_text_js("t", "x", 150, 250, "Arial", 16, "0x000000", 1.0)
    assert ".position.set(150, 250)" in code


def test_gen_text_js_alpha():
    code = gen_text_js("t", "x", 0, 0, "Arial", 16, "0x000000", 0.5)
    assert ".alpha = 0.5" in code


def test_gen_text_js_add_child():
    code = gen_text_js("myText", "hi", 0, 0, "Arial", 16, "0x000000", 1.0)
    assert "app.stage.addChild(myText)" in code


def test_gen_text_js_variable_name():
    code = gen_text_js("greeting", "hi", 0, 0, "Arial", 16, "0x000000", 1.0)
    assert "const greeting = new Text(" in code


def test_gen_text_js_special_characters_quotes():
    code = gen_text_js("t", "He said \"hello\"", 0, 0, "Arial", 16, "0x000000", 1.0)
    assert "hello" in code
    assert "new Text(" in code


def test_gen_text_js_special_characters_newline():
    code = gen_text_js("t", "line1\nline2", 0, 0, "Arial", 16, "0x000000", 1.0)
    assert "line1" in code
    assert "line2" in code
    assert "new Text(" in code
