/* === S Y N F I G ========================================================= */
/*!	\file onemoment.cpp
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

#include <synfig/general.h>

#include <gtkmm/image.h>
#include <gdkmm/pixbufloader.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/fixed.h>

#include "onemoment.h"
#include "app.h"

#include <gui/localization.h>

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

/* === M E T H O D S ======================================================= */

Gtk::Window *OneMoment::wnd = NULL;

OneMoment::OneMoment()//:
	//Gtk::Window(Gtk::WINDOW_TOPLEVEL)
{

	if (wnd) {
		wnd->show();
		return;
	}

	wnd = new Gtk::Window(Gtk::WINDOW_TOPLEVEL);

	// Create the Label
	Gtk::Label *label = manage(new class Gtk::Label(_("One Moment, Please...")));

	wnd->set_title(_("Working..."));
	wnd->set_modal(true);
	wnd->set_decorated(0);
	wnd->property_window_position().set_value(Gtk::WIN_POS_CENTER);
	wnd->set_resizable(false);
	wnd->add(*label);

	Pango::AttrList attr_list;
	Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*16));
	pango_size.set_start_index(0);
	pango_size.set_end_index(64);
	attr_list.change(pango_size);

	label->set_attributes(attr_list);

	label->set_size_request(400,60);

	wnd->set_transient_for((Gtk::Window&)(*App::main_window));
	
	// show everything off
	wnd->show_all();

	wnd->present();
	App::process_all_events();
}

OneMoment::~OneMoment()
{
	wnd->hide();
}
