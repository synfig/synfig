"""
BlinePoint.py
To store a single Bline point, this class will be used

REFER HERE:
https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/dashitem.cpp
"""

import sys
from common.Vector import Vector
sys.path.append("..")


class BlinePoint:
    """
    Class to store Synfig format Bline points
    TODO: origin_ is not kept as a parameter for Blinepoint because it is not
    yet used. We can set it as a parameter once we know where to use it.
    """
    def __init__(self, vertex_, width_, split_tangent_radius_, split_tangent_angle_, tangent_1, tangent_2, origin_ = 0, boned_vertex_ = False):
        self.vertex_ = vertex_
        self.width_ = width_
        self.origin_ = 0
        self.split_tangent_radius_ = split_tangent_radius_
        self.split_tangent_angle_ = split_tangent_angle_
        self.boned_vertex_ = boned_vertex_
        self.tangent_ = []
        self.tangent_.append(tangent_1)
        self.tangent_.append(tangent_2)
        self.origin_ = origin_
        self.vertex_setup_ = vertex_
        self.update_flags()
        self.update_tangent2()

    def update_flags(self):
        self.split_tangent_both_ = self.split_tangent_radius_ and self.split_tangent_angle_
        self.merge_tangent_both_ = (not self.split_tangent_radius_) and (not self.split_tangent_angle_)

    def update_tangent2(self):
        if self.tangent_[0].mag_squared() != 0:
            self.tangent2_radius_split_ = Vector(self.tangent_[1].mag(), self.tangent_[0].angle())
        else:
            self.tangent2_radius_split_ = self.tangent_[1]
        self.tangent2_angle_split_ = Vector(self.tangent_[0].mag(), self.tangent_[1].angle())

    def get_tangent2(self):
        if self.merge_tangent_both_:
            return self.tangent_[0]
        if self.split_tangent_both_:
            return self.tangent_[1]
        if self.split_tangent_radius_:
            return self.tangent2_radius_split_
        return self.tangent2_angle_split_

    def get_tangent1(self):
        return self.tangent_[0]

    def set_tangent1(self, x):
        self.tangent_[0] = x
        self.update_tangent2()

    def set_tangent2(self, x):
        self.tangent_[1] = x
        self.update_tangent2()

    def get_vertex(self):
        return self.vertex_

    def get_split_tangent_angle(self):
        return self.split_tangent_angle_

    def get_split_tangent_radius(self):
        return self.split_tangent_radius_

    def get_width(self):
        return self.width_

    def set_width(self, x):
        self.width_ = x

    def set_origin(self, x):
        self.origin_ = x

    def get_origin(self):
        return self.origin_

    def get_boned_vertex_flag(self):
        return self.boned_vertex_

    def set_split_tangent_both(self, x=True):
        self.set_split_tangent_radius(x)
        self.set_split_tangent_angle(x)

    def get_split_tangent_both(self):
        return self.split_tangent_both_

    def set_split_tangent_radius(self, x=True):
        self.split_tangent_radius_ = x
        self.update_tangent2()
        self.update_flags()

    def set_split_tangent_angle(self, x=True):
        self.split_tangent_angle_ = x
        self.update_tangent2()
        self.update_flags()

    def set_vertex(self, x):
        self.vertex_ = x
        self.vertex_setup_ = self.vertex_
