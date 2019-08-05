/* === S Y N F I G ========================================================= */
/*!	\file docks/dockbook.h
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

#ifndef __SYNFIG_STUDIO_DOCKBOOK_H
#define __SYNFIG_STUDIO_DOCKBOOK_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/notebook.h>
#include <synfig/string.h>
#include <gtkmm/tooltip.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class DockManager;
class Dockable;
class DockDropArea;

class DockBook : public Gtk::Notebook
{
	friend class DockManager;
	friend class Dockable;

	sigc::signal<void> signal_empty_;
	sigc::signal<void> signal_changed_;

	bool deleting_;

public:
	bool allow_empty;

	DockBook();
	~DockBook();

	sigc::signal<void>& signal_empty() { return signal_empty_; }
	sigc::signal<void>& signal_changed() { return signal_changed_; }

	using Gtk::Container::add;
	void add(Dockable& dockable, int position=-1);
	void remove(Dockable& dockable);

	void present();

	void clear();

	synfig::String get_local_contents()const;

	synfig::String get_contents()const;
	void set_contents(const synfig::String& x);

	void refresh_tabs_headers();

	void refresh_tab(Dockable*);

	bool tab_button_pressed(GdkEventButton* event, Dockable* dockable);
	void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	//! Override the default handler of the signal Gtk::Notebook::signal_switch_page().
	//! to do some extra work in case of CanvasView Dockable type
	/*! \see App::set_selected_canvas_view */
	void on_switch_page(Gtk::Widget* page, guint page_num);

	void set_dock_area_visibility(bool visible, DockBook * source);

protected:
	DockDropArea *dock_area;
}; // END of studio::DockBook

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
