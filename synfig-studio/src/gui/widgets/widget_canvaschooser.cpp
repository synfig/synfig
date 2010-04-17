/* === S Y N F I G ========================================================= */
/*!	\file widget_canvaschooser.cpp
**	\brief Template File
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "widgets/widget_canvaschooser.h"
#include <gtkmm/menu.h>
#include "app.h"

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_CanvasChooser::Widget_CanvasChooser()
{
}

Widget_CanvasChooser::~Widget_CanvasChooser()
{
}

void
Widget_CanvasChooser::set_parent_canvas(synfig::Canvas::Handle x)
{
	assert(x);
	parent_canvas=x;
}

void
Widget_CanvasChooser::set_value_(synfig::Canvas::Handle data)
{
	set_value(data);
	activate();
}

void
Widget_CanvasChooser::set_value(synfig::Canvas::Handle data)
{
	assert(parent_canvas);
	canvas=data;

	canvas_menu=manage(new class Gtk::Menu());

	synfig::Canvas::Children::iterator iter;
	synfig::Canvas::Children &children(parent_canvas->children());
	String label;

	if(canvas)
	{
		label=canvas->get_name().empty()?canvas->get_id():canvas->get_name();
		canvas_menu->items().push_back(Gtk::Menu_Helpers::MenuElem(label));
	}

	for(iter=children.begin();iter!=children.end();iter++)
		if(*iter!=canvas)
		{
			label=(*iter)->get_name().empty()?(*iter)->get_id():(*iter)->get_name();
			canvas_menu->items().push_back(
				Gtk::Menu_Helpers::MenuElem(
					label,
					sigc::bind(
						sigc::mem_fun(
							*this,
							&Widget_CanvasChooser::set_value_
						),
						*iter
					)
				)
			);
		}
	canvas_menu->items().push_back(
		Gtk::Menu_Helpers::MenuElem(
			_("Other..."),
			sigc::mem_fun(*this,&Widget_CanvasChooser::chooser_menu)
		)
	);
	set_menu(*canvas_menu);

	if(canvas)
		set_history(0);
}

const etl::handle<synfig::Canvas> &
Widget_CanvasChooser::get_value()
{
	return canvas;
}

void
Widget_CanvasChooser::chooser_menu()
{
	String canvas_name;

	if (!App::dialog_entry(_("Choose Canvas"),_("Enter the relative name of the canvas that you want"),canvas_name))
	{
		// the user hit 'cancel', so set the parameter back to its previous value
		set_value_(canvas);
		return;
	}

	if (canvas_name == "")
	{
		App::dialog_error_blocking(_("Error"),_("No canvas name was specified"));
		set_value_(canvas);
		return;
	}

	Canvas::Handle new_canvas;
	try
	{
		String warnings;
		new_canvas=parent_canvas->find_canvas(canvas_name, warnings);
		set_value_(new_canvas);
	}
	catch(std::runtime_error x)
	{
		App::dialog_error_blocking(_("Error:Exception Thrown"),String(_("Error selecting canvas:\n\n")) + x.what());
		set_value_(canvas);
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("Unknown Exception"));
		set_value_(canvas);
	}
}
