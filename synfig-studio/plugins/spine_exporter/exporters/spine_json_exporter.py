#!/usr/bin/env python
"""
spine_json_exporter.py
This module provides functionality to export a list of Bone objects
into a Spine-compatible JSON format. The exported JSON is structured
with a "bones" array containing each bone's transformation data.
"""

import json

def build_spine_json(bones_list):
    """
    Given a list of Bone objects, build the Spine JSON structure.

    Parameters:
        bones_list (list): A list of Bone objects.
        
    Returns:
        A dictionary representing the Spine JSON data.
        Example:
            {
                "bones": [
                    {"name": "root", "x": 0, "y": 0, "rotation": 0, "scaleX": 1, "scaleY": 1},
                    {"name": "bone1", "parent": "root", "x": 100, "y": 0, "rotation": 45, "scaleX": 1, "scaleY": 1}
                ]
            }
    """
    bones_data = []
    for bone in bones_list:
        bones_data.append(bone.to_dict())

    spine_json = {
        "bones": bones_data
        # More elements can be added here (e.g., "slots", "skins", "animations")
    }
    return spine_json

def export_spine_json(bones_list, output_file):
    """
    Exports the given list of Bone objects into a JSON file in Spine format.

    Parameters:
        bones_list (list): A list of Bone objects.
        output_file (str): The path to the output JSON file.
    """
    spine_data = build_spine_json(bones_list)
    with open(output_file, "w") as f_out:
        json.dump(spine_data, f_out, indent=2)
    print("Spine JSON exported to:", output_file)

# For testing as script:
if __name__ == "__main__":
    # For testing purposes, we create a dummy bones list.
    from bones.Bone import Bone

    # Dummy bone objects for example:
    dummy_bones = [
        Bone(name="root", x=0, y=0, rotation=0, scaleX=1, scaleY=1),
        Bone(name="bone1", parent="root", x=100, y=50, rotation=30, scaleX=1, scaleY=1)
    ]
    export_spine_json(dummy_bones, "spine_export.json")
