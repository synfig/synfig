/* === S Y N F I G ========================================================= */
/*!	\file splash.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include <string>

#include <ETL/stringf>

#include <gtkmm/image.h>
#include <gdkmm/pixbufloader.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/fixed.h>

#include <synfig/general.h>

#include "splash.h"
#include "app.h"

#include "general.h"

#endif

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef VERSION
#define VERSION	"unknown"
#define PACKAGE	"synfigstudio"
#endif

#ifdef WIN32
#	ifdef IMAGE_DIR
#		undef IMAGE_DIR
#		define IMAGE_DIR "share\\pixmaps"
#	endif
#endif

#ifndef IMAGE_DIR
#	define IMAGE_DIR "/usr/local/share/pixmaps"
#endif

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === G L O B A L S ======================================================= */
extern      const guint gtk_major_version;
extern      const guint gtk_minor_version;
extern      const guint gtk_micro_version;
extern      const guint gtk_binary_age;
extern      const guint gtk_interface_age;

/* === P R O C E D U R E S ================================================= */

class studio::SplashProgress : public synfig::ProgressCallback
{
	Splash &splash;

public:

	SplashProgress(Splash &splash):splash(splash) { }

	virtual bool task(const std::string &task)
	{
		if(splash.tasklabel)
		{
			splash.tasklabel->set_label(task);
			splash.tasklabel->show();
		}
		else
		{
			cerr<<task<<endl;
		}

		while(studio::App::events_pending())studio::App::iteration(false);
		return true;
	}

	virtual bool error(const std::string &task)
	{
		if(splash.tasklabel)
		{
			splash.tasklabel->set_label(_("ERROR:")+task);
			splash.tasklabel->show();
		}
		else
		{
			cerr<<task<<endl;
		}

		while(studio::App::events_pending())studio::App::iteration(false);
		return true;
	}

	virtual bool warning(const std::string &task)
	{
		if(splash.tasklabel)
		{
			splash.tasklabel->set_label(_("WARNING:")+task);
			splash.tasklabel->show();
		}
		else
		{
			cerr<<task<<endl;
		}

		while(studio::App::events_pending())studio::App::iteration(false);
		return true;
	}

	virtual bool amount_complete(int current, int total)
	{
		if(splash.progressbar)
		{
			splash.progressbar->set_fraction((float)current/(float)total);
			splash.progressbar->show();
		}
		else
			cerr<<current<<'/'<<total<<endl;

		while(studio::App::events_pending())studio::App::iteration(false);
		return true;
	}
}; // END of class SplashProgress

/* === M E T H O D S ======================================================= */

