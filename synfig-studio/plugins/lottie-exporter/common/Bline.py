"""
Bline.py
Will store the Parameters class for Bline
"""

import sys
import common
import settings
from common.BlinePoint import BlinePoint
from synfig.animation import to_Synfig_axis
from properties.shapePropKeyframe.helper import get_tangent_at_frame
sys.path.append("..")


class Bline:
    """
    Class to store Synfig format bline
    """
    def __init__(self, bline, parent):
        """
        This parent should be parameter
        """
        self.parent = parent
        self.bline = bline
        self.type = bline.attrib["type"]
        self.entry_list = []
        self.extract_entries(self.entry_list)

    def get(self):
        """
        Returns the original param
        """
        return self.bline

    def __getitem__(self, itr):
        """
        Returns the child corresponding to itr
        """
        return self.entry_list[itr]

    def __setitem__(self, itr, val):
        """
        Sets the value of child corresponding to itr
        """
        self.entry_list[itr] = val

    def get_layer_type(self):
        """
        Recursively go till the top until layer is not reached
        """
        if isinstance(self.parent, common.Layer.Layer):
            return self.parent.get_type()
        return self.parent.get_layer_type()

    def get_layer(self):
        """
        Recursively find the layer of this bline
        """
        if isinstance(self.parent, common.Layer.Layer):
            return self.parent
        return self.parent

    def get_type(self):
        """
        Returns the type of bline
        """
        return self.type

    def get_loop(self):
        """
        Returns whether the bline is looped or not
        """
        loop = False
        if "loop" in self.bline.keys():
            v = self.bline.attrib["loop"]
            if v == "true":
                loop = True
        return loop

    def extract_entries(self, entry_list):
        """
        Stores the entries in a list
        """
        for entry in self.bline:
            # Assuming it's child is always composite for now
            entry_list.append({})
            if entry[0].tag == "composite":
                element = entry[0]
            else:
                element = entry
            for param in element:
                tag = param.tag
                if tag == "animated":   # For dynamic list
                    tag = param.attrib["type"]
                entry_list[-1][tag] = common.Param.Param(param, element)
            if entry[0].tag == "composite":
                entry_list[-1]["composite"] = element

    def get_entry_list(self):
        """
        Returns the entry list
        """
        return self.entry_list

    def get_len(self):
        """
        Returns the number of entries
        """
        return len(self.entry_list)

    def set_entry(self, itr, entry, tag="vector"):
        """
        Sets the entry at itr to val
        """
        for param in entry:
            self.entry_list[itr][tag] = common.Param.Param(param, entry)

    def get_list_at_frame(self, fr):
        """
        Returns the Bline list at a particular frame
        """
        bline_list = []
        for entry in self.get_entry_list():
            pos = to_Synfig_axis(entry["point"].get_value(fr), "vector")
            pos = common.Vector.Vector(pos[0], pos[1])
            width = to_Synfig_axis(entry["width"].get_value(fr), "real")
            # origin = to_Synfig_axis(entry["origin"].get_value(fr), "real")
            t1 = entry["t1"]
            t2 = entry["t2"]
            split_r = entry["split_radius"]
            split_a = entry["split_angle"]
            t1, t2 = get_tangent_at_frame(t1, t2, split_r, split_a, fr)
            # convert to synfig units
            t1 /= settings.PIX_PER_UNIT
            t2 /= settings.PIX_PER_UNIT

            split_r_val = split_r.get_value(fr)
            split_a_val = split_a.get_value(fr)

            curr = BlinePoint(pos, width, split_r_val, split_a_val, t1, t2)
            bline_list.append(curr)

        return bline_list
