"""
Param.py
Will store the Parameters class for Synfig parameters 
"""

import sys
import settings
sys.path.append("..")


class Param:
    """
    Class to keep Synfig format parameters
    """
    def __init__(self, param, parent_layer):
        self.parent_layer = parent_layer
        self.param = param

    def get(self):
        """
        Returns the original param
        """
        return self.param
