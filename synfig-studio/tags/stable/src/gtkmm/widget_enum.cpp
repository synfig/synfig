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

#include <gtkmm/menu.h>
#include "widget_enum.h"
#include <ETL/stringf>
#include <synfig/valuenode.h>

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

Widget_Enum::Widget_Enum()
{
}

Widget_Enum::~Widget_Enum()
{
}

void
Widget_Enum::set_param_desc(const synfig::ParamDesc &x)
{
	param_desc=x;
	//refresh();
}

void
Widget_Enum::set_value_(int data)
{
	set_value(data);
	activate();
}

void
Widget_Enum::refresh()
{
	enum_menu = manage(new class Gtk::Menu());

	std::list<synfig::ParamDesc::EnumData> enum_list=param_desc.get_enum_list();
	std::list<synfig::ParamDesc::EnumData>::iterator iter;

	String name=strprintf("(%d)",value);

	for(iter=enum_list.begin();iter!=enum_list.end();iter++)
		if(iter->value!=value)
			enum_menu->items().push_back(Gtk::Menu_Helpers::MenuElem(iter->local_name,
				sigc::bind(sigc::mem_fun(*this,&Widget_Enum::set_value_),iter->value)
			));
		else
			name=iter->local_name;

	enum_menu->items().push_front(Gtk::Menu_Helpers::MenuElem(name));

	set_menu(*enum_menu);
}

void
Widget_Enum::set_value(int data)
{
	value=data;

	refresh();

	set_history(0);
}

int
Widget_Enum::get_value() const
{
	return value;
}
