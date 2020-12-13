/* === S Y N F I G ========================================================= */
/*!	\file docks/dockable.h
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

#ifndef __SYNFIG_STUDIO_DOCKABLE_H
#define __SYNFIG_STUDIO_DOCKABLE_H

/* === H E A D E R S ======================================================= */

#include "dialogsettings.h"

#include <synfig/string.h>

#include <gtkmm/stockid.h>
#include <gtkmm/grid.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class DockManager;
class DockBook;

class Dockable : public Gtk::Grid
{
private:
	friend class DockManager;
	friend class DockBook;

	sigc::signal<void> signal_stock_id_changed_;

	synfig::String name_;
	synfig::String local_name_;

	Gtk::StockID stock_id_;
	bool use_scrolled;

	Gtk::ScrolledWindow *container;
	Gtk::EventBox *toolbar_container;
	bool dnd_success_;

public:
	sigc::signal<void>& signal_stock_id_changed() { return signal_stock_id_changed_; }

public:
	Dockable(const synfig::String& name,const synfig::String& local_name, Gtk::StockID stock_id = Gtk::StockID(" "));
	~Dockable();

	const synfig::String& get_name()const { return name_; }
	
	const synfig::String& get_local_name()const { return local_name_; }
	void set_local_name(const synfig::String&);

	bool get_use_scrolled() const;
	void set_use_scrolled(bool x);

	const Gtk::StockID& get_stock_id()const { return stock_id_; }
	void set_stock_id(Gtk::StockID x) { stock_id_=x; signal_stock_id_changed()(); }

	void add(Gtk::Widget& x);
	void set_toolbar(Gtk::Toolbar& toolbar);
	Gtk::ToolButton* add_button(const Gtk::StockID& stock_id, const synfig::String& tooltip = synfig::String());

	void reset_container();
	void reset_toolbar();
	void clear();

	void attach_dnd_to(Gtk::Widget& widget);
	void detach();
	void detach_to_pointer();
	virtual void present();
	virtual Gtk::Widget* create_tab_label();

	/// Appends serialized extra layout info of this dockable
	/// \param params[out] serialized data. It must not have ']' or '|' characters.
	virtual void write_layout_string(std::string &params) const;
	virtual void read_layout_string(const std::string &params) const;

private:
	void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint info, guint time);
	void on_drag_end(const Glib::RefPtr<Gdk::DragContext>&context);
	void on_drag_begin(const Glib::RefPtr<Gdk::DragContext>&context);
	void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
}; // END of studio::Dockable

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
