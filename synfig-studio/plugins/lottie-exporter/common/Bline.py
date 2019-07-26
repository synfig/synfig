"""
Bline.py
Will store the Parameters class for Bline
"""

import sys
import settings
import common
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
