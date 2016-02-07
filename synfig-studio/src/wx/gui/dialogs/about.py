# -*- coding: utf-8 -*-

# ====================== S Y N F I G =============================================
# 	File: about.py
#	Description: About Dialog implementation
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

from general import *
import sys, os

VERSION = "1.0.2"

if sys.platform[0:3] == 'win':
	IMAGE_DIR = "share\\pixmaps"
else:
	IMAGE_DIR = "/usr/local/share/pixmaps"

IMAGE_EXT = "png"


import wx
import wx.lib.agw.hyperlink as hl
from wx.lib.wordwrap import wordwrap

cwd = os.getcwd()

class AboutDialog(wx.Dialog):
	def __init__(self, parent, ID, title, pos=wx.DefaultPosition, size=(420, 350), style=wx.DEFAULT_DIALOG_STYLE):
		wx.Dialog.__init__(self,parent,ID,title,pos,size, style)

		v = wx.BoxSizer(wx.VERTICAL)
		v1 = wx.BoxSizer(wx.VERTICAL)
		v2 = wx.BoxSizer(wx.VERTICAL)
		v.Add(v1,0,wx.ALIGN_CENTRE_HORIZONTAL)
		v.Add(v2,0,wx.ALIGN_CENTRE_HORIZONTAL)
		
		# Icons
		self.SetIcon(wx.Icon(cwd + "\synfig_icon.ico"))
		png = wx.Image(cwd + "\synfig_icon." +IMAGE_EXT, wx.BITMAP_TYPE_PNG).ConvertToBitmap()
		img = wx.StaticBitmap(self, -1, png)

		# Labels
		self.program_lbl = wx.StaticText(self, -1, "",style=wx.ALIGN_CENTRE)
		self.version_lbl = wx.StaticText(self, -1, "",style=wx.ALIGN_CENTRE)
		self.comment_lbl = wx.StaticText(self, -1, "",style=wx.ALIGN_CENTRE)
		self.website_lbl = hl.HyperLinkCtrl(self, -1, "Visit the Synfig website")
		self.copyright_lbl = wx.StaticText(self, -1, "",style=wx.ALIGN_CENTRE)
		self.credits_txt = wx.TextCtrl(self, -1, "", size = (390, 110), style=wx.ALIGN_LEFT|wx.TE_MULTILINE|wx.TE_READONLY)
		self.credits_txt.Show(False)
		self.license_txt = wx.TextCtrl(self, -1, "", size = (390, 110), style=wx.ALIGN_LEFT|wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_RICH2)
		self.license_txt.Show(False)


		v1.Add((10,10), 0, wx.ALIGN_CENTRE)
		v1.Add(img, 0, wx.ALIGN_CENTRE) # Logo
		v1.Add((10,10), 0, wx.ALIGN_CENTRE)
		v1.Add(self.program_lbl, 0, wx.ALIGN_CENTRE)
		v1.Add((10,10), 0, wx.ALIGN_CENTRE)
		v2.Add(self.version_lbl, 0, wx.ALIGN_CENTRE)
		v2.Add(self.comment_lbl, 0, wx.ALIGN_CENTRE)
		v2.Add(self.website_lbl, 0, wx.ALIGN_CENTRE)
		v2.Add(self.copyright_lbl, 0, wx.ALIGN_CENTRE)
		v2.Add(self.credits_txt, 0, wx.ALIGN_BOTTOM)
		v2.Add(self.license_txt, 0, wx.ALIGN_BOTTOM)

		# Buttons
		credits_btn = wx.Button(self, -1, "Credits", (10, 290))
		license_btn = wx.Button(self, -1, "License", (105, 290))
		close_btn = wx.Button(self, -1, "Close", (310, 290))

		# Signals
		self.Bind(wx.EVT_BUTTON, self.OnCredits, credits_btn)
		self.Bind(wx.EVT_BUTTON, self.OnLicense, license_btn)
		self.Bind(wx.EVT_BUTTON, self.OnClose, close_btn)

		# Properties
		self.version = ""
		self.authors = ""
		self.artists = ""
		self.OnCreditsClicked = False
		self.OnLicenseClicked = False
		#self.SetAutoLayout(True)
		self.SetSizer(v)


	def OnCredits(self, event):
		if self.OnCreditsClicked == False:
			self.version_lbl.Show(False)
			self.comment_lbl.Show(False)
			self.website_lbl.Show(False)
			self.copyright_lbl.Show(False)
			self.license_txt.Show(False)
			self.credits_txt.Show(True)
			self.Layout()
			self.OnCreditsClicked = True
			self.OnLicenseClicked = False
		else:
			self.credits_txt.Show(False)
			self.license_txt.Show(False)
			self.version_lbl.Show(True)
			self.comment_lbl.Show(True)
			self.website_lbl.Show(True)
			self.copyright_lbl.Show(True)
			self.Layout()
			self.OnCreditsClicked = False

	def OnLicense(self, event):
		if self.OnLicenseClicked == False:
			self.version_lbl.Show(False)
			self.comment_lbl.Show(False)
			self.website_lbl.Show(False)
			self.copyright_lbl.Show(False)
			self.credits_txt.Show(False)
			self.license_txt.Show(True)
			self.Layout()
			self.OnLicenseClicked = True
			self.OnCreditsClicked = False
		else:
			self.credits_txt.Show(False)
			self.license_txt.Show(False)
			self.version_lbl.Show(True)
			self.comment_lbl.Show(True)
			self.website_lbl.Show(True)
			self.copyright_lbl.Show(True)
			self.Layout()
			self.OnLicenseClicked = False

	def OnClose(self, event):
		self.Destroy()

	def set_version(self, version):
		self.version = version
		self.version_lbl.SetLabel(self.version)

	def get_version(self):
		return self.version

	def set_comment(self, comment):
		self.comment_lbl.SetLabel(comment)

	def set_program_name(self, program_name):
		self.program_lbl.SetLabel(program_name)
		font = wx.Font(10, wx.DEFAULT, wx.NORMAL, wx.FONTWEIGHT_BOLD, False, "Arial")
		self.program_lbl.SetFont(font)

	def set_website(self, website):
		self.website_lbl.SetURL(website)

	def set_website_label(self, website_label):
		self.website_lbl.SetLabel(website_label)

	def set_copyright(self, copyright):
		self.copyright_lbl.SetLabel(copyright)
		font = wx.Font(8, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False, "Arial")
		self.copyright_lbl.SetFont(font)

	def set_license(self, license):
		self.license_txt.SetValue(license)
		font = wx.Font(6, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False, "Arial")
		self.license_txt.SetFont(font)

	def set_authors(self, authors):
		t = ""
		for i in authors:
			t = t + i + "\n\t\t\t"
		t = "\t\tCreated by " + t
		self.authors = t
		self.credits_txt.SetValue(self.authors)
		font = wx.Font(8, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False, "Arial")
		self.credits_txt.SetFont(font)

	def set_artists(self, artists):
		t = ""
		for i in artists:
			t = t + i + "\n\t\t\t"
		t = "\n\t\tArtwork by " + t
		self.artists = self.authors + t

		self.credits_txt.SetValue(self.artists)
		font = wx.Font(8, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False, "Arial")
		self.credits_txt.SetFont(font)

	def show(self):
		self.ShowModal()

