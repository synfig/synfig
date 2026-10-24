/* === S Y N F I G ========================================================= */
/*!	\file dock_tooloptions.cpp
**	\brief Implementation File for ToolOptions Panel
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2009 Nikita Kitaev
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "docks/dock_tooloptions.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_ToolOptions::Dock_ToolOptions()
	: Dockable("tool_options", _("Tool Options"), "about_icon")
	, empty_label_(_("This tool has no options"))
{
	add(sub_vbox_);

	sub_vbox_.set_margin_end(10);
	sub_vbox_.set_margin_bottom(10);
	
	set_widget(empty_label_);
	empty_label_.show();
}

Dock_ToolOptions::~Dock_ToolOptions()
{
}

void
Dock_ToolOptions::clear()
{
	Dockable::clear();

	set_local_name(_("Tool Options"));
	set_icon("about_icon");

	add(sub_vbox_);
	sub_vbox_.set_valign(Gtk::Align::ALIGN_CENTER);
	set_widget(empty_label_);
	empty_label_.show();
}

void
Dock_ToolOptions::set_widget(Gtk::Widget& x)
{
	std::vector<Gtk::Widget*> children = sub_vbox_.get_children();
	for (std::vector<Gtk::Widget*>::iterator i = children.begin(); i != children.end(); ++i)
		sub_vbox_.remove(**i);
	sub_vbox_.show();
	sub_vbox_.pack_start(x, false, false);
	sub_vbox_.set_valign(Gtk::Align::ALIGN_FILL);
	x.show();
}
