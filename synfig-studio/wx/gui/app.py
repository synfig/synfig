# -*- coding: utf-8 -*-

# ====================== S Y N F I G =============================================
# 	File: app.py
#	Description: 
#
#	This package is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License as
#	published by the Free Software Foundation; either version 2 of
#	the License, or (at your option) any later version.
#
#	This package is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#	General Public License for more details.
#
# =========================================================================

import os
import sys
import math
import wx
import wx.aui

if sys.platform == 'win32':
	WIN32 = 1
	WINVER = 0x0500
	SINGLE_THREADED = 1


from general import *


MISC_DIR_PREFERENCE		    = "misc_dir"
ANIMATION_DIR_PREFERENCE	= "animation_dir"
IMAGE_DIR_PREFERENCE		= "image_dir"
SKETCH_DIR_PREFERENCE		= "sketch_dir"
RENDER_DIR_PREFERENCE		= "render_dir"

def DPM2DPI(x):
	return (float(x)/39.3700787402)
def DPI2DPM(x):
	return (float(x)*39.3700787402)

if sys.platform == 'win32':
	IMAGE_DIR = "share\\pixmaps"
else:
	IMAGE_DIR = "/usr/local/share/pixmaps"

IMAGE_EXT = "tif"

if sys.platform == 'win32':
	PLUGIN_DIR = "share\\synfig\\plugins"
else:
	PLUGIN_DIR = "/usr/local/share/synfig/plugins"


class App(wx.Frame):
	def __init__(self, basepath, argc, argv):
		wx.Frame.__init__(self,None, -1, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE |wx.SUNKEN_BORDER |wx.CLIP_CHILDREN)

		self.app_base_path_=os.path.dirname(basepath)
		self.SetTitle(_("Synfig Studio (Experimental)"))
		self.Show()

