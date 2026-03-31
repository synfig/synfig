"""Tests for converters.image — PixiJS Sprite with base64-embedded images."""
import base64

import pytest

from converters.image import gen_image_js, _get_mime_type


class TestGetMimeType:
    """Tests for _get_mime_type helper."""

    def test_png(self):
        assert _get_mime_type('photo.png') == 'image/png'

    def test_jpg(self):
        assert _get_mime_type('photo.jpg') == 'image/jpeg'

    def test_jpeg(self):
        assert _get_mime_type('photo.jpeg') == 'image/jpeg'

    def test_gif(self):
        assert _get_mime_type('anim.gif') == 'image/gif'

    def test_webp(self):
        assert _get_mime_type('pic.webp') == 'image/webp'

    def test_unknown_extension_defaults_to_png(self):
        assert _get_mime_type('file.bmp') == 'image/png'

    def test_uppercase_extension(self):
        assert _get_mime_type('PHOTO.PNG') == 'image/png'

    def test_path_with_directories(self):
        assert _get_mime_type('/some/path/to/image.jpg') == 'image/jpeg'


class TestGenImageJs:
    """Tests for gen_image_js code generation."""

    @pytest.fixture()
    def tiny_png(self, tmp_path):
        """Create a minimal PNG file for testing."""
        # Minimal valid 1x1 PNG
        import struct
        import zlib

        def _make_png():
            signature = b'\x89PNG\r\n\x1a\n'

            ihdr_data = struct.pack('>IIBBBBB', 1, 1, 8, 2, 0, 0, 0)
            ihdr_crc = zlib.crc32(b'IHDR' + ihdr_data) & 0xFFFFFFFF
            ihdr = struct.pack('>I', 13) + b'IHDR' + ihdr_data + struct.pack('>I', ihdr_crc)

            raw = b'\x00\x00\x00\x00'
            compressed = zlib.compress(raw)
            idat_crc = zlib.crc32(b'IDAT' + compressed) & 0xFFFFFFFF
            idat = struct.pack('>I', len(compressed)) + b'IDAT' + compressed + struct.pack('>I', idat_crc)

            iend_crc = zlib.crc32(b'IEND') & 0xFFFFFFFF
            iend = struct.pack('>I', 0) + b'IEND' + struct.pack('>I', iend_crc)

            return signature + ihdr + idat + iend

        path = tmp_path / 'test.png'
        path.write_bytes(_make_png())
        return path

    def test_output_contains_assets_load(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'Assets.load' in result

    def test_output_contains_base64_data_uri(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'data:image/png;base64,' in result

    def test_output_contains_new_sprite(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'new Sprite(myImg_tex)' in result

    def test_output_contains_position(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'myImg.position.set(10, 20)' in result

    def test_output_contains_width(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'myImg.width = 100' in result

    def test_output_contains_height(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'myImg.height = 200' in result

    def test_output_contains_add_child(self, tiny_png):
        result = gen_image_js('myImg', str(tiny_png), 10, 20, 100, 200)
        assert 'app.stage.addChild(myImg)' in result

    def test_base64_data_is_valid(self, tiny_png):
        """The embedded base64 can be decoded back to the original bytes."""
        result = gen_image_js('myImg', str(tiny_png), 0, 0, 1, 1)
        # Extract base64 portion
        prefix = 'data:image/png;base64,'
        start = result.index(prefix) + len(prefix)
        # base64 ends at the closing quote
        end = result.index("'", start)
        b64_str = result[start:end]
        decoded = base64.b64decode(b64_str)
        assert decoded == tiny_png.read_bytes()

    def test_file_not_found_raises(self):
        with pytest.raises(FileNotFoundError):
            gen_image_js('img', '/nonexistent/path/img.png', 0, 0, 1, 1)

    def test_returns_string(self, tiny_png):
        result = gen_image_js('spr', str(tiny_png), 5, 10, 50, 60)
        assert isinstance(result, str)

    def test_different_name_used_throughout(self, tiny_png):
        result = gen_image_js('hero', str(tiny_png), 0, 0, 32, 32)
        assert 'hero_tex' in result
        assert 'hero.position' in result
        assert 'hero.width' in result
        assert 'hero.height' in result
        assert 'addChild(hero)' in result
