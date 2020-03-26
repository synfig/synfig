"""
Gradient.py
Will store the gradient class
"""

import sys
import copy
import math
import common
import settings
from common.Color import Color
sys.path.append("..")


class Gradient:
    """
    To store the gradients in Synfig and operations on them
    Currently, it is assumed that only 2 colors will be provided in the gradient
    """

    def __init__(self, gradient):
        """
        Args:
            gradient (lxml.etree._Element) : Gradient as extracted from the xml

        Returns:
            (None)
        """
        self.synfig_gradient = gradient
        self.colors = []
        self.extract_colors()

    def extract_colors(self):
        """
        Helps in extracting all the colors present in the gradient
        """
        initial_list = []
        for col in self.synfig_gradient:
            red = float(col[0].text) ** (1/settings.GAMMA[0])
            green = float(col[1].text) ** (1/settings.GAMMA[1])
            blue = float(col[2].text) ** (1/settings.GAMMA[2])
            alpha = float(col[3].text)
            initial_list.append([float(col.attrib["pos"]), Color(red, green, blue, alpha)]) 
        self.colors = initial_list

    def reverse_gamma(self, color):
        """
        Given a color, it reverses the effect of gamma as done in extract_colors()

        Args:
            color (common.Color.Color) : color element

        Returns:
            (common.Color.Color) : color element with gamma effect reversed
        """
        ret = copy.deepcopy(color)
        ret.red = ret.red ** settings.GAMMA[0]
        ret.green = ret.green ** settings.GAMMA[1]
        ret.blue = ret.blue ** settings.GAMMA[2]
        return ret

    def get_lottie_final_list(self):
        """
        Appends the avg of colors as required by lottie format. This function is not currently used
        anywhere. But might be helpful in future

        Args:
            (None)

        Returns:
            (list) : list with average of colors appended
        """
        initial_list = copy.deepcopy(self.colors)
        avg_list = []
        # Appending the avg colors, as required by lottie
        for i in range(len(initial_list)-1):
            col1, col2 = initial_list[i], initial_list[i+1]
            avg_pos = (col1[0] + col2[0]) / 2
            avg_col = (col1[1] + col2[1]) / 2
            avg_list.append([avg_pos, avg_col])

        ret = [None]*(len(initial_list) + len(avg_list))
        ret[::2] = initial_list
        ret[1::2] = avg_list
        return ret

    def get_colors(self):
        """
        Returns the list of colors in a gradient

        Args:
            (None)

        Returns:
            (list) : list of colors of whom gradient is comprised
        """
        return self.colors


    def get_val(self):
        """
        Get the gradient in the format required by lottie

        Args:
            (None)

        Returns:
            (list) : Stores the gradient in the list format
        """
        ret = []
        for col in self.colors:
            pos = col[0]
            r, g, b = col[1].red, col[1].green, col[1].blue
            ret.extend([pos, r, g, b])

        # Alpha values of the colors to be fit here
        for col in self.colors:
            pos = col[0]
            a = col[1].alpha
            #ret.extend([pos, a])

        return ret

    def get_color_at_x(self, x):
        """
        Returns the color at any position x(between 0 and 1)
        https://github.com/synfig/synfig/blob/ae11655a9bba068543be7a5df9090958579de78e/synfig-core/src/synfig/gradient.cpp#L178

        Args:
            x (float) : position at which value of color is needed

        Returns:
            (common.Color.Color) : color at position x
        """
        if len(self.colors) == 0:
            return Color()
        if len(self.colors) == 1 or math.isnan(x):
            return self.reverse_gamma(self.colors[0][1])

        # point is outside of gradient
        if x <= self.colors[0][0]:
            return self.reverse_gamma(self.colors[0][1])
        if x >= self.colors[-1][0]:
            return self.reverse_gamma(self.colors[-1][1])

        # upper bound
        i = self.upper_bound(self.colors, x, 0, len(self.colors))
        # considering i stores the upper bound
        j = i
        i = i - 1
        d = self.colors[j][0] - self.colors[i][0]
        if d <= common.misc.real_high_precision():
            return self.reverse_gamma(self.colors[i][1])

        amount = (x - self.colors[i][0]) / d
        col = self.blend(self.reverse_gamma(self.colors[i][1]), self.reverse_gamma(self.colors[j][1]), amount)
        return col

    def blend(self, col1, col2, amount):
        """
        https://github.com/synfig/synfig/blob/ae11655a9bba068543be7a5df9090958579de78e/synfig-core/src/synfig/gradient.cpp#L200

        Args:
            col1 (common.Color.Color) : Color 1
            col2 (common.Color.Color) : Color 2
            amount (float)            : the percentage of what color 1 and color 2 is needed

        Returns:
            (common.Color.Color) : The blended color by some amount 'amount'
        """
        return col1*(1-amount)+col2*amount

    def upper_bound(self, a, x, lo=0, hi=None):
        """
        Function to find the upper bound in an array/list

        Args:
            a (list)  : The list to be searched in
            x (float) : The number whose upper bound is to be found
            lo (int)  : Starting position of the list
            hi (int)  : Ending position of the list

        Returns:
            (int) : The index of upper bound of a number x
        """
        if lo < 0:
            raise ValueError('lo must be non-negative')
        if hi is None:
            hi = len(a)
        while lo < hi:
            mid = (lo+hi)//2
            # Use _lt_ to match the logic in list.sort() and in heapq
            if self.comp(x, a[mid][0]): hi = mid
            else: lo = mid+1
        return lo

    def comp(self, a, b):
        """
        Compares 2 values

        Args:
            a (float) : Number 1 to be compared
            b (float) : Number 2 to be compared

        Returns:
            (bool) : True if b > a under the real_high_precision
                   : False otherwise
        """
        if (b - a) > common.misc.real_high_precision():     # returns True if b > a
            return True
        return False
