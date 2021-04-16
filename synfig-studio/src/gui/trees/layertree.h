/* === S Y N F I G ========================================================= */
/*!	\file layertree.h
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

#ifndef __SYNFIG_STUDIO_LAYERTREE_H
#define __SYNFIG_STUDIO_LAYERTREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtkmm/table.h>
#include <gtkmm/treeview.h>

#include <gui/timemodel.h>
#include <gui/trees/layerparamtreestore.h>
#include <gui/trees/layertreestore.h>
#include <gui/widgets/widget_value.h>

#include <synfigapp/value_desc.h>

/* === M A C R O S ========================================================= */

// comment this out if you don't want the params dialog to have a 'timetrack' column
// (alternatively, export SYNFIG_DISABLE_PARAMS_PANEL_TIMETRACK=1 in environment at runtime)
#define TIMETRACK_IN_PARAMS_PANEL

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace Gtk { class TreeModelSort; };

namespace studio {

class CellRenderer_TimeTrack;
class CellRenderer_ValueBase;

class LayerTree : public Gtk::Table
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef studio::ColumnID ColumnID;
/*	enum ColumnID
	{
		COLUMNID_NAME,
		COLUMNID_VALUE,
		COLUMNID_TIME_TRACK,

		COLUMNID_END			//!< \internal
	};
*/
	typedef std::list<synfig::Layer::Handle> LayerList;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	//LayerTreeStore::Model model;

	LayerTreeStore::Model layer_model;
	LayerParamTreeStore::Model param_model;

	synfig::Layer::Handle last_top_selected_layer;
	Gtk::TreePath last_top_selected_path;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:
	Gtk::TreeView layer_tree_view_;
	Gtk::TreeView param_tree_view_;

	Gtk::HBox *hbox;

	Glib::RefPtr<Gtk::Adjustment> layer_amount_adjustment_;

	Gtk::HScale *layer_amount_hscale;

	synfig::Layer::Handle quick_layer;

	Glib::RefPtr<LayerTreeStore> layer_tree_store_;

	Glib::RefPtr<LayerParamTreeStore> param_tree_store_;

	Glib::RefPtr<Gtk::TreeModelSort> sorted_layer_tree_store_;

#ifdef TIMETRACK_IN_PARAMS_PANEL
	CellRenderer_TimeTrack *cellrenderer_time_track;
#endif	// TIMETRACK_IN_PARAMS_PANEL

	Gtk::TreeView::Column* column_time_track;

	Gtk::TreeView::Column* column_z_depth;

	CellRenderer_ValueBase *cellrenderer_value;

	sigc::signal<void,synfig::Layer::Handle> signal_layer_toggle_;

	sigc::signal<void,synfigapp::ValueDesc,synfig::ValueBase> signal_edited_value_;

	sigc::signal<bool, int, Gtk::TreeRow, ColumnID> signal_layer_user_click_;
	sigc::signal<bool, GdkEventButton*> signal_no_layer_user_click_;

	sigc::signal<bool, int, Gtk::TreeRow, ColumnID> signal_param_user_click_;

	sigc::signal<void,synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >,int> signal_waypoint_clicked_layertree_;

	sigc::signal<void,int> signal_param_tree_header_height_changed_;

	bool disable_amount_changed_signal;

	Gtk::Button *button_raise;
	Gtk::Button *button_lower;
	Gtk::Button *button_duplicate;
	Gtk::Button *button_encapsulate;
	Gtk::Button *button_delete;

	Widget_ValueBase blend_method_widget;

	bool param_tree_style_changed;

	int param_tree_header_height;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	void create_layer_tree();
	void create_param_tree();
	//! Update the param_tree_view header height.
	/*! \return true if param_tree_header_height updated, else false
	*/
	bool update_param_tree_header_height();

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_edited_value(const Glib::ustring&path_string,synfig::ValueBase value);

	void on_layer_renamed(const Glib::ustring&path_string,const Glib::ustring& value);

	void on_layer_toggle(const Glib::ustring& path_string);

