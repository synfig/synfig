#!/usr/bin/env python

#
# Copyright (c) 2012-2013 by Konstantin Dmitriev <ksee.zelgadis@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

import os
import sys
import codecs

def check_substring(data, substring):
	s = "\n".join(data);
	if substring in s:
			return True
	return False

def process(filename):
	def merge_defs():
		defs=False
		for l in template_contents:
			if defs:
				if not "</defs>" in l:
					inputfile_f.write(l)
				else:
					break
			elif "<defs>" in l:
				defs=True
	
	template_filename = os.path.join(os.path.dirname(sys.argv[0]), 'stickman.sif')

	template_f = codecs.open(template_filename, 'r', encoding='utf-8')
	template_contents = template_f.readlines()
	template_f.close()
	
	# Read the input file
	inputfile_f = codecs.open(filename, 'r', encoding='utf-8')
	inputfile_contents = inputfile_f.readlines()
	inputfile_f.close()
	
	# Now write results to the same file
	inputfile_f = open(filename, 'w', encoding='utf-8')

	num=1
	while check_substring(inputfile_contents, '(stk%s' % num):
		num+=1

	for i, line in enumerate(template_contents):
		template_contents[i] = line.replace('(stk','(stk%s' % num)

	defs_found=False
	for line in inputfile_contents:
		if "</defs>" in line:
			defs_found=True
			merge_defs()
		if line == "</canvas>\n":
			if not defs_found:
				inputfile_f.write("<defs>\n")
				merge_defs()
				inputfile_f.write("</defs>\n")
			canvas=False
			for l in template_contents:
				if canvas:
					if not l == "</canvas>\n":
						inputfile_f.write(l)
					else:
						break
				elif "</defs>" in l:
					canvas=True
		inputfile_f.write(line)

	inputfile_f.close()

if len(sys.argv) < 2:
	sys.exit()
else:
	process(sys.argv[1])
	

