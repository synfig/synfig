"""
interactive.py — Generate PixiJS interactive event JS code.
"""


def gen_interactive_js(target_name, event_type="pointerdown", action="toggle"):
    """Generate JS code to add interactive events to a PixiJS display object.

    Args:
        target_name: JS variable name of the target display object
        event_type: DOM event type (default: 'pointerdown')
        action: Action type - 'toggle' (visibility) or 'play' (tween animation)

    Returns:
        JS code string that adds interactivity to the target
    """
    lines = [f"  {target_name}.eventMode = 'static';"]
    lines.append(f"  {target_name}.cursor = 'pointer';")
    if action == "toggle":
        lines.append(f"  {target_name}.on('{event_type}', () => {{")
        lines.append(f"    {target_name}.visible = !{target_name}.visible;")
        lines.append(f"  }});")
    elif action == "play":
        lines.append(f"  {target_name}.on('{event_type}', () => {{")
        lines.append(f"    if ({target_name}_tween) {target_name}_tween.play(app.ticker);")
        lines.append(f"  }});")
    return "\n".join(lines) + "\n"
