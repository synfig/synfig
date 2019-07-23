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
        return self.bline[itr]

    def __setitem__(self, itr, val):
        """
        Sets the value of child corresponding to itr
        """
        self.bline[itr] = val

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
        return self.parent.get_layer()

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
            for param in entry[0]:
                tag = param.tag
                entry_list[-1][tag] = common.Param.Param(param, entry[0])
            entry_list[-1]["composite"] = entry[0]

    def get_entry_list(self):
        """
        Returns the entry list
        """
        return self.entry_list
