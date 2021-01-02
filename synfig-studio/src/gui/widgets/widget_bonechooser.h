/* === S Y N F I G ========================================================= */
/*!	\file widget_bonechooser.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_WIDGET_BONECHOOSER_H
#define __SYNFIG_STUDIO_WIDGET_BONECHOOSER_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/comboboxtext.h>
#include <synfig/canvas.h>
#include <synfig/valuenodes/valuenode_bone.h>
#include <synfigapp/value_desc.h>
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_BoneChooser : public Gtk::ComboBoxText
{
	synfig::Canvas::Handle parent_canvas;
	synfigapp::ValueDesc value_desc;
	std::vector<synfig::ValueNode_Bone::Handle> bones;

	synfig::ValueNode_Bone::Handle bone;
	void set_value_(synfig::ValueNode_Bone::Handle data);

protected:
	virtual void on_changed();

public:

	Widget_BoneChooser();
	~Widget_BoneChooser();

	void set_parent_canvas(synfig::Canvas::Handle x);
	void set_value(synfig::ValueNode_Bone::Handle data);
	const synfig::ValueNode_Bone::Handle &get_value();

	void set_value_desc(const synfigapp::ValueDesc &x) { value_desc=x; }
	const synfigapp::ValueDesc &get_value_desc() { return value_desc; }

}; // END of class Widget_BoneChooser

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
