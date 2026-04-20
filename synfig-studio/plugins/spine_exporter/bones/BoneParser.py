#!/usr/bin/env python
"""
BoneParser.py
This module provides the BoneParser class and helper functions to parse bones
from a Synfig Canvas, compute their global transformation values, and instantiate
Bone objects for spine-exporter.
"""

from bones.Bone import Bone

def compute_bone_transform(bone_param, frame=0):
    """
    Compute the final transformation values for a given bone parameter at a specific frame.
    We call the __get_value method on the bone_param (a Param object) to obtain:
    (origin, angle, local_scale, recursive_scale)

    Returns:
        A tuple (origin, angle, scale) where:
          origin: a list [x, y] representing the global position.
          angle: a float representing rotation (in degrees).
          scale: a tuple (scaleX, scaleY) combining local and recursive scales.
    Note: In a more complete implementation, one might handle vector math more precisely.
    """
    # Retrieve computed values; __get_value should return (origin, angle, local_scale, recursive_scale)
    # For this basic example, we use frame=0
    try:
        origin, angle, local_scale, recursive_scale = bone_param.__get_value(frame)
    except Exception:
        # Fallback values in case of error
        origin, angle, local_scale, recursive_scale = [0, 0], 0, 1, [1, 1]

    # For our basic implementation we assume uniform scaling
    # If recursive_scale is a list [scaleX, scaleY], we could combine it with local_scale
    if isinstance(recursive_scale, (list, tuple)) and len(recursive_scale) == 2:
        scaleX = local_scale * recursive_scale[0]
        scaleY = local_scale * recursive_scale[1]
    else:
        # In case recursive_scale is a single value
        scaleX = local_scale * recursive_scale
        scaleY = scaleX

    return origin, angle, (scaleX, scaleY)

def get_parent_name(bone_param, canvas):
    """
    Find and return the parent bone's name for the given bone_param, if any.
    We check the bone_param's subparameter "parent" to see if a parent guid is provided.

    Parameters:
        bone_param: a Param object representing a bone.
        canvas: the Canvas instance that holds all bones.

    Returns:
        The name of the parent bone if found, else None.
    """
    # Check if a parent parameter exists in the bone's subparams.
    parent_param = bone_param.subparams.get("parent")
    if parent_param:
        # Look for the GUID attribute in the parent parameter.
        guid = parent_param.param.attrib.get("guid", "")
        if guid:
            # Use the canvas.get_bone() method to find the parent's Param.
            parent_bone_param = canvas.get_bone(guid)
            if parent_bone_param:
                # Use the "name" attribute if present, or fallback to the guid.
                return parent_bone_param.param.attrib.get("name", guid)
    return None

def parse_bones(canvas, frame=0):
    """
    Collects and parses all bones from the given canvas.

    For each bone in canvas.bones (a dictionary keyed by guid), we:
      - Compute the global transformation by calling compute_bone_transform.
      - Create a Bone object with an appropriate name, transformation values and parent information.

    Parameters:
        canvas: A Canvas instance that has a dictionary of bones.
        frame: The frame at which to compute bone transformation, typically 0.
        
    Returns:
        A list of Bone objects.
    """
    bones_list = []

    for guid, bone_param in canvas.bones.items():
        # Retrieve a bone's name from the parameter. Fallback to using guid if "name" is absent.
        bone_name = bone_param.param.attrib.get("name", guid)
        # Compute transformation values (origin, angle, and scaling)
        origin, angle, (scaleX, scaleY) = compute_bone_transform(bone_param, frame)
        # Get parent name based on the bone's subparams (if exists)
        parent_name = get_parent_name(bone_param, canvas)

        # Create the Bone instance.
        bone_obj = Bone(
            name=bone_name,
            parent=parent_name,
            x=origin[0],
            y=origin[1],
            rotation=angle,
            scaleX=scaleX,
            scaleY=scaleY
        )

        bones_list.append(bone_obj)

    return bones_list
