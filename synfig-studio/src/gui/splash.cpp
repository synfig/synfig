/* === S Y N F I G ========================================================= */
/*!	\file splash.cpp
**	\brief writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2008 Paul Wise
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

#include <synfig/general.h>

#include <iostream>
#include <string>

#include <ETL/stringf>

#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/fixed.h>
#include <gdkmm/rgba.h>

#include "splash.h"
#include "app.h"

#include <gui/localization.h>
#include "gui/resourcehelper.h"

#endif

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef VERSION
#define VERSION	"unknown"
#define PACKAGE	"synfigstudio"
#endif

#ifndef IMAGE_EXT
#	define IMAGE_EXT	"png"
#endif

/* === G L O B A L S ======================================================= */

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

		synfig::info(task);

		studio::App::process_all_events();
		return true;
	}

	virtual bool error(const std::string &task)
	{
		if(splash.tasklabel)
		{
			splash.tasklabel->set_label(_("ERROR:")+task);
			splash.tasklabel->show();
		}

		synfig::error(task);

		studio::App::process_all_events();
		return true;
	}

	virtual bool warning(const std::string &task)
	{
		if(splash.tasklabel)
		{
			splash.tasklabel->set_label(_("WARNING:")+task);
			splash.tasklabel->show();
		}

		synfig::warning(task);

		studio::App::process_all_events();
		return true;
	}

	virtual bool amount_complete(int current, int total)
	{
		if(splash.progressbar)
		{
			splash.progressbar->set_fraction((float)current/(float)total);
			splash.progressbar->show();
		}

		studio::App::process_all_events();
		return true;
	}
}; // END of class SplashProgress

/* === M E T H O D S ======================================================= */

Splash::Splash():
	Gtk::Window(Gtk::WINDOW_TOPLEVEL)
{
	std::string imagepath = ResourceHelper::get_image_path() + '/';

	// Create the splash image
	Gtk::Image* splash_image = manage(new class Gtk::Image());
	/* Dual-splash code:
	srand(time(NULL));
	const float ran = rand()/float(RAND_MAX);
	int number = 1;
	if(ran >0.499999)
		number = 2;
	//synfig::info("%s", strprintf("%d",number).c_str());
	splash_image->set(imagepath+"splash_screen"+strprintf("%d",number)+"." IMAGE_EXT);
	*/
	splash_image->set(imagepath + "splash_screen." IMAGE_EXT);
	splash_image->set_alignment(0.5,0.5);
	splash_image->set_padding(0,0);

	// Get the image size
	int image_w = 350; int image_h = 0;
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = splash_image->get_pixbuf();
	if( pixbuf ){
		image_w = pixbuf->get_width();
		image_h = pixbuf->get_height();
	}

	// Create the progress bar
	progressbar = manage(new class Gtk::ProgressBar());
	progressbar->set_size_request(image_w,24);

	// Create the current task label
	tasklabel = manage(new class Gtk::Label());
	tasklabel->set_size_request(image_w,24);
	tasklabel->set_use_underline(false);

	// Create the current task label
	versionlabel = manage(new class Gtk::Label());
	versionlabel->set_label("" VERSION);
	versionlabel->set_size_request(image_w,24);
	versionlabel->set_use_underline(false);
	versionlabel->override_color(Gdk::RGBA("#FFFFFF"));
	versionlabel->show();

	// Create the Gtk::Fixed container and put all of the widgets into it
	Gtk::Fixed* fixed = manage(new class Gtk::Fixed());
	if( pixbuf ) fixed->put(*splash_image, 0, 0);
	fixed->put(*progressbar, 0, image_h+24);
	fixed->put(*tasklabel, 0, image_h);
	fixed->put(*versionlabel, 0, image_h-24);

	// Create shadow around the outside of the window
	Gtk::Frame* frame = manage(new class Gtk::Frame());
	frame->set_shadow_type(Gtk::SHADOW_OUT);
  	frame->add(*fixed);

	// Set up the parameters for this pop-up window
	set_title("Synfig Studio " VERSION);
	set_modal(false);
	property_window_position().set_value(Gtk::WIN_POS_CENTER);
	set_resizable(false);
	set_type_hint(Gdk::WINDOW_TYPE_HINT_SPLASHSCREEN);
	set_auto_startup_notification(false);
	try {
		set_icon_from_file(imagepath+"synfig_icon."+IMAGE_EXT);
	} catch(...) {
		synfig::warning("Unable to open "+imagepath+"synfig_icon."+IMAGE_EXT);
	}
	add(*frame);

	// show everything off
	if( pixbuf ) splash_image->show();
	fixed->show();
	frame->show();

	// Once the splash is shown, we want startup stuff to continue as normal
	signal_map().connect(sigc::mem_fun(*this, &Splash::enable_startup_notification));

	cb=new SplashProgress(*this);
}

Splash::~Splash()
{
	delete cb;
}

synfig::ProgressCallback *
Splash::get_callback()
{
	return cb;
}

void
Splash::enable_startup_notification(){
	set_auto_startup_notification(true);
}
