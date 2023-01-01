#!/usr/bin/env python

#
# Copyright (c) 2012 by Konstantin Dmitriev <ksee.zelgadis@gmail.com>
# Copyright (c) 2022 by Erwan le Gall <synfig@elegant.codes>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

import os
import sys
import xml.etree.ElementTree as ET

def process(filename):
	
	tree = ET.parse(filename)
	root = tree.getroot()
	for inactive_layer in root.findall('layer[@active="false"]'):
		inactive_layer.set('active', 'true')
	tree.write(filename, encoding='UTF-8')

if len(sys.argv) < 2:
	sys.exit()
else:
	process(sys.argv[1])

