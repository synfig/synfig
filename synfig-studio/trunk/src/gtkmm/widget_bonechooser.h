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

#include <synfig/bone.h>
#include <gtkmm/optionmenu.h>


/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Menu; };

namespace studio {

class Widget_BoneChooser : public Gtk::OptionMenu
{
	Gtk::Menu *bone_menu;
	synfig::Bone::Handle parent_bone;

	synfig::Bone::Handle bone;
	void set_value_(synfig::Bone::Handle data);
public:

	Widget_BoneChooser();
	~Widget_BoneChooser();

	void set_parent_bone(synfig::Bone::Handle x);
	void set_value(synfig::Bone::Handle data);
	const synfig::Bone::Handle &get_value();
private:
	void chooser_menu();
}; // END of class Widget_BoneChooser

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
