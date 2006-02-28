/* === S Y N F I G ========================================================= */
/*!	\file widget_canvaschooser.cpp
**	\brief Template File
**
**	$Id: widget_canvaschooser.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "widget_canvaschooser.h"
#include <gtkmm/menu.h>
#include "app.h"

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
Widget_CanvasChooser::set_parent_canvas(etl::handle<synfig::Canvas> x)
{
	assert(x);
	parent_canvas=x;
}

void
Widget_CanvasChooser::set_value_(etl::handle<synfig::Canvas> data)
{
	set_value(data);
	activate();
}

void
Widget_CanvasChooser::set_value(etl::handle<synfig::Canvas> data)
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
	App::dialog_entry(_("Choose Canvas"),_("Enter the relative name of the canvas that you want"),canvas_name);
	Canvas::Handle new_canvas;
	try
	{
		new_canvas=parent_canvas->find_canvas(canvas_name);
		set_value_(new_canvas);
	}
	catch(std::runtime_error x)
	{
		App::dialog_error_blocking(_("Error:Exception Thrown"),x.what());
		set_value_(canvas);		
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("Unknown Exception"));
		set_value_(canvas);
	}
}
