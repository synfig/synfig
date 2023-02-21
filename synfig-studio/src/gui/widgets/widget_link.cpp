/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_link.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2014 Jérôme Blanchi
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

#include <synfig/general.h>

#include "widgets/widget_link.h"

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Widget_Link::Widget_Link()
	: ObjectBase    ("widget_link")
	, value_active  (*this, "tooltip_active"  , "Unlink")
	, value_inactive(*this, "tooltip_inactive", "Link")
{
	init();
}

Widget_Link::Widget_Link(const std::string &tlt_inactive, const std::string &tlt_active)
	: ObjectBase    ("widget_link")
	, value_active  (*this, "tooltip_active"  , tlt_active)
	, value_inactive(*this, "tooltip_inactive", tlt_inactive)
{
	init();
}

Widget_Link::Widget_Link(BaseObjectType *cobject)
	: ObjectBase    ("widget_link")
	, ToggleButton  (cobject)
	, value_active  (*this, "tooltip_active"  , "Unlink")
	, value_inactive(*this, "tooltip_inactive", "Link")
{
	init();
}

Widget_Link::Widget_Link(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder)
	: ObjectBase    ("widget_link")
	, ToggleButton  (cobject)
	, builder       (builder)
	, value_active  (*this, "tooltip_active"  , "Unlink")
	, value_inactive(*this, "tooltip_inactive", "Link")
{
	init();
}

Widget_Link::~Widget_Link() = default;

void
Widget_Link::init()
{
	on_toggled();
}


void
Widget_Link::on_toggled()
{
	// refresh icon & tool-tip text
	if (get_active()) {
		set_image_from_icon_name("utils_chain_link_on_icon", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
		set_tooltip_text(property_tooltip_active().get_value());
	} else {
		set_image_from_icon_name("utils_chain_link_off_icon", Gtk::BuiltinIconSize::ICON_SIZE_BUTTON);
		set_tooltip_text(property_tooltip_inactive().get_value());
	}
}

GType Widget_Link::gtype = 0;

Glib::ObjectBase
*Widget_Link::wrap_new(GObject *o)
{
	if (gtk_widget_is_toplevel(GTK_WIDGET(o)))
		return new Widget_Link(GTK_TOGGLE_BUTTON(o));
	else
		return manage(new Widget_Link(GTK_TOGGLE_BUTTON(o)));
}

void
Widget_Link::register_type()
{
	if(gtype)
		return;

	Widget_Link dummy;

	gtype = G_OBJECT_TYPE(dummy.gobj());

	wrap_register(gtype, Widget_Link::wrap_new);
}
