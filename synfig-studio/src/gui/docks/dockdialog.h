/* === S Y N F I G ========================================================= */
/*!	\file docks/dockdialog.h
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

#ifndef __SYNFIG_STUDIO_DOCK_DIALOG_H
#define __SYNFIG_STUDIO_DOCK_DIALOG_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/stockid.h>
#include <gtkmm/button.h>
#include "dialogsettings.h"
#include <synfig/string.h>
#include <gtkmm/dialog.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/accelgroup.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class Box; class Paned;  };
namespace studio {

class DockManager;
class DockBook;
class Dockable;
class Widget_CompSelect;
class CanvasView;

class DockDialog : public Gtk::Window
{
	friend class DockManager;
	friend class DockBook;
	friend class Dockable;
	sigc::connection empty_sig;

	bool composition_selector_;

	bool is_deleting;

	bool is_horizontal;

private:
	std::list<DockBook*> dock_book_list;

	std::vector<Gtk::Paned*>	panels_;
	std::vector<int>			dock_book_sizes_;


	DockBook* last_dock_book;

	Widget_CompSelect* widget_comp_select;
	Gtk::Box *box;

	int id_;

	void on_hide();

	void refresh();

	void refresh_title();

	void set_id(int x) { id_=x; }

	void refresh_accel_group();

	void drop_on_append(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
	void drop_on_prepend(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);

	//! Keyboard event dispatcher following window priority
	bool on_key_press_event(GdkEventKey* event);

	bool focused_widget_has_priority(Gtk::Widget* focused);

public:

	const std::vector<int>&	get_dock_book_sizes()const { return dock_book_sizes_;}
	void set_dock_book_sizes(const std::vector<int>&);
	void rebuild_sizes();

	bool close();

	int get_id()const { return id_; }

	DockBook* append_dock_book();
	DockBook* prepend_dock_book();
	void erase_dock_book(DockBook*);

	void set_composition_selector(bool x);
	bool get_composition_selector()const { return composition_selector_; }

	DockDialog();
	~DockDialog();

	DockBook& get_dock_book();
	const DockBook& get_dock_book()const;

	synfig::String get_contents()const;
	void set_contents(const synfig::String& x);
}; // END of studio::DockDialog

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
