# -*- coding: utf-8 -*-

# ====================== S Y N F I G =============================================
# 	File: main.py
#	Description: Synfig Studio Entrypoint
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
import wx

#import pch
#import config

from app import App
#import ipc

from general import *

if sys.platform == 'win32':
	import locale


# ============== E N T R Y P O I N T =====================================


if __name__ == '__main__':

	argc = len(sys.argv)

	binary_path = os.path.dirname(os.path.abspath(sys.argv[0])) #C:\Program Files\Synfig\bin
	#print ("  " + "binary_path: " + binary_path)

	#                                C:\Program Files\Synfig           \\          share        \\          locale
	#locale_dir = etl::dirname(etl::dirname(binary_path))+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"locale";
	locale_dir = os.path.dirname(os.path.dirname(binary_path)) + os.path.sep + "share" + os.path.sep + "locale"
	#print ("  " + "locale_dir: " + locale_dir)
	if sys.platform == 'win32':
		locale_dir = locale_from_utf8(locale_dir)
	
	#locale.setlocale(locale.LC_ALL, "")
	#gettext.bindtextdomain(GETTEXT_PACKAGE,  locale_dir)
	#gettext.bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8")
	#gettext.textdomain(GETTEXT_PACKAGE)

	
	# {
	# 	SmartFILE file(IPC::make_connection());
	# 	if(file)
	# 	{
	# 		print("\n")
	# 		print("   " + _("synfig studio is already running") + "\n\n")
	# 		print("   " + _("the existing process will be used") + "\n\n")
	# 		#// Hey, another copy of us is open!
	# 		#// don't bother opening us, just go ahead and
	# 		#// tell the other copy to load it all up
	# 		if (argc > 1)
	# 			fprintf(file.get(),"F\n");
	# 		while(--argc)
	# 			if((argv)[argc] && (argv)[argc][0]!='-')
	# 				fprintf(file.get(),"O %s\n",etl::absolute_path((argv)[argc]).c_str());
	# 		fprintf(file.get(),"F\n");
	# 		return 0;
	# 	}
	# }
	
	print("\n")
	print("   " + _("synfig studio -- starting up application...") + "\n\n")

	try:
		#studio::App app(etl::dirname(binary_path), &argc, &argv);
		app = wx.App(False)
		App(os.path.dirname(binary_path), argc, sys.argv)
		app.MainLoop()


	except Exception, e:
		print("Exception: " + str(e) + "\n")