"""
Count.py
Will store the Count class
"""

import sys
sys.path.append("..")


class Count:
    """
    Class to keep count of variable
    """
    def __init__(self):
        """
        Args:
            (None)

        Returns:
            (None)
        """
        self.idx = -1

    def inc(self):
        """
        This method increases the count by 1 and returns the new count

        Args:
            (None)

        Returns:
            (int) : The updated count is returned
        """
        self.idx += 1
        return self.idx
