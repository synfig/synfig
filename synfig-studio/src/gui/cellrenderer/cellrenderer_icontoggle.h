/* === S Y N F I G ========================================================= */
/*!	\file cellrenderer_icontoggle.h
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_STUDIO_CELLRENDERER_ICONTOGGLE_H
#define SYNFIG_STUDIO_CELLRENDERER_ICONTOGGLE_H

/* === H E A D E R S ======================================================= */

#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>

#include <gtkmm/cellrenderertoggle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CellRenderer_IconToggle : public Gtk::CellRendererToggle
{
	Glib::Property<std::string> property_active_icon_name_;
	Glib::Property<std::string> property_inactive_icon_name_;
	Glib::Property<std::string> property_inconsistent_icon_name_;

public:
	CellRenderer_IconToggle();
	~CellRenderer_IconToggle();

	Glib::PropertyProxy<std::string> property_active_icon_name() { return property_active_icon_name_.get_proxy(); }
	Glib::PropertyProxy<std::string> property_inactive_icon_name() { return property_inactive_icon_name_.get_proxy(); }
	Glib::PropertyProxy<std::string> property_inconsistent_icon_name() { return property_inconsistent_icon_name_.get_proxy(); }

protected:

	virtual void
	render_vfunc(
		const ::Cairo::RefPtr< ::Cairo::Context>& cr,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState flags);

}; // END of class CellRenderer_IconToggle

}; // END of namespace studio

/* === E N D =============================================================== */

#endif // SYNFIG_STUDIO_CELLRENDERER_ICONTOGGLE_H
