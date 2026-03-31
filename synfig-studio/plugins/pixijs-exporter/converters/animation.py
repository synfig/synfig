"""
animation.py — Convert Synfig Waypoints to SynfigTween keyframe calls
"""

def interpolation_to_easing(interp_type, tension=0, continuity=0, bias=0):
    """Convert Synfig interpolation type to SynfigTween easing parameter."""
    if interp_type == "linear":
        return "'linear'"
    elif interp_type == "constant":
        return "'constant'"
    elif interp_type == "halt":
        return "'ease-out'"
    elif interp_type in ("TCB", "clamped"):
        # Convert TCB to cubic bezier control points
        # Based on Kochanek-Bartels tangent formula
        c1 = (1.0 - tension) * (1.0 + continuity) * (1.0 + bias) / 2.0
        c2 = (1.0 - tension) * (1.0 - continuity) * (1.0 - bias) / 2.0
        if abs(c1 + c2) < 1e-9:
            return "'linear'"
        # Map to cubic-bezier approximation
        x1 = round(max(0, min(1, c2 / (c1 + c2 + 0.001))), 4)
        y1 = round(max(0, min(1, c1)), 4)
        x2 = round(max(0, min(1, 1.0 - c1 / (c1 + c2 + 0.001))), 4)
        y2 = round(max(0, min(1, 1.0 - c2)), 4)
        return f"[{x1}, {y1}, {x2}, {y2}]"
    else:
        return "'linear'"

def waypoints_to_tween_js(target_name, prop_name, waypoints):
    """
    Generate SynfigTween.addKeyframe() calls from Synfig waypoints.

    waypoints: list of {"time": float(seconds), "value": number,
                        "before": str, "after": str,
                        "tension": float, "continuity": float, "bias": float}
    """
    if not waypoints:
        return ""

    lines = []
    for wp in waypoints:
        time_sec = round(wp["time"], 4)
        value = wp["value"]
        easing = interpolation_to_easing(
            wp.get("after", "linear"),
            wp.get("tension", 0),
            wp.get("continuity", 0),
            wp.get("bias", 0)
        )
        lines.append(
            f"  {target_name}_tween.addKeyframe({time_sec}, "
            f"{{ {prop_name}: {value} }}, {easing});"
        )
    return "\n".join(lines) + "\n"

def gen_tween_setup_js(target_name):
    """Generate the SynfigTween instantiation."""
    return (
        f"  const {target_name}_tween = new SynfigTween({target_name});\n"
    )

def gen_tween_play_js(target_name, loop=True):
    """Generate play call (must be after all keyframes are added)."""
    loop_str = "true" if loop else "false"
    return (
        f"  {target_name}_tween.loop = {loop_str};\n"
        f"  {target_name}_tween.play(app.ticker);\n"
    )
