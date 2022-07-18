/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_icontoggle.cpp
**	\brief A CellRendererToggle with custom pixbufs for checkbox state
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**                2022      Synfig contributors
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
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include "cellrenderer_icontoggle.h"

# include <gtkmm/icontheme.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

CellRenderer_IconToggle::CellRenderer_IconToggle()
	: Glib::ObjectBase(typeid(CellRenderer_IconToggle)),
	  Gtk::CellRendererToggle(),
	  property_active_icon_name_(*this, "active_icon_name"),
	  property_inactive_icon_name_(*this, "inactive_icon_name"),
	  property_inconsistent_icon_name_(*this, "inconsistent_icon_name")
{
}

CellRenderer_IconToggle::~CellRenderer_IconToggle()
{
}

void
CellRenderer_IconToggle::render_vfunc(
	const ::Cairo::RefPtr< ::Cairo::Context>& cr,
	Gtk::Widget& widget,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	if (!cr || background_area.has_zero_area() || cell_area.has_zero_area())
		return;

	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();
	std::string icon_name;
	if (property_inconsistent())
		icon_name = property_inconsistent_icon_name_;
	else if (get_active())
		icon_name = property_active_icon_name_;
	else
		icon_name = property_inactive_icon_name_;

	if (!icon_name.empty())
		pixbuf = icon_theme->load_icon(icon_name, cell_area.get_height());

	if (!pixbuf) {
		Gtk::CellRendererToggle::render_vfunc(cr, widget, background_area, cell_area, flags);
	} else {
		gdk_cairo_set_source_pixbuf(cr->cobj(), pixbuf->gobj(), cell_area.get_x(), cell_area.get_y());
		cr->paint();
	}
}
