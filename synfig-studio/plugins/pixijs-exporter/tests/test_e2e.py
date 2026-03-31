import os
import subprocess
import tempfile

def test_gradient_sif_to_html(fixtures_dir, exporter_dir):
    sif_path = os.path.join(fixtures_dir, "gradient.sif")
    if not os.path.isfile(sif_path):
        import pytest
        pytest.skip("gradient.sif fixture not found")
    with tempfile.NamedTemporaryFile(suffix=".html", delete=False) as f:
        out_path = f.name
    try:
        script = os.path.join(exporter_dir, "pixijs-exporter.py")
        result = subprocess.run(
            ["python3", script, sif_path, out_path],
            capture_output=True, text=True, timeout=30,
            cwd=exporter_dir
        )
        assert result.returncode == 0, f"Export failed: {result.stderr}"
        assert os.path.isfile(out_path)
        with open(out_path, 'r') as f:
            html = f.read()
        assert "<!DOCTYPE html>" in html
        assert "pixi" in html.lower()
        assert "Application" in html
    finally:
        if os.path.exists(out_path):
            os.unlink(out_path)

def test_basic_shapes_sif_to_html(fixtures_dir, exporter_dir):
    sif_path = os.path.join(fixtures_dir, "basic_shapes.sif")
    if not os.path.isfile(sif_path):
        import pytest
        pytest.skip("basic_shapes.sif fixture not found")
    with tempfile.NamedTemporaryFile(suffix=".html", delete=False) as f:
        out_path = f.name
    try:
        script = os.path.join(exporter_dir, "pixijs-exporter.py")
        result = subprocess.run(
            ["python3", script, sif_path, out_path],
            capture_output=True, text=True, timeout=30,
            cwd=exporter_dir
        )
        assert result.returncode == 0, f"Export failed: {result.stderr}"
        with open(out_path, 'r') as f:
            html = f.read()
        assert "<!DOCTYPE html>" in html
        assert len(html) > 500
    finally:
        if os.path.exists(out_path):
            os.unlink(out_path)
