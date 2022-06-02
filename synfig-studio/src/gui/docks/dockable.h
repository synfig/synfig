/* === S Y N F I G ========================================================= */
/*!	\file docks/dockable.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_STUDIO_DOCKABLE_H
#define __SYNFIG_STUDIO_DOCKABLE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/eventbox.h>
#include <gtkmm/grid.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>

#include <synfig/string.h>

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

	sigc::signal<void> signal_icon_changed_;

	synfig::String name_;
	synfig::String local_name_;

	std::string icon_name;
	bool use_scrolled;

	Gtk::ScrolledWindow *container;
	Gtk::EventBox *toolbar_container;
	bool dnd_success_;

public:
	sigc::signal<void>& signal_icon_changed() { return signal_icon_changed_; }

public:
	Dockable(const synfig::String& name, const synfig::String& local_name, std::string icon_name_ = "");
	~Dockable();

	const synfig::String& get_name()const { return name_; }
	
	const synfig::String& get_local_name()const { return local_name_; }
	void set_local_name(const synfig::String&);

	bool get_use_scrolled() const;
	void set_use_scrolled(bool x);

	const std::string& get_icon()const { return icon_name; }
	void set_icon(std::string name) { icon_name = name; signal_icon_changed()(); }

	void add(Gtk::Widget& x);
	void set_toolbar(Gtk::Toolbar& toolbar);
	Gtk::ToolButton* add_button(const std::string& icon_name, const synfig::String& tooltip = synfig::String());

	void reset_container();
	void reset_toolbar();
	void clear();

	Gtk::ScrolledWindow* get_container() const
		{ return container; }

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
