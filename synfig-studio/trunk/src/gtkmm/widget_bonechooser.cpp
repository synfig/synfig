/* === S Y N F I G ========================================================= */
/*!	\file widget_bonechooser.cpp
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

#include "widget_bonechooser.h"
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

#if 0
Widget_BoneChooser::Widget_BoneChooser()
{
}

Widget_BoneChooser::~Widget_BoneChooser()
{
}

void
Widget_BoneChooser::set_parent_bone(synfig::Bone::Handle x)
{
	assert(x);
	parent_bone=x;
}

void
Widget_BoneChooser::set_value_(synfig::Bone::Handle data)
{
	set_value(data);
	activate();
}

void
Widget_BoneChooser::set_value(synfig::Bone::Handle data)
{
	assert(parent_bone);
	bone=data;

	bone_menu=manage(new class Gtk::Menu());

	synfig::Bone::Children::iterator iter;
	synfig::Bone::Children &children(parent_bone->children());
	String label;

	if(bone)
	{
		label=bone->get_name().empty()?bone->get_id():bone->get_name();
		bone_menu->items().push_back(Gtk::Menu_Helpers::MenuElem(label));
	}

	for(iter=children.begin();iter!=children.end();iter++)
		if(*iter!=bone)
		{
			label=(*iter)->get_name().empty()?(*iter)->get_id():(*iter)->get_name();
			bone_menu->items().push_back(
				Gtk::Menu_Helpers::MenuElem(
					label,
					sigc::bind(
						sigc::mem_fun(
							*this,
							&Widget_BoneChooser::set_value_
						),
						*iter
					)
				)
			);
		}
	bone_menu->items().push_back(
		Gtk::Menu_Helpers::MenuElem(
			_("Other..."),
			sigc::mem_fun(*this,&Widget_BoneChooser::chooser_menu)
		)
	);
	set_menu(*bone_menu);

	if(bone)
		set_history(0);
}

const etl::handle<synfig::Bone> &
Widget_BoneChooser::get_value()
{
	return bone;
}

void
Widget_BoneChooser::chooser_menu()
{
	String bone_name;

	if (!App::dialog_entry(_("Choose Bone"),_("Enter the relative name of the bone that you want"),bone_name))
	{
		// the user hit 'cancel', so set the parameter back to its previous value
		set_value_(bone);
		return;
	}

	if (bone_name == "")
	{
		App::dialog_error_blocking(_("Error"),_("No bone name was specified"));
		set_value_(bone);
		return;
	}

	Bone::Handle new_bone;
	try
	{
		String warnings;
		new_bone=parent_bone->find_bone(bone_name, warnings);
		set_value_(new_bone);
	}
	catch(std::runtime_error x)
	{
		App::dialog_error_blocking(_("Error:Exception Thrown"),String(_("Error selecting bone:\n\n")) + x.what());
		set_value_(bone);
	}
	catch(...)
	{
		App::dialog_error_blocking(_("Error"),_("Unknown Exception"));
		set_value_(bone);
	}
}
#endif
