"""
Activepoint.py
To store if a point is active at a time T or not

REFER HERE:
https://github.com/synfig/synfig/blob/15607089680af560ad031465d31878425af927eb/synfig-core/src/synfig/activepoint.h
"""

import sys
import settings
from canvas import convert_time_to_frames
from functools import total_ordering
sys.path.append("..")


@total_ordering
class Activepoint:
    """
    Class to store the active point of Synfig
    """
    def __init__(self, time, state):
        self.state = state

        # Convert time to frames
        if time == 'SOT':
            self.time = settings.lottie_format["ip"]
        elif time == 'EOT':
            self.time = settings.lottie_format["op"]
        else:
            self.time = convert_time_to_frames(time)

    def __lt__(self, rhs):
        return ((self.time) < (rhs.time))

    def __eq__(self, rhs):
        return ((self.time) == (rhs.time))
