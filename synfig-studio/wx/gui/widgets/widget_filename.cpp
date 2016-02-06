/* === S Y N F I G ========================================================= */
/*!	\file widget_filename.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include "widgets/widget_filename.h"
#include "app.h"
#include "canvasview.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
//using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Filename::Widget_Filename()
{
	entry_filename=manage(new Gtk::Entry());
	label_find= manage(new Gtk::Label(" . . . "));
	button_choose=manage(new Gtk::Button());
	Pango::AttrList attr_list;
	{
		Pango::AttrInt pango_size(Pango::Attribute::create_attr_size(Pango::SCALE*7));
		pango_size.set_start_index(0);
		pango_size.set_end_index(64);
		attr_list.change(pango_size);
	}
	label_find->set_attributes(attr_list);
	label_find->set_ellipsize(Pango::ELLIPSIZE_END);
	button_choose->add(*label_find);

	set_hexpand(true);
	entry_filename->set_hexpand(true);

	add(*button_choose);
	add(*entry_filename);

	entry_filename->show();
	button_choose->show();
	label_find->show();

	button_choose->signal_clicked().connect(sigc::mem_fun(*this, &studio::Widget_Filename::on_button_choose_pressed));
	//entry_filename->signal_value_changed().connect(sigc::mem_fun(*this, &studio::Widget_Filename::on_value_changed));
	entry_filename->signal_activate().connect(sigc::mem_fun(*this, &studio::Widget_Filename::on_value_changed));
}

Widget_Filename::~Widget_Filename()
{
}

void
Widget_Filename::set_has_frame(bool x)
{
	entry_filename->set_has_frame(x);
}


void
Widget_Filename::set_value(const std::string &data)
{
	entry_filename->set_text(data);
}

string
Widget_Filename::get_value() const
{
	try
	{
		return entry_filename->get_text();
	}
	catch(...)
	{
		throw string("Caught unknown exception");
	}
}

void
Widget_Filename::on_value_changed()
{
	signal_value_changed()();
}

void
Widget_Filename::on_button_choose_pressed()
{
	string filename=entry_filename->get_text();
	if(filename.empty())
		filename=".";
	else
		filename = etl::absolute_path(
			etl::dirname(App::get_selected_canvas_view()->get_canvas()->get_file_name()) +
			ETL_DIRECTORY_SEPARATOR +
			filename);

        synfig::Layer::Handle layer(App::get_selected_canvas_view()->get_selection_manager()->get_selected_layer());

	// Sound layer
	if (layer->get_name() == "sound")
	{
		if(App::dialog_open_file_audio(_("Please choose an audio file"), filename, MISC_DIR_PREFERENCE))
			entry_filename->set_text((filename));
	}

	// Import Image layer
	else if (layer->get_name() == "import")
	{
		if(App::dialog_open_file_image(_("Please choose an image file"), filename, MISC_DIR_PREFERENCE))
			entry_filename->set_text((filename));
	}

	else
	{
		if(App::dialog_open_file(_("Please choose a file"), filename, MISC_DIR_PREFERENCE))
			entry_filename->set_text(filename);
	}
}

