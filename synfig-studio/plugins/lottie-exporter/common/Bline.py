"""
Bline.py
Will store the Parameters class for Bline
"""

import sys
import common
import settings
from common.BlinePoint import BlinePoint
from synfig.animation import to_Synfig_axis
from common.ActivepointList import ActivepointList
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
            entry_list[-1]["ActivepointList"] = ActivepointList(entry.attrib.get("on"), entry.attrib.get("off"))
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
            curr = self.get_value_at_frame(entry, fr)
            bline_list.append(curr)
        return bline_list

    def get_value_at_frame(self, entry, fr):
        """
        """
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
        return BlinePoint(pos, width, split_r_val, split_a_val, t1, t2)

    def get_list_at_frame(self, fr):
        """
        Returns the Bline list at a particular frame
        Refer: https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_bline.cpp
        """
        EPSILON = 0.0000001
        ret_list = []
        first_flag = True
        rising = [False]
        index = 0
        next_scale = 1.0

        first = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
        first.set_origin(100)

        for iterr in range(self.get_len()):
            entry = self.get_entry_list()[iterr]
            amount = entry["ActivepointList"].amount_at_time(fr, rising)
            assert(amount >= 0.0)
            assert(amount <= 1.0)

            # It's fully on
            if amount > 1.0 - EPSILON:
                if first_flag:
                    first_iter = iterr
                    first = prev = get_blinepoint(iterr, fr)
                    first_flag = False
                    ret_list.append(first)
                    continue
                curr = get_blinepoint(iterr, fr)

                if next_scale != 1.0:
                    ret_list[-1].set_split_tangent_both(True)
                    ret_list[-1].set_tangent2(prev.get_tangent2()*next_scale)
                    ret_list.append(curr)
                    ret_list[-1].set_split_tangent_both(True)
                    ret_list[-1].set_tangent2(curr.get_tangent2())
                    ret_list[-1].set_tangent1(curr.get_tangent1()*next_scale)
                    next_scale = 1.0
                else:
                    ret_list.append(curr)
                prev = curr

            # It's partly on
            elif amount > 1.0:
                blp_prev_off = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
                blp_here_off = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
                dist_from_begin = 0
                dist_from_end = 0

                if not rising[0]:
                    try:
                        on_time = self.get_entry_list()[iterr]["ActivepointList"].find_prev(fr).get_time()
                    except Exception as e:
                        on_time = -32767*512
                    try:
                        off_time = self.get_entry_list()[iterr]["ActivepointList"].find_next(fr).get_time()
                    except:
                        off_time = 32767*512
                else:
                    try:
                        off_time = self.get_entry_list()[iterr]["ActivepointList"].find_prev(fr).get_time()
                    except Exception as e:
                        off_time = -32767*512
                    try:
                        on_time = self.get_entry_list()[iterr]["ActivepointList"].find_next(fr).get_time()
                    except:
                        on_time = 32767*512

                blp_here_on = get_blinepoint(iterr, on_time)
                end_iter = iterr

                end_iter += 1
                while end_iter != self.get_len():
                    if self.get_entry_list()[end_iter]["ActivepointList"].amount_at_time(fr) > amount:
                        break
                    end_iter += 1

                if end_iter == self.get_len():
                    if self.get_loop() and (not first_flag):
                        end_iter = first_iter
                    else:
                        end_iter = self.get_len() - 1

                blp_next_off = get_blinepoint(end_iter, off_time)

                begin_iter = iterr
                blp_prev_off.set_origin(100)

                while True:
                    if begin_iter == 0:
                        if self.get_loop():
                            begin_iter = self.get_len()
                        else:
                            break
                    begin_iter -= 1
                    dist_from_begin += 1

                    if begin_iter == iterr:
                        break

                    if self.get_entry_list()[begin_iter]["ActivepointList"].amount_at_time(fr) > amount:
                        blp_prev_off = get_blinepoint(begin_iter, off_time)
                        break

                if blp_prev_off.get_origin() == 100:
                    if first_flag:
                        begin_iter = 0
                    else:
                        begin_iter = first_iter
                    blp_prev_off = get_blinepoint(begin_iter, off_time)

                curve = Hermite(blp_prev_off.get_vertex(),
                                blp_next_off.get_vertex(),
                                blp_prev_off.get_tangent2(),
                                blp_next_off.get_tangent1())

                blp_here_off.set_vertex(curve.value(blp_here_on.get_origin()))
                blp_here_off.set_width((blp_next_off.get_width()-blp_prev_off.get_width())*blp_here_on.get_origin()+blp_prev_off.get_width())
                blp_here_off.set_tangent1(curve.derivative(blp_here_on.get_origin()));
                blp_here_off.set_tangent2(curve.derivative(blp_here_on.get_origin()));

                prev_tangent_scalar = 1
                next_tangent_scalar = 1

                if begin_iter == (iterr - 1) or dist_from_begin == 1:
                    prev_tangent_scalar = self.linear_interpolation(blp_here_on.get_origin(), 1.0, amount)
                else:
                    prev_tangent_scalar = self.linear_interpolation(blp_here_on.get_origin()-prev.get_origin(), 1.0, amount)

                if end_iter == (iterr + 1) or dist_from_end == 1:
                    next_tangent_scalar=linear_interpolation(1.0-blp_here_on.get_origin(), 1.0, amount)
                elif self.get_len() != (iterr + 1):
                    nextt = get_blinepoint(iterr+1, fr)
                    next_tangent_scalar = linear_interpolation(nextt.get_origin()-blp_here_on.get_origin(), 1.0, amount)
                else:
                    next_tangent_scalar=linear_interpolation(blp_next_off.get_origin()-blp_here_on.get_origin(), 1.0, amount)
                    next_scale=next_tangent_scalar
                     

            index += 1

    def linear_interpolation(self, a, b, c):
        return (b-a)*c + a

    def get_blinepoint(self, current, t):
        """
        """
        bpcurr = self.get_value_at_frame(self.get_entry_list()[current], t)
        if not bpcurr.get_boned_vertex_flag():
            return bpcurr

        nxt = current
        previous = current
        
        nxt += 1
        if nxt == self.get_len():
            nxt = 0
        if current == 0:
            previous = self.get_len()
        previous -= 1

        bpprev = self.get_value_at_frame(self.get_entry_list()[previous], t)
        bpnext = self.get_value_at_frame(self.get_entry_list()[nxt], t)

        t1 = bpcurr.get_tangent1()
        t2 = bpcurr.get_tangent2()
        v = bpcurr.get_vertex()
        vp = bpprev.get_vertex()
        vn = bpnext.get_vertex()
        vs = bpcurr.get_vertex_setup()
        vps = bpprev.get_vertex_setup()
        vnx = bpnext.get_vertex_setup()
        beta01 = t1.angle()
        beta02 = t2.angle()

        alpha = (vn - vp).angle() - (vns - vps).angle()
        if bpcurr.get_split_tangent_both():
            gamma = ((v-(vn+vp)*0.5).angle()-(vn-vp).angle()) - ((vs-(vns+vps)*0.5).angle()-(vns-vps).angle())
        else:
            gamma = Angle()

        beta1 = alpha + gamma + beta01
        beta2 = alpha + gamma + beta02

        tt1 = Vector(t1.mag()*CosAngle(beta1).get(), t1.mag()*SinAngle(beta1).get())
        tt2 = Vector(t2.mag()*CosAngle(beta2).get(), t2.mag()*SinAngle(beta2).get())

        bpcurr.set_tangent1(tt1)
        bpcurr.set_tangent2(tt2)

        return bpcurr

