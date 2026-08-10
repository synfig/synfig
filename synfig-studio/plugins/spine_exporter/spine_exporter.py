#!/usr/bin/env python
"""
spine-exporter.py
Main entry point for the spine-exporter plugin.

This script performs the following steps:

- Parses a Synfig .sif file (XML format).
- Constructs a Canvas instance from the XML.
- Uses BoneParser to traverse the bone hierarchy and compute global transformations.
- Exports the computed bone data in Spine JSON format.

Usage:
    python spine-exporter.py input.sif output.json
"""

import sys
from lxml import etree
from core.Canvas import Canvas
from bones.BoneParser import parse_bones
from exporters.spine_json_exporter import export_spine_json

def main():
    if len(sys.argv) < 3:
        print("Usage: python spine-exporter.py input.sif output.json")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    try:
        tree = etree.parse(input_file)
    except Exception as e:
        print("Error parsing input file '{}': {}".format(input_file, e))
        sys.exit(1)

    root = tree.getroot()

    # Create a Canvas instance from the root XML element.
    canvas = Canvas(root, is_root_canvas=True)

    # Parse the bones at frame 0.
    bones_list = parse_bones(canvas, frame=0)

    # Export the bones data to a Spine JSON file.
    export_spine_json(bones_list, output_file)
    print("Export complete.")

if __name__ == "__main__":
    main()
