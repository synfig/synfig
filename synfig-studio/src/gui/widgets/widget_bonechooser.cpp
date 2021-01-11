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

#include "widgets/widget_bonechooser.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_BoneChooser::Widget_BoneChooser() { }
Widget_BoneChooser::~Widget_BoneChooser() { }

void
Widget_BoneChooser::set_parent_canvas(synfig::Canvas::Handle x)
{
	assert(x);
	parent_canvas=x;
}

void
Widget_BoneChooser::set_value_(synfig::ValueNode_Bone::Handle data)
{
	printf("%s:%d Widget_BoneChooser::set_value_ data = %lx\n", __FILE__, __LINE__, uintptr_t(data.get()));
	set_value(data);
	activate();
}

void
Widget_BoneChooser::on_changed()
{
	int i = get_active_row_number();
	if (i < 0 || i >= (int)bones.size()) return;
	if (bone != bones[i]) set_value_(bones[i]);
}

void
Widget_BoneChooser::set_value(synfig::ValueNode_Bone::Handle data)
{
	printf("%s:%d Widget_BoneChooser::set_value data = %lx\n", __FILE__, __LINE__, uintptr_t(data.get()));
	assert(parent_canvas);
	bone=data;

	Time time(parent_canvas->get_time());

	set_active(-1);
	remove_all();
	bones.clear();

	if (get_value_desc().is_value_node())
	{
		// the ValueNode is either a ValueNode_Const or a ValueNode_Animated I think
		ValueNode_Bone::BoneSet parent_set(ValueNode_Bone::get_possible_parent_bones(get_value_desc().get_value_node()));

		// insert the entry for the currently selected value first so that it appears selected
		if (!bone->is_root())
		{

			String label((*(bone->get_link("name")))(time).get(String()));
			if (label.empty()) label=bone->get_guid().get_string();

			parent_set.erase(bone); // erase it from the set so it won't be inserted twice
			bones.push_back(bone);
			append(label);
			// (*(bone->get_link("name")))(time).get(String());
		}
		else
		{
			bones.push_back(ValueNode_Bone::get_root_bone());
			append(_("<None>"));
		}

		for (ValueNode_Bone::BoneSet::iterator iter=parent_set.begin(); iter!=parent_set.end(); iter++)
		{
			ValueNode_Bone::Handle bone_value_node(*iter);

			String label((*(bone_value_node->get_link("name")))(time).get(String()));
			if (label.empty()) label=bone_value_node->get_guid().get_string();

			bones.push_back(bone_value_node);
			append(label);
		}
	}

	if (bone && !bone->is_root())
	{
		bones.push_back(ValueNode_Bone::get_root_bone());
		append(_("<None>"));
	}

	if(bone)
		set_active(0);
}

const etl::handle<synfig::ValueNode_Bone> &
Widget_BoneChooser::get_value()
{
	return bone;
}
