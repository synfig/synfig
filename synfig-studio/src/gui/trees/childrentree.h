/* === S Y N F I G ========================================================= */
/*!	\file trees/childrentree.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#ifndef __SYNFIG_STUDIO_CHILDRENTREE_H
#define __SYNFIG_STUDIO_CHILDRENTREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/tooltip.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtkmm/button.h>

#include <synfig/valuenodes/valuenode_animated.h>

#include <synfigapp/value_desc.h>
#include <synfigapp/canvasinterface.h>

#include <gui/timemodel.h>
#include <widgets/widget_value.h>

#include "childrentreestore.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class CellRenderer_TimeTrack;
class CellRenderer_ValueBase;

class ChildrenTree : public Gtk::Table
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef studio::ColumnID ColumnID;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	ChildrenTreeStore::Model model;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Gtk::TreeView tree_view;

	Gtk::HBox *hbox;

	Glib::RefPtr<ChildrenTreeStore> children_tree_store_;

	CellRenderer_TimeTrack *cellrenderer_time_track;

	Gtk::TreeView::Column* column_time_track;

	CellRenderer_ValueBase *cellrenderer_value;

	sigc::signal<void,synfigapp::ValueDesc,synfig::ValueBase> signal_edited_value_;

	sigc::signal<bool, int, Gtk::TreeRow, ColumnID> signal_user_click_;

	sigc::signal<void,synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >,int> signal_waypoint_clicked_childrentree_;

	Widget_ValueBase blend_method_widget;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_edited_value(const Glib::ustring&path_string,synfig::ValueBase value);

	void on_waypoint_clicked_childrentree(const etl::handle<synfig::Node>& node,const synfig::Time&,const synfig::Time&, const synfig::Time&,int button);

	bool on_tree_view_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip);

	bool on_tree_event(GdkEvent *event);

	void on_selection_changed();

	void on_dirty_preview();

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	Gtk::HBox& get_hbox() { return *hbox; }

	Gtk::TreeView& get_tree_view() { return tree_view; }

	Glib::RefPtr<Gtk::TreeSelection> get_selection() { return tree_view.get_selection(); }
	Glib::SignalProxy1< bool,GdkEvent* >  signal_event () { return tree_view.signal_event(); }

	ChildrenTree();
	~ChildrenTree();

	void set_model(Glib::RefPtr<ChildrenTreeStore> children_tree_store_);

	void set_time_model(const etl::handle<TimeModel> &x);

	void set_show_timetrack(bool x=true);

	//! Signal called with a value has been edited.
	sigc::signal<void,synfigapp::ValueDesc,synfig::ValueBase>& signal_edited_value() { return signal_edited_value_; }

	sigc::signal<bool,int, Gtk::TreeRow, ColumnID>& signal_user_click() { return signal_user_click_; }

	sigc::signal<void,synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >,int>& signal_waypoint_clicked_childrentree() { return signal_waypoint_clicked_childrentree_; }

	etl::handle<synfigapp::SelectionManager> get_selection_manager() { return children_tree_store_->canvas_interface()->get_selection_manager(); }

}; // END of ChildrenTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
