/* === S Y N F I G ========================================================= */
/*!	\file docks/dockdroparea.h
**	\brief Template Header
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#ifndef __SYNFIG_STUDIO_DOCKDROPAREA_H
#define __SYNFIG_STUDIO_DOCKDROPAREA_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/table.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class DockDropArea : public Gtk::Table
{
protected:
	void drop_on(bool vertical, bool first, const Glib::RefPtr<Gdk::DragContext>& context, const Gtk::SelectionData& selection_data, guint time);
	void drop_on_left(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	void drop_on_right(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	void drop_on_top(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	void drop_on_bottom(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);

public:
	Gtk::Widget *target;

	DockDropArea(Gtk::Widget *target = NULL);
}; // END of studio::DockDropArea

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
