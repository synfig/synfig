"""
DashItem.py
To store a single Dash Item, this class will be used

REFER HERE:
https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/dashitem.cpp
"""

import sys
import common
sys.path.append("..")


class DashItem:
    """
    Class to store Synfig format Dash Items
    """
    def __init__(self, offset, length, sidebefore, sideafter):
        self.offset_ = offset
        self.length_ = length
        self.side_type_ = []
        self.side_type_.append(sidebefore)
        self.side_type_.append(sideafter)

    def set_position(self, x):
        self.position_ = x

    def get_side_type_before(self):
        return self.side_type_[0]

    def get_side_type_after(self):
        return self.side_type_[1]

    def get_length(self):
        return self.length_

    def get_offset(self):
        return self.offset_
