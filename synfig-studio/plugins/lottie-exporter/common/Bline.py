"""
Bline.py
Will store the Parameters class for Bline
"""

import sys
import common
import settings
from common.BlinePoint import BlinePoint
from common.Vector import Vector
from common.Hermite import Hermite
from synfig.animation import to_Synfig_axis
from common.Angle import TanAngle, DegAngle, CosAngle, SinAngle
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

    def get_list_at_frame_old(self, fr):
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

        iterr = 0
        while iterr != self.get_len():
            entry = self.get_entry_list()[iterr]
            amount = entry["ActivepointList"].amount_at_time(fr, rising)
            assert(amount >= 0.0)
            assert(amount <= 1.0)

            # It's fully on
            if amount > 1.0 - EPSILON:
                if first_flag:
                    first_iter = iterr
                    first = prev = self.get_blinepoint(iterr, fr)
                    first_flag = False
                    ret_list.append(first)
                    iterr += 1
                    index += 1
                    continue
                curr = self.get_blinepoint(iterr, fr)

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
            elif amount > 0.0:
                blp_here_on = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
                blp_here_off = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
                blp_here_now = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
                blp_prev_off = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
                blp_next_off = BlinePoint(Vector(0, 0), 1, True, False, Vector(0, 0), Vector(0, 0))
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

                blp_here_on = self.get_blinepoint(iterr, on_time)
                end_iter = iterr

                end_iter += 1
                while end_iter != self.get_len():
                    if self.get_entry_list()[end_iter]["ActivepointList"].amount_at_time(fr, rising) > amount:
                        break
                    end_iter += 1

                if end_iter == self.get_len():
                    if self.get_loop() and (not first_flag):
                        end_iter = first_iter
                    else:
                        end_iter = self.get_len() - 1

                blp_next_off = self.get_blinepoint(end_iter, off_time)

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

                    if self.get_entry_list()[begin_iter]["ActivepointList"].amount_at_time(fr, rising) > amount:
                        blp_prev_off = self.get_blinepoint(begin_iter, off_time)
                        break

                if blp_prev_off.get_origin() == 100:
                    if first_flag:
                        begin_iter = 0
                    else:
                        begin_iter = first_iter
                    blp_prev_off = self.get_blinepoint(begin_iter, off_time)

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

                # QUERY/DOUBT
                if begin_iter == (iterr - 1) or dist_from_begin == 1:
                    prev_tangent_scalar = self.linear_interpolation(blp_here_on.get_origin(), 1.0, amount)
                else:
                    prev_tangent_scalar = self.linear_interpolation(blp_here_on.get_origin()-prev.get_origin(), 1.0, amount)

                if end_iter == (iterr + 1) or dist_from_end == 1:
                    next_tangent_scalar= self.linear_interpolation(1.0-blp_here_on.get_origin(), 1.0, amount)
                elif self.get_len() != (iterr + 1):
                    nextt = self.get_blinepoint(iterr+1, fr)
                    next_tangent_scalar = self.linear_interpolation(nextt.get_origin()-blp_here_on.get_origin(), 1.0, amount)
                else:
                    next_tangent_scalar = self.linear_interpolation(blp_next_off.get_origin()-blp_here_on.get_origin(), 1.0, amount)
                next_scale=next_tangent_scalar

                # My second try
                off_coord_sys = []
                on_coord_sys = []
                curr_coord_sys = []
                end_pos_at_off_time = self.get_blinepoint(end_iter, off_time).get_vertex()
                begin_pos_at_off_time = self.get_blinepoint(begin_iter, off_time).get_vertex()
                off_coord_origin = (begin_pos_at_off_time + end_pos_at_off_time)/2
                off_coord_sys.append((begin_pos_at_off_time - end_pos_at_off_time).norm())
                off_coord_sys.append(off_coord_sys[0].perp())

                end_pos_at_on_time = self.get_blinepoint(end_iter, on_time).get_vertex()
                begin_pos_at_on_time = self.get_blinepoint(begin_iter, on_time).get_vertex()
                on_coord_origin = (begin_pos_at_on_time + end_pos_at_on_time)/2
                on_coord_sys.append((begin_pos_at_on_time - end_pos_at_on_time).norm())
                on_coord_sys.append(on_coord_sys[0].perp())

                end_pos_at_current_time = self.get_blinepoint(end_iter, fr).get_vertex()
                begin_pos_at_current_time = self.get_blinepoint(begin_iter, fr).get_vertex()
                curr_coord_origin = (begin_pos_at_current_time + end_pos_at_current_time)/2
                curr_coord_sys.append((begin_pos_at_current_time - end_pos_at_current_time).norm())
                curr_coord_sys.append(curr_coord_sys[0].perp())

                # swapping
                temp = curr_coord_sys[0][1]
                curr_coord_sys[0][1] = curr_coord_sys[1][0]
                curr_coord_sys[1][0] = temp

                trans_on_point = Vector(0, 0)
                trans_off_point = Vector(0, 0)
                trans_on_t1 = Vector(0, 0)
                trans_off_t1 = Vector(0, 0)
                trans_on_t2 = Vector(0, 0)
                trans_off_t2 = Vector(0, 0)

                trans_on_point = self.transform_coords(blp_here_on.get_vertex(),  trans_on_point,  on_coord_origin,  on_coord_sys)
                trans_off_point = self.transform_coords(blp_here_off.get_vertex(), trans_off_point, off_coord_origin, off_coord_sys)

                trans_on_t1 = self.transform_coords(blp_here_on.get_tangent1(),  trans_on_t1,  Vector(0, 0), on_coord_sys)
                trans_off_t1 = self.transform_coords(blp_here_off.get_tangent1(), trans_off_t1, Vector(0, 0), off_coord_sys);

                if blp_here_on.get_split_tangent_both():
                    trans_on_t2 = self.transform_coords(blp_here_on.get_tangent2(),  trans_on_t2,  Vector(0, 0), on_coord_sys)
                    trans_off_t2 = self.transform_coords(blp_here_off.get_tangent2(), trans_off_t2, Vector(0, 0), off_coord_sys)

                tmp = Vector(0, 0)
                tmp = self.untransform_coords(self.linear_interpolation(trans_off_point, trans_on_point, amount), tmp, curr_coord_origin, curr_coord_sys)
                blp_here_now.set_vertex(tmp)

                tmp = Vector(0, 0)
                tmp = self.untransform_coords(self.radial_interpolation(trans_off_t1,trans_on_t1,amount), tmp, Vector(0, 0), curr_coord_sys)
                blp_here_now.set_tangent1(tmp)

                # blp_here_now.set_tangent1(self.radial_interpolation(blp_here_off.get_tangent1(), blp_here_on.get_tangent1(), amount))

                if blp_here_on.get_split_tangent_both():
                    blp_here_now.set_split_tangent_both(True)
                    tmp = Vector(0, 0)
                    tmp = self.untransform_coords(self.radial_interpolation(trans_off_t2,trans_on_t2,amount), tmp, Vector(0, 0), curr_coord_sys)
                    blp_here_now.set_tangent2(tmp)
                else:
                    blp_here_now.set_split_tangent_both(False)

                blp_here_now.set_origin(blp_here_on.get_origin())
                blp_here_now.set_width(self.linear_interpolation(blp_here_off.get_width(), blp_here_on.get_width(), amount))

                if first_flag:
                    blp_here_now.set_tangent1(blp_here_now.get_tangent1()*prev_tangent_scalar)
                    first_iter = iterr
                    first = prev = blp_here_now
                    first_flag = False
                    ret_list.append(blp_here_now)
                    iterr += 1
                    index += 1
                    continue

                ret_list[-1].set_split_tangent_both(True)
                ret_list[-1].set_tangent2(prev.get_tangent2()*prev_tangent_scalar)
                ret_list.append(blp_here_now)
                ret_list[-1].set_split_tangent_both(True)
                ret_list[-1].set_tangent1(blp_here_now.get_tangent1()*prev_tangent_scalar)
                prev = blp_here_now

            index += 1
            iterr += 1

        if next_scale != 1:
            ret_list[-1].set_split_tangent_both(True)
            ret_list[-1].set_tangent2(prev.get_tangent2()*next_scale)

        """
        if fr == 1:
            for bp in ret_list:
                v = bp.get_vertex()
                v = bp.get_width()
                v = bp.get_tangent1()
                print(v)
            print("anish", ret_list)
        """
        return ret_list

    def radial_interpolation(self, a, b, c):
        """
        """
        if a.is_equal_to(Vector(0, 0)) or b.is_equal_to(Vector(0, 0)):
            return linear_interpolation(a, b, c)

        mag = self.mag_combo(a.mag(), b.mag(), c)
        angle_a = TanAngle(Vector(a[1], a[0]))
        angle_b = TanAngle(Vector(b[1], b[0]))
        diff = DegAngle(angle_b - angle_a).get()
        if diff < -180: angle_b += DegAngle(360)
        elif diff > 180:    angle_a += DegAngle(360)
        ang = self.ang_combo(angle_a, angle_b, c)

        return Vector(mag*CosAngle(ang).get(), mag*SinAngle(ang).get())

    def mag_combo(self, a, b, t):
        return (b-a)*t+a

    def ang_combo(self, a, b, t):
        return b.dist(a)*t + a

    def untransform_coords(self, inn, out, coord_origin, coord_sys):
        out[0] = inn * coord_sys[0] 
        out[1] = inn * coord_sys[1]
        out += coord_origin
        return out

    def transform_coords(self, inn, out, coord_origin, coord_sys):
        inn -= coord_origin
        out[0] = inn * coord_sys[0]
        out[1] = inn * coord_sys[1]
        return out

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
        vns = bpnext.get_vertex_setup()
        beta01 = t1.angle()
        beta02 = t2.angle()

        alpha = (vn - vp).angle() - (vns - vps).angle()
        if bpcurr.get_split_tangent_both():
            gamma = ((v-(vn+vp)*0.5).angle()-(vn-vp).angle()) - ((vs-(vns+vps)*0.5).angle()-(vns-vps).angle())
        else:
            gamma = Angle(0)

        beta1 = alpha + gamma + beta01
        beta2 = alpha + gamma + beta02

        tt1 = Vector(t1.mag()*CosAngle(beta1).get(), t1.mag()*SinAngle(beta1).get())
        tt2 = Vector(t2.mag()*CosAngle(beta2).get(), t2.mag()*SinAngle(beta2).get())

        bpcurr.set_tangent1(tt1)
        bpcurr.set_tangent2(tt2)

        return bpcurr

