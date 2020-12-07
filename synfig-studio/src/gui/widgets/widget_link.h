/* === S Y N F I G ========================================================= */
/*!	\file widgets/widget_link.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_WIDGET_LINK_H
#define __SYNFIG_STUDIO_WIDGET_LINK_H

/* === H E A D E R S ======================================================= */

#include <glibmm/property.h>

#include <gtkmm/builder.h>
#include <gtkmm/togglebutton.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Widget_Link: public Gtk::ToggleButton
{
	void init();

protected:
	void on_toggled() override;

public:
	Widget_Link();
	Widget_Link(const std::string &tlt_inactive, const std::string &tlt_active);
	~Widget_Link();

// Glade & GtkBuilder related
private:
	Glib::RefPtr<Gtk::Builder> builder;

	Glib::Property<std::string> value_active;
	Glib::Property<std::string> value_inactive;

	static GType gtype;

public:
	Widget_Link(BaseObjectType *cobject);
	Widget_Link(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder);

	static Glib::ObjectBase *wrap_new(GObject *o);
	static void register_type();

	Glib::PropertyProxy<std::string> property_tooltip_active()   { return value_active.get_proxy(); }
	Glib::PropertyProxy<std::string> property_tooltip_inactive() { return value_inactive.get_proxy(); }
}; // END of class Widget_Link

} // END of namespace studio

/* === E N D =============================================================== */

#endif /* WIDGET_LINK_H_ */
