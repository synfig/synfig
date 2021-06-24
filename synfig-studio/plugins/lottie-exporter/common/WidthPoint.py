"""
WidthPoint.py
To store a single Width Point, this class will be used

REFER HERE:
https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/widthpoint.cpp
"""

import sys
import math
import common
from functools import total_ordering
sys.path.append("..")


@total_ordering
class WidthPoint:
    """
    Class to store Synfig format Width points
    ****TODO: Synfig is currently not using lower_bound and upper_bound values
    They are set to 0 and 1 respectively
    """
    def __init__(self, position, width, sidebefore, sideafter, dash = False, lower_bound = 0.0, upper_bound = 1.0):
        self.position_ = position
        self.width_ = width
        self.dash_ = dash
        self.side_type_ = []
        self.side_type_.append(sidebefore)
        self.side_type_.append(sideafter)
        self.lower_bound_ = lower_bound
        self.upper_bound_ = upper_bound

    def set_position(self, x):
        self.position_ = x

    def get_norm_position(self, wplistloop):
        return (self.get_bound_position(wplistloop)- self.lower_bound_) / (self.upper_bound_ - self.lower_bound_)

    def get_bound_position(self, wplistloop):
        ret = self.position_ - self.lower_bound_
        rnge = self.upper_bound_ - self.lower_bound_
        if not wplistloop:
            if ret < 0: ret = 0
            if ret > rnge: ret = rnge
        else:
            ret = math.fmod(ret, rnge)
            ret = ret if (ret >= 0.0) else (rnge + ret) 
        return ret + self.lower_bound_

    def set_side_type_before(self, sidebefore):
        side_type_[0] = sidebefore

    def get_side_type_before(self):
        return self.side_type_[0]

    def set_side_type_after(self, sideafter):
        side_type_[1] = sideafter

    def get_side_type_after(self):
        return self.side_type_[1]

    def get_position(self):
        return self.position_

    def get_width(self):
        return self.width_

    def __lt__(self, rhs):
        return ((self.position_) < (rhs.position_))

    def __eq__(self, rhs):
        return (self.position_ == rhs.position_ and
                self.width_ == rhs.width_ and
                self.lower_bound_ == rhs.lower_bound_ and
                self.upper_bound_ == rhs.upper_bound_ and
                self.side_type[0] == rhs.side_type[0] and
                self.side_type[1] == rhs.side_type[1])
