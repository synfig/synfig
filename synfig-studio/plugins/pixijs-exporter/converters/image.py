"""
image.py — Generate PixiJS Sprite JS code with base64-embedded image data.
"""
import base64
import os


MIME_MAP = {
    'png': 'image/png',
    'jpg': 'image/jpeg',
    'jpeg': 'image/jpeg',
    'gif': 'image/gif',
    'webp': 'image/webp',
}


def _get_mime_type(image_path):
    """Get MIME type from file extension."""
    ext = image_path.rsplit('.', 1)[-1].lower()
    return MIME_MAP.get(ext, 'image/png')


def gen_image_js(name, image_path, x, y, w, h, project_dir=None):
    """Generate JS code to create a PixiJS Sprite from a base64-encoded image.

    Args:
        name: JS variable name
        image_path: Path to the image file on disk
        x: X position in pixels
        y: Y position in pixels
        w: Display width in pixels
        h: Display height in pixels
        project_dir: If provided, validate image_path is within this directory

    Returns:
        JS code string creating a PixiJS Sprite with inline base64 data URI

    Raises:
        FileNotFoundError: If image_path doesn't exist
        ValueError: If image_path resolves outside project_dir
    """
    real_path = os.path.realpath(image_path)
    if project_dir:
        real_project = os.path.realpath(project_dir)
        if not real_path.startswith(real_project + os.sep):
            raise ValueError(
                f"Image path '{image_path}' resolves outside project directory"
            )
    with open(real_path, 'rb') as f:
        data = base64.b64encode(f.read()).decode()
    mime = _get_mime_type(image_path)
    return (
        f"  const {name}_tex = await Assets.load('data:{mime};base64,{data}');\n"
        f"  const {name} = new Sprite({name}_tex);\n"
        f"  {name}.position.set({x}, {y});\n"
        f"  {name}.width = {w};\n"
        f"  {name}.height = {h};\n"
        f"  app.stage.addChild({name});\n"
    )
