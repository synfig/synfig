/* === S Y N F I G ========================================================= */
/*!	\file widget_sublayer.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2015 Max May
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

#include <gui/widgets/widget_sublayer.h>

#include <gui/localization.h>

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/layers/layer_pastecanvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_Sublayer::Widget_Sublayer():
	value()
{
	enum_TreeModel = Gtk::ListStore::create(enum_model);
	set_model(enum_TreeModel);
	pack_start(enum_model.name);
}

Widget_Sublayer::~Widget_Sublayer()
{
}

void
Widget_Sublayer::set_value_desc(const synfigapp::ValueDesc &x)
{
	value_desc=x;
	// First clear the current items in the ComboBox
	enum_TreeModel->clear();
	cout << value_desc.get_layer() << endl;
	etl::handle<synfig::Layer_PasteCanvas> p = etl::handle<synfig::Layer_PasteCanvas>::cast_dynamic(value_desc.get_layer());
	if(p)
	{
		synfig::Canvas::Handle canvas = p->get_sub_canvas();
		if(canvas)
		{
			// Fill the combo with the layers' descriptions
			Gtk::TreeModel::Row row = *(enum_TreeModel->append());
			row[enum_model.value] = "";
			row[enum_model.name] = _("<empty>");
			for(IndependentContext i = canvas->get_independent_context(); *i; i++)
			{
				Gtk::TreeModel::Row row = *(enum_TreeModel->append());
				std::string desc = (*i)->get_description();
				row[enum_model.value] = desc;
				row[enum_model.name] = desc;
			}
		}
	}
	refresh();
}

void
Widget_Sublayer::refresh()
{
	typedef Gtk::TreeModel::Children type_children;
	type_children children = enum_TreeModel->children();
	for(type_children::iterator iter = children.begin();
		iter != children.end(); ++iter)
	{
		Gtk::TreeModel::Row row = *iter;
		if(row.get_value(enum_model.value) == value)
		{
			set_active(iter);
			return;
		}
	}
}

void
Widget_Sublayer::set_value(string data)
{
	value=data;
	refresh();
}

string
Widget_Sublayer::get_value() const
{
	return value;
}

void
Widget_Sublayer::on_changed()
{
	Gtk::TreeModel::iterator iter = get_active();
	if(iter)
	{
		Gtk::TreeModel::Row row = *iter;
		value = row.get_value(enum_model.value);
	}
}
