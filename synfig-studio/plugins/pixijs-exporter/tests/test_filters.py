from converters.filters import gen_blur_filter_js


def test_blur_filter_basic():
    result = gen_blur_filter_js("sprite1", 4, 4)
    expected = "  sprite1.filters = [new BlurFilter({ strengthX: 4, strengthY: 4 })];\n"
    assert result == expected


def test_blur_filter_contains_blur_filter():
    result = gen_blur_filter_js("obj", 2, 3)
    assert "BlurFilter" in result


def test_blur_filter_correct_strengths():
    result = gen_blur_filter_js("obj", 7, 12)
    assert "strengthX: 7" in result
    assert "strengthY: 12" in result


def test_blur_filter_correct_target_name():
    result = gen_blur_filter_js("myContainer", 1, 1)
    assert "myContainer.filters" in result


def test_blur_filter_zero_strength():
    result = gen_blur_filter_js("node", 0, 0)
    expected = "  node.filters = [new BlurFilter({ strengthX: 0, strengthY: 0 })];\n"
    assert result == expected


def test_blur_filter_float_strength():
    result = gen_blur_filter_js("layer", 2.5, 3.7)
    assert "strengthX: 2.5" in result
    assert "strengthY: 3.7" in result