Splash::Splash():
	Gtk::Window(getenv("SYNFIG_DISABLE_POPUP_WINDOWS") ? Gtk::WINDOW_TOPLEVEL : Gtk::WINDOW_POPUP),
	can_self_destruct(true)
{
	int image_w=300,image_h=350;

	std::string imagepath;
#ifdef WIN32
	imagepath=App::get_base_path()+ETL_DIRECTORY_SEPARATOR+IMAGE_DIR;
#else
	imagepath=IMAGE_DIR;
#endif
	char* synfig_root=getenv("SYNFIG_ROOT");
	if(synfig_root) {
		imagepath=synfig_root;
		imagepath+=ETL_DIRECTORY_SEPARATOR;

		imagepath+="share/pixmaps";
	}
	imagepath+=ETL_DIRECTORY_SEPARATOR;


	// Create the Logo
	Gtk::Image *Logo = manage(new class Gtk::Image());
	Logo->set(imagepath+"splash_screen."IMAGE_EXT);
	Logo->set_size_request(image_w,image_h);
	Logo->set_alignment(0.5,0.5);
	Logo->set_padding(0,0);

	// Create the Copyright Label
	Gtk::Label *CopyrightLabel = manage(new class Gtk::Label(SYNFIG_COPYRIGHT));
	CopyrightLabel->set_size_request(image_w,24);
	CopyrightLabel->set_alignment(0.5,0.5);
	CopyrightLabel->set_padding(0,0);
	CopyrightLabel->set_justify(Gtk::JUSTIFY_CENTER);
	CopyrightLabel->set_line_wrap(false);
	CopyrightLabel->modify_fg(Gtk::STATE_NORMAL,Gdk::Color("black"));

	/* Scale the text to fit */
	int width = 0;
	int height = 0;
	float size=11;
	Glib::RefPtr<Pango::Layout> l = CopyrightLabel->get_layout();
	Pango::FontDescription fd = Pango::FontDescription("Sans, 11");
	l->set_font_description(fd);
	l->set_justify(Pango::ALIGN_CENTER);
	fd.set_size(int(size*Pango::SCALE));
	l->set_font_description(fd);
	l->get_pixel_size(width,height);
	while( width >= image_w-6 ){
		size-=0.5;
		fd.set_size((int)(size*Pango::SCALE));
		l->set_font_description(fd);
		l->get_pixel_size(width,height);
	}
	CopyrightLabel->modify_font(fd);

	// Create the Version information label
	Gtk::Label *VersionLabel = manage(new class Gtk::Label(_("Version")));
	VersionLabel->set_size_request(image_w,80);
	VersionLabel->set_flags(Gtk::CAN_FOCUS);
	VersionLabel->set_alignment(0.5,0.5);
	VersionLabel->set_padding(0,0);
	VersionLabel->set_justify(Gtk::JUSTIFY_CENTER);
	VersionLabel->set_line_wrap(false);
	VersionLabel->modify_fg(Gtk::STATE_NORMAL,Gdk::Color("black"));

	// Set the version label to contain the correct information
	string ver;
	ver+="Version "VERSION" ("__DATE__ /* " "__TIME__ */ ")\n";
	ver+="Using Synfig ";
	ver+=synfig::get_version();
	#ifdef __GNUC__
		ver+=strprintf(" and GNU G++ %d.%d.%d",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
	#endif

	ver+=strprintf("\nGtk+ %d.%d.%d",gtk_major_version,gtk_minor_version,gtk_micro_version);

	#ifdef _DEBUG
		ver+="\nDEBUG BUILD";
	#endif
	VersionLabel->set_text(ver);

	/* Scale the text to fit */
	width = 0;
	height = 0;
	size=11;
	l = VersionLabel->get_layout();
	fd = Pango::FontDescription("Sans, 11");
	l->set_font_description(fd);
	l->set_justify(Pango::ALIGN_CENTER);
	fd.set_size(int(size*Pango::SCALE));
	l->set_font_description(fd);
	l->get_pixel_size(width,height);
	while( width >= image_w-6 ){
		size-=0.5;
		fd.set_size((int)(size*Pango::SCALE));
		l->set_font_description(fd);
		l->get_pixel_size(width,height);
	}
	VersionLabel->modify_font(fd);

 	// Create the image that will be used on the close button
 	Gtk::Image *image2 = manage(new class Gtk::Image(Gtk::StockID("gtk-close"), Gtk::IconSize(4)));
	image2->set_alignment(0.5,0.5);
	image2->set_padding(0,0);

	// Create the close button, and attach the image to it
	CloseButton = manage(new class Gtk::Button());
	CloseButton->set_size_request(24,24);
	CloseButton->set_flags(Gtk::CAN_FOCUS);
	_tooltips.set_tip(*CloseButton, _("Close"), "");
	CloseButton->set_relief(Gtk::RELIEF_NONE);
	CloseButton->add(*image2);

	// Create the progress bar
	progressbar = manage(new class Gtk::ProgressBar());
	progressbar->set_size_request(image_w,24);

	// Create the current task label
	tasklabel = manage(new class Gtk::Label());
	tasklabel->set_size_request(image_w,24);
	tasklabel->set_use_underline(false);

	// Create the Gtk::Fixed container and put all of the widgets into it
	Gtk::Fixed *fixed1 = manage(new class Gtk::Fixed());
	fixed1->put(*Logo, 0, 0);
	fixed1->put(*CopyrightLabel, 0, image_h-25);
	fixed1->put(*CloseButton, image_w-24, 0);
	fixed1->put(*VersionLabel, 0, image_h-90);
	fixed1->put(*progressbar, 0, image_h+24);
	fixed1->put(*tasklabel, 0, image_h);

	// Set up the parameters for this pop-up window
	set_title("Synfig Studio "VERSION);
	set_modal(false);
	property_window_position().set_value(Gtk::WIN_POS_CENTER);
	set_resizable(false);
	add(*fixed1);

	// show everything off
	Logo->show();
	CopyrightLabel->show();
	image2->show();
	CloseButton->show();
	VersionLabel->show();
	fixed1->show();

	// Connect relevant signals
	CloseButton->signal_clicked().connect(sigc::mem_fun(*this, &Splash::close));

	cb=new SplashProgress(*this);
}

Splash::~Splash()
{
	delete cb;
}

void Splash::close()
{
	hide();
	if(can_self_destruct)
		delete this;
}

void
Splash::set_can_self_destruct(bool x)
{
	can_self_destruct=x;
	if(x==true)
		CloseButton->show();
	else
		CloseButton->hide();
}

synfig::ProgressCallback *
Splash::get_callback()
{
	return cb;
}
