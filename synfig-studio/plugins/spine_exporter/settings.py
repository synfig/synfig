#!/usr/bin/env python
"""
settings.py
Configuration settings for spine-exporter.
This file defines global constants such as tag identifiers, default values,
and simple helper classes for counters.
"""
#ROOT_CANVAS = None
# ---------------------------------------------------------------------
# Bone-related settings
# ---------------------------------------------------------------------
# Tags to identify nodes representing bones in the Synfig XML.
BONES = {"bone", "bone_root"}

# Tags for methods used in converting/linking values.
CONVERT_METHODS = {"bone_link"}

# ---------------------------------------------------------------------
# Layer type settings
# ---------------------------------------------------------------------
# These sets (or strings) help identify different types of layers.
GROUP_LAYER = {"group"}
PRE_COMP_LAYER = {"precomp"}
SHAPE_LAYER = {"shape"}
SOLID_LAYER = {"solid"}
SHAPE_SOLID_LAYER = {"shapesolid"}  # if applicable
IMAGE_LAYER = {"image"}
UNKNOWN_LAYER = "unknown"

# Default names for various layer types.
LAYER_SHAPE_NAME = "ShapeLayer_"
LAYER_SOLID_NAME = "SolidLayer_"
LAYER_IMAGE_NAME = "ImageLayer_"
LAYER_PRECOMP_NAME = "PrecompLayer_"

# ---------------------------------------------------------------------
# Canvas and Layer Naming Defaults
# ---------------------------------------------------------------------
CANVAS_NAME = "Canvas_"

# ---------------------------------------------------------------------
# Simple incremental counter helpers for unique names
# ---------------------------------------------------------------------
class Incrementor:
    def __init__(self):
        self.count = 0

    def inc(self):
        self.count += 1
        return self.count

# Global counters used for unique naming in the exporter.
layer_count = Incrementor()
canvas_count = Incrementor()

# ---------------------------------------------------------------------
# Additional Settings
# ---------------------------------------------------------------------
# If needed, specify an export frame to evaluate animations.
DEFAULT_EXPORT_FRAME = 0

# If additional logging or debugging options are needed, add them here:
DEBUG = False