	void on_waypoint_clicked_layertree(const etl::handle<synfig::Node>& node, const synfig::Time&, const synfig::Time&, const synfig::Time&, int button);

	void on_waypoint_changed(synfig::Waypoint& waypoint , synfig::ValueNode::Handle value_node);

	bool on_layer_tree_event(GdkEvent *event);

	bool on_param_tree_event(GdkEvent *event);

	bool on_param_tree_view_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip);

	bool on_layer_tree_view_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip);

	void on_selection_changed();

	void on_dirty_preview();

	void on_amount_value_changed();

	void on_blend_method_changed();

	void on_param_column_label_tree_style_updated();
	bool on_param_column_label_tree_draw(const ::Cairo::RefPtr< ::Cairo::Context>& cr);

public:

	// void on_raise_pressed();

	// void on_lower_pressed();

	// void on_duplicate_pressed();

	// void on_encapsulate_pressed();

	// void on_delete_pressed();

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	Gtk::HBox& get_hbox() { return *hbox; }

	Gtk::TreeView& layer_tree_view() { return layer_tree_view_; }
	const Gtk::TreeView& layer_tree_view()const { return layer_tree_view_; }

	Gtk::TreeView& param_tree_view() { return param_tree_view_; }
	const Gtk::TreeView& param_tree_view()const { return param_tree_view_; }

	Glib::RefPtr<Gtk::TreeSelection> get_selection() { return layer_tree_view().get_selection(); }
	Glib::SignalProxy1< bool,GdkEvent* >  signal_event () { return layer_tree_view().signal_event(); }

	LayerTree();
	~LayerTree();

	void set_model(Glib::RefPtr<LayerTreeStore> layer_tree_store_);

	void set_time_model(const etl::handle<TimeModel> &x);

	void set_show_timetrack(bool x=true);

	//! Signal called when layer is toggled.
	sigc::signal<void,synfig::Layer::Handle>& signal_layer_toggle() { return signal_layer_toggle_; }

	//! Signal called with a value has been edited.
	sigc::signal<void,synfigapp::ValueDesc,synfig::ValueBase>& signal_edited_value() { return signal_edited_value_; }

	sigc::signal<bool,int, Gtk::TreeRow, ColumnID>& signal_layer_user_click() { return signal_layer_user_click_; }

	sigc::signal<bool, GdkEventButton*>& signal_no_layer_user_click() { return signal_no_layer_user_click_; }

	sigc::signal<bool,int, Gtk::TreeRow, ColumnID>& signal_param_user_click() { return signal_param_user_click_; }

	sigc::signal<void,synfigapp::ValueDesc,std::set<synfig::Waypoint,std::less<synfig::UniqueID> >,int>& signal_waypoint_clicked_layertree() { return signal_waypoint_clicked_layertree_; }

	//! Signal fired when the param treeview header height has changed. The first parameter hold the header height
	/*! \see LayerTree::update_param_tree_header_height() */
	sigc::signal<void,int>& signal_param_tree_header_height_changed() { return signal_param_tree_header_height_changed_; }

	etl::handle<synfigapp::SelectionManager> get_selection_manager() { return layer_tree_store_->canvas_interface()->get_selection_manager(); }

	void select_layer(synfig::Layer::Handle layer);
	void select_layers(const LayerList& layer_list);
	void select_all_children_layers(synfig::Layer::Handle layer);
	void select_all_children(Gtk::TreeModel::Children::iterator iter);
	LayerList get_selected_layers()const;
	synfig::Layer::Handle get_selected_layer()const;
	void clear_selected_layers();

	void expand_layer(synfig::Layer::Handle layer);
	void expand_layers(const LayerList& layer_list);
	LayerList get_expanded_layers()const;

	void select_param(const synfigapp::ValueDesc& valuedesc);

private:
	void get_expanded_layers(LayerList &list, const Gtk::TreeNodeChildren &rows)const;

	bool on_key_press_event(GdkEventKey* event);

}; // END of LayerTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