class About(AboutDialog):
	def __init__(self, parent,ID=-1,title="About Synfig Studio"):
		AboutDialog.__init__(self,parent,ID,title)
		self.set_program_name("Synfig Studio")
		self.set_version(VERSION)
		self.set_comment("2D vector animation studio")
		self.set_website("http://synfig.org/")
		self.set_copyright("Copyright 2001-2013\nRobert B. Quattlebaum Jr.,\nAdrian Bentley and Synfig contributors")
		license = wordwrap("This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.\n\n"

		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		"GNU General Public License for more details.\n\n"

		"You should have received a copy of the GNU General Public License along "
		"with this program; if not, write to the Free Software Foundation, Inc., "
		"51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or visit: http://www.gnu.org/", 550, wx.ClientDC(self)) 
		self.set_license(license)

		authors = []
		authors.append("Original developers:")
		authors.append("")
		authors.append("Robert B. Quattlebaum Jr (darco)")
		authors.append("Adrian Bentley")
		authors.append("");
		authors.append("Contributors:")
		authors.append("")
		authors.append("Adrian Winchell (SnapSilverlight)")
		authors.append("Andreas Jochens")
		authors.append("Brendon Higgins")
		authors.append("Carlos López González (genete)")
		authors.append("Carlos A. Sosa Navarro")
		authors.append("Chris Moore (dooglus)")
		authors.append("Chris Norman (pixelgeek)")
		authors.append("Cyril Brulebois (KiBi)")
		authors.append("Daniel Fort")
		authors.append("Daniel Hornung (rubikcube)")
		authors.append("David Roden (Bombe)")
		authors.append("Denis Zdorovtsov (trizer)")
		authors.append("Dmitriy Pomerantsev (Atrus)")
		authors.append("Douglas Lau")
		authors.append("Evgenij Katunov")
		authors.append("Gerald Young (Yoyobuae)")
		authors.append("Gerco Ballintijn")
		authors.append("IL'dar AKHmetgaleev (AkhIL)")
		authors.append("Ivan Mahonin")
		authors.append("Jerome Blanchi (d.j.a.y.)")
		authors.append("Konstantin Dmitriev (zelgadis)")
		authors.append("Luka Pravica")
		authors.append("Nikita Kitaev (nikitakit)")
		authors.append("Martin Michlmayr (tbm)")
		authors.append("Max May (Permutatrix)")
		authors.append("Miguel Gea Milvaques (xerakko)")
		authors.append("Paul Wise (pabs)")
		authors.append("Ralf Corsepius")
		authors.append("Ramon Miranda")
		authors.append("Ray Frederikson")
		authors.append("Timo Paulssen (timonator)")
		authors.append("Yu Chen (jcome)")
		authors.append("Yue Shi Lai")
		self.set_authors(authors)

		artists = []
		artists.append("Aurore D (rore)");
		artists.append("Bertrand Grégoire (berteh)")
		artists.append("Carl-Christian Gehl (Razputin)")
		artists.append("Carlos López González (genete)")
		artists.append("Chris Norman (pixelgeek)")
		artists.append("Daniel Hornung (rubikcube)")
		artists.append("David Rylander (rylleman)")
		artists.append("Franco Iacomella (Yaco)")
		artists.append("Gerald Young (Yoyobuae)")
		artists.append("Henrique Lopes Barone")
		artists.append("Konstantin Dmitriev (zelgadis)")
		artists.append("Madeleine Crubellier (mad0)")
		artists.append("Nikolai Mamashev (solkap)")
		artists.append("Robert B. Quattlebaum Jr. (darco)")
		artists.append("Thimotee Guiet (satrip)")
		artists.append("Yu Chen (jcome)")
		self.set_artists(artists)
		
		self.CenterOnScreen()

        
	#void close(int);
	#void on_link_clicked(Gtk::AboutDialog&, const Glib::ustring &url);


	#// TRANSLATORS: change this to your name, separate multiple names with \n
	#set_translator_credits(_("translator-credits"));

	#std::string imagepath;
    #ifdef WIN32
	#imagepath=App::get_base_path()+os.path.sep+IMAGE_DIR;
    #else
	#imagepath=IMAGE_DIR;
    #endif
	#char* synfig_root=getenv("SYNFIG_ROOT");
	#if(synfig_root) {
	#	imagepath=synfig_root;
	#	imagepath+=ETL_DIRECTORY_SEPARATOR;
	#	imagepath+="share";
	#	imagepath+=ETL_DIRECTORY_SEPARATOR;
	#	imagepath+="pixmaps";
	#}
	#imagepath+=ETL_DIRECTORY_SEPARATOR;

	#Gtk::Image *Logo = manage(new class Gtk::Image());
	#Logo->set(imagepath+"synfig_icon."IMAGE_EXT);
	#set_logo(Logo->get_pixbuf());
