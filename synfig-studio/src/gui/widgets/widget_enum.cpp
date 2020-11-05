/* === S Y N F I G ========================================================= */
/*!	\file widget_enum.cpp
**	\brief Template File
**
**	$Id$
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

#include <synfig/general.h>

#include <gtkmm/menu.h>
#include "widgets/widget_enum.h"
#include <ETL/stringf>
#include <synfig/valuenode.h>

#include <gui/localization.h>

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

Widget_Enum::Widget_Enum():
	value()
{
	set_hexpand();

	enum_TreeModel = Gtk::ListStore::create(enum_model);
	set_model(enum_TreeModel);
	pack_start(enum_model.icon, false);
	pack_start(enum_model.local_name, true);
	this->set_wrap_width(1); // https://github.com/synfig/synfig/issues/650

	Gtk::CellRendererText *text = dynamic_cast<Gtk::CellRendererText*>(this->get_cells()[1]);
	text->property_ellipsize().set_value(Pango::ELLIPSIZE_END);
}

Widget_Enum::~Widget_Enum()
{
}

void
Widget_Enum::set_param_desc(const synfig::ParamDesc &x)
{
	param_desc=x;
	// First clear the current items in the ComobBox
	enum_TreeModel->clear();
	std::list<synfig::ParamDesc::EnumData> enum_list=param_desc.get_enum_list();
	std::list<synfig::ParamDesc::EnumData>::iterator iter;
	// Fill the combo with the values
	for(iter=enum_list.begin();iter!=enum_list.end();iter++)
		{
			Gtk::TreeModel::Row row = *(enum_TreeModel->append());
			row[enum_model.value] = iter->value;
			row[enum_model.local_name] = iter->local_name;
		}
	refresh();
}

void
Widget_Enum::set_icon(Gtk::TreeNodeChildren::size_type index, const Glib::RefPtr<Gdk::Pixbuf> &icon)
{
	// check if the index is valid
	if(index > enum_TreeModel->children().size()-1 )
		return;
	Glib::ustring path(strprintf("%d", index).c_str());
	Gtk::TreeModel::Row row = *(enum_TreeModel->get_iter(path));
	row[enum_model.icon]=icon;
}

void
Widget_Enum::refresh()
{
	typedef Gtk::TreeModel::Children type_children;
	type_children children = enum_TreeModel->children();
	for(type_children::iterator iter = children.begin();
		iter != children.end(); ++iter)
	{
		Gtk::TreeModel::Row row = *iter;
		if(row[enum_model.value] == value)
		{
			set_active(iter);
			return;
		}
	}
}

void
Widget_Enum::set_value(int data)
{
	value=data;
	refresh();

}

int
Widget_Enum::get_value() const
{
	return value;
}

void Widget_Enum::on_changed()
{
	Gtk::TreeModel::iterator iter = get_active();
	if(iter)
		{
			Gtk::TreeModel::Row row = *iter;
			value = row[enum_model.value];
		}
}
