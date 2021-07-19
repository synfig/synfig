"""
ActivepointList.py
To store the on/off times of spline points

REFER HERE:
https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/activepoint.h
"""

import sys
from common.Activepoint import Activepoint
sys.path.append("..")


class ActivepointList:
    """
    Class to store the active points of Synfig
    """
    def __init__(self, on_time, off_time):
        self.active_point_list = []
        if on_time is None and off_time is None:
            return

        if on_time is not None:
            self.on_time = on_time.split(",")
            for time in self.on_time:
                self.active_point_list.append(Activepoint(time, True))

        if off_time is not None:
            self.off_time = off_time.split(",")
            for time in self.off_time:
                self.active_point_list.append(Activepoint(time, False))

        # Sort this list, and remove duplicates if any
        # Removing duplicates
        self.active_point_list = self.remove_duplicates()
        self.active_point_list = sorted(self.active_point_list)

    def remove_duplicates(self):
        """
        https://stackoverflow.com/questions/480214/how-do-you-remove-duplicates-from-a-list-whilst-preserving-order

        If at a single time we have two stops, then remove one of that
        """
        seq = self.active_point_list
        seen = set()
        seen_add = seen.add
        return [x for x in seq if not (x.time in seen or seen_add(x.time))]

    def empty(self):
        return len(self.active_point_list) == 0

    def find(self, frame):
        """
        https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_dynamiclist.cpp#L268
        """
        for itr in self.active_point_list:
            if itr.time == frame:
                return itr

        raise Exception("Value not found")

    def find_prev(self, frame):
        """
        https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_dynamiclist.cpp#L324
        """
        itr = len(self.active_point_list)

        while True:
            itr -= 1
            if self.active_point_list[itr].time < frame:
                return self.active_point_list[itr]

            if not (itr != 0):
                break

        raise Exception("Could not find prev")

    def find_next(self, frame):
        """
        https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_dynamiclist.cpp#L296
        """
        for itr in self.active_point_list:
            if itr.time > frame:
                return itr

        raise Exception("Could not find next")

    def amount_at_time(self, frame, rising):
        """
        https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_dynamiclist.cpp#L394
        """
        if self.empty():
            return 1

        try:
            itr = self.find(frame)
            return 1 if itr.state else 0
        except Exception as e:
            pass

        try:
            prev_itr = self.find_prev(frame)
        except Exception as e:
            return 1 if self.find_next(frame).state else 0

        try:
            next_itr = self.find_next(frame)
        except Exception as e:
            return 1 if prev_itr.state else 0

        if next_itr.state == prev_itr.state:
            return 1 if next_itr.state else 0 

        if rising[0]:
            rising[0] = next_itr.state

        if next_itr.state == True:
            return float((frame - prev_itr.time)/(next_itr.time - prev_itr.time))
        return float((next_itr.time - frame)/(next_itr.time - prev_itr.time))

    def status_at_time(self, t):
        """
        https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/valuenodes/valuenode_dynamiclist.cpp#L441
        """
        state = True
        if not self.empty():
            if len(self.active_point_list) == 1:
                state = self.active_point_list[0].state
            else:
                entry_itr = 0
                while entry_itr != len(self.active_point_list):
                    if self.active_point_list[entry_itr].time == t:
                        return self.active_point_list[entry_itr].state
                    if self.active_point_list[entry_itr].time > t:
                        break
                    entry_itr += 1
                prev_itr = entry_itr
                prev_itr -= 1

                if entry_itr == len(self.active_point_list):
                    state = self.active_point_list[prev_itr].state
                elif entry_itr == 0:
                    state = self.active_point_list[entry_itr].state
                elif self.active_point_list[entry_itr].priority == self.active_point_list[prev_itr].priority:
                    state = self.active_point_list[entry_itr].state or self.active_point_list[prev_itr].state
                elif self.active_point_list[entry_itr].priority > self.active_point_list[prev_itr].priority:
                    state = self.active_point_list[entry_itr].state
                else:
                    state = self.active_point_list[prev_itr].state

        return state

    def update_frame_window(self, window):
        """
        Find the minimum and maximum points at which the waypoints are located
        """
        if self.empty():
            return

        for itr in self.active_point_list:
            fr = itr.time

            if fr > window["last"]:
                window["last"] = fr
            if fr < window["first"]:
                window["first"] = fr
