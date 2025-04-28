#!/usr/bin/env python
"""
Bone.py
This module defines the Bone class for spine-exporter.
A Bone instance holds the final computed transformation values for a bone:
• name : The bone’s unique name.
• parent : Name of the parent bone (or None for root).
• x, y : Global position coordinates.
• rotation : Rotation (in degrees).
• scaleX, scaleY: Scaling factors along x and y axes.
The Bone class also provides a method to represent the bone data as a dictionary
following the typical Spine JSON format.
"""

class Bone:
    def init(self, name, parent=None, x=0, y=0, rotation=0, scaleX=1, scaleY=1):
        self.name = name
        self.parent = parent
        self.x = x
        self.y = y
        self.rotation = rotation
        self.scaleX = scaleX
        self.scaleY = scaleY

    def to_dict(self):
        """
        Returns the bone data as a dictionary in a format comparable to the Spine JSON schema.
        Example:
            {
            "name": "bone1",
            "parent": "root",
            "x": 100,
            "y": 0,
            "rotation": 45,
            "scaleX": 1,
            "scaleY": 1
            }
        Note: The "parent" key may be omitted or set to None if this bone has no parent.
        """
        bone_dict = {
            "name": self.name,
            "x": self.x,
            "y": self.y,
            "rotation": self.rotation,
            "scaleX": self.scaleX,
            "scaleY": self.scaleY
        }
        if self.parent:
            bone_dict["parent"] = self.parent
        return bone_dict

    def __str__(self):
        """
        Returns a readable string representation of the Bone.
        """
        return "Bone(name={}, parent={}, x={}, y={}, rotation={}, scaleX={}, scaleY={})".format(
            self.name, self.parent, self.x, self.y, self.rotation, self.scaleX, self.scaleY
        )

    def __repr__(self):
        return self.__str__()