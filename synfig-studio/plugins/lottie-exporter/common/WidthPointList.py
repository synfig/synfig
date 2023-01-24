"""
WidthPointList.py
Will store the Parameters class for Width point list
"""

import sys
import common
import properties.shapePropKeyframe as advanced_outline
from synfig.animation import to_Synfig_axis
from common.ActivepointList import ActivepointList
sys.path.append("..")


class WidthPointList:
    """
    Class to store Synfig format Width points list
    """
    def __init__(self, width_point, parent, bline_loop):
        """
        This parent should be parameter
        """
        self.parent = parent
        self.width_point = width_point
        self.bline_loop = bline_loop
        self.type = width_point.attrib["type"]
        self.entry_list = []
        self.extract_entries(self.entry_list)

    def get(self):
        """
        Returns the original param
        """
        return self.width_point

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
        if "loop" in self.width_point.keys():
            v = self.width_point.attrib["loop"]
            if v == "true":
                loop = True
        return loop

    def extract_entries(self, entry_list):
        """
        Stores the entries in a list
        """
        for entry in self.width_point:
            # Assuming it's child is always composite for now
            entry_list.append({})
            entry_list[-1]["ActivepointList"] = ActivepointList(entry.attrib.get("on"), entry.attrib.get("off"))
            element = entry[0]
            for param in element:
                tag = param.tag
                if tag == "animated":   # For dynamic list
                    tag = param.attrib["type"]
                entry_list[-1][tag] = common.Param.Param(param, element)
            assert(entry[0].tag == "composite")
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

    def get_list_at_frame_2(self, fr):
        """
        Returns list of WidthPoint's at a particular frame
        """
        wplist = []
        for entry in self.get_entry_list():
            pos = entry["position"].get_value(fr)
            pos = to_Synfig_axis(pos, "real")
            width = entry["width"].get_value(fr)
            width = to_Synfig_axis(width, "real")

            side_before = entry["side_before"].get()
            side_before = int(side_before[0].attrib["value"])

            side_after = entry["side_after"].get()
            side_after = int(side_after[0].attrib["value"])

            lower_bound = entry["lower_bound"].get()
            lower_bound = float(lower_bound[0].attrib["value"])

            upper_bound = entry["upper_bound"].get()
            upper_bound = float(upper_bound[0].attrib["value"])

            wplist.append(common.WidthPoint.WidthPoint(pos, width, side_before, side_after, False, lower_bound, upper_bound))

        return wplist
    
    def get_value_at_frame(self, entry, fr):
        pos = entry["position"].get_value(fr)
        pos = to_Synfig_axis(pos, "real")
        width = entry["width"].get_value(fr)
        width = to_Synfig_axis(width, "real")

        side_before = entry["side_before"].get()
        side_before = int(side_before[0].attrib["value"])

        side_after = entry["side_after"].get()
        side_after = int(side_after[0].attrib["value"])

        lower_bound = entry["lower_bound"].get()
        lower_bound = float(lower_bound[0].attrib["value"])

        upper_bound = entry["upper_bound"].get()
        upper_bound = float(upper_bound[0].attrib["value"])

        curr = common.WidthPoint.WidthPoint(pos, width, side_before, side_after, False, lower_bound, upper_bound)
        return curr

    def get_list_at_frame(self, fr):
        """
        Refer: https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_wplist.cpp#L281
        """
        wplist = []
        rising = [False]

        for entry in self.get_entry_list():
            amount = entry["ActivepointList"].amount_at_time(fr, rising)
            assert(amount >= 0)
            assert(amount <= 1)

            curr = self.get_value_at_frame(entry, fr)

            # it's fully on
            if amount > 1 - 0.0000001:
                wplist.append(curr)
            # it's partly on
            elif amount > 0:
                # This is where the interesting stuff happens
                # on_time, and off_time are never needed, so why bother writing
                # them?

                i_width = self.interpolated_width(curr.get_norm_position(self.get_loop()), fr)
                curr_width = curr.get_width()

                curr.set_width(i_width*(1.0-amount)+(curr_width)*amount)
                wplist.append(curr)
        return wplist

    def interpolated_width(self, position, time):
        prev = self.find_prev_valid_entry_by_position(position, time)
        neext = self.find_next_valid_entry_by_position(position, time)
        prev.normalize(self.get_loop())
        neext.normalize(self.get_loop())
        return advanced_outline.advanced_outline.widthpoint_interpolate(prev, neext, position)

    def find_prev_valid_entry_by_position(self, position, time):
        prev_pos = -123456
        prev_ret = common.WidthPoint.WidthPoint(prev_pos, 0.0, 0, 0, False)
        if len(self.get_entry_list()) == 0:
            return prev_ret
        for entry in self.get_entry_list():
            curr = self.get_value_at_frame(entry, time)
            curr_pos = curr.get_norm_position(self.get_loop())

            status = entry["ActivepointList"].status_at_time(time)
            if (curr_pos < position) and (curr_pos > prev_pos) and status:
                prev_pos = curr_pos
                prev_ret = curr

        if prev_ret.get_position() == -123456:
            if self.bline_loop:
                prev_ret = self.find_prev_valid_entry_by_position(2.0, time)
            else:
                prev_ret = self.find_next_valid_entry_by_position(-1.0, time)
                prev_ret.set_position(0.0)

        return prev_ret

    def find_next_valid_entry_by_position(self, position, time):
        next_pos = 1.0
        next_ret = common.WidthPoint.WidthPoint(next_pos, 0.0, 0, 0, False)

        for entry in self.get_entry_list():
            curr = self.get_value_at_frame(entry, time)
            curr_pos = curr.get_norm_position(self.get_loop())
            status = entry["ActivepointList"].status_at_time(time)
            if (curr_pos > position) and (curr_pos < next_pos) and status:
                next_pos = curr_pos
                next_ret = curr
        return next_ret
