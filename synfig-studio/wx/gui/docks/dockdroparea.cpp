/* === S Y N F I G ========================================================= */
/*!	\file dockdroparea.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "app.h"
#include "docks/dockdroparea.h"
#include "docks/dockmanager.h"
#include <gtkmm/button.h>

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

DockDropArea::DockDropArea(Gtk::Widget *target):
	Gtk::Table(3, 3, true),
	target(target)
{
	std::vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back( Gtk::TargetEntry("SYNFIG_DOCK") );

	Gtk::Button *button_left   = manage(new Gtk::Button());
	Gtk::Button *button_right  = manage(new Gtk::Button());
	Gtk::Button *button_top    = manage(new Gtk::Button());
	Gtk::Button *button_bottom = manage(new Gtk::Button());

	button_left->drag_dest_set(listTargets);
	button_right->drag_dest_set(listTargets);
	button_top->drag_dest_set(listTargets);
	button_bottom->drag_dest_set(listTargets);

	button_left->signal_drag_data_received().connect(
			sigc::mem_fun(*this,&DockDropArea::drop_on_left));
	button_right->signal_drag_data_received().connect(
			sigc::mem_fun(*this,&DockDropArea::drop_on_right));
	button_top->signal_drag_data_received().connect(
			sigc::mem_fun(*this,&DockDropArea::drop_on_top));
	button_bottom->signal_drag_data_received().connect(
			sigc::mem_fun(*this,&DockDropArea::drop_on_bottom));

	attach(*button_left,   0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
	attach(*button_right,  2, 3, 1, 2, Gtk::FILL, Gtk::FILL);
	attach(*button_top,    1, 2, 0, 1, Gtk::FILL, Gtk::FILL);
	attach(*button_bottom, 1, 2, 2, 3, Gtk::FILL, Gtk::FILL);
	show_all_children();
}

void
DockDropArea::drop_on(bool vertical, bool first, const Glib::RefPtr<Gdk::DragContext>& context, const Gtk::SelectionData& selection_data, guint time)
{
	if (target != NULL && (selection_data.get_length() >= 0) && (selection_data.get_format() == 8))
	{
		Dockable& dockable(**reinterpret_cast<Dockable**>(const_cast<guint8*>(selection_data.get_data())));
		if (DockManager::add_dockable(*target, dockable, vertical, first))
		{
			context->drag_finish(true, false, time);
			App::dock_manager->update_window_titles();
			return;
		}
	}
	context->drag_finish(false, false, time);
}

void
DockDropArea::drop_on_left(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	drop_on(false, true, context, selection_data, time);
}

void
DockDropArea::drop_on_right(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	drop_on(false, false, context, selection_data, time);
}

void
DockDropArea::drop_on_top(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	drop_on(true, true, context, selection_data, time);
}

void
DockDropArea::drop_on_bottom(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
	drop_on(true, false, context, selection_data, time);
}
