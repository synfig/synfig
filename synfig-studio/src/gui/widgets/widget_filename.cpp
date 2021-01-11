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

#include <gui/widgets/widget_filename.h>

#include <gtkmm/button.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/localization.h>

#include <synfig/canvasfilenaming.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Filename::Widget_Filename()
{
	entry_filename=manage(new Gtk::Entry());
	icon_browse = manage(new Gtk::Image(Gtk::StockID("synfig-open"), Gtk::ICON_SIZE_SMALL_TOOLBAR));
	button_choose=manage(new Gtk::Button());
	button_choose->add(*icon_browse);

	set_hexpand(true);
	entry_filename->set_hexpand(true);

	add(*button_choose);
	add(*entry_filename);

	entry_filename->show();
	button_choose->show();
	icon_browse->show();

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
	filename = synfig::CanvasFileNaming::make_full_filename(canvas->get_file_name(), filename);

	if(filename.empty())
		filename=".";
	else
		filename = etl::absolute_path(
			etl::dirname(App::get_selected_canvas_view()->get_canvas()->get_file_name()) +
			ETL_DIRECTORY_SEPARATOR +
			filename);

	synfig::Layer::Handle layer(App::get_selected_canvas_view()->get_selection_manager()->get_selected_layer());

	bool selected = false;

	// Sound layer
	if (layer->get_name() == "sound")
		selected = App::dialog_open_file_audio(_("Please choose an audio file"), filename, ANIMATION_DIR_PREFERENCE);
	else
	// Import Image layer
	if (layer->get_name() == "import")
		selected = App::dialog_open_file_image(_("Please choose an image file"), filename, IMAGE_DIR_PREFERENCE);
	else
		selected = App::dialog_open_file(_("Please choose a file"), filename, ANIMATION_DIR_PREFERENCE);

	if (selected)
	{
		filename = synfig::CanvasFileNaming::make_short_filename(canvas->get_file_name(), filename);
		entry_filename->set_text(filename);
		entry_filename->activate();
	}
}

