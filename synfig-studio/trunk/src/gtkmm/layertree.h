/* === S I N F G =========================================================== */
/*!	\file layertree.h
**	\brief Template Header
**
**	$Id: layertree.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_STUDIO_LAYERTREE_H
#define __SINFG_STUDIO_LAYERTREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/scale.h>
#include <gtkmm/button.h>

#include <sinfgapp/canvasinterface.h>
#include <sinfgapp/value_desc.h>
#include "layertreestore.h"
#include "layerparamtreestore.h"
#include <sinfg/valuenode_animated.h>

#include "widget_value.h"

/* === M A C R O S ========================================================= */

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
	typedef std::list<sinfg::Layer::Handle> LayerList;
	
	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:
	
	//LayerTreeStore::Model model;

	LayerTreeStore::Model layer_model;
	LayerParamTreeStore::Model param_model;

	sinfg::Layer::Handle last_top_selected_layer;
	Gtk::TreePath last_top_selected_path;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Gtk::Tooltips tooltips_;
	Gtk::TreePath last_tooltip_path;



	Gtk::TreeView* layer_tree_view_;

	Gtk::TreeView* param_tree_view_;

	

	Gtk::HBox *hbox;

	Gtk::Adjustment layer_amount_adjustment_;

	Gtk::HScale *layer_amount_hscale;

	sinfg::Layer::Handle quick_layer;

	Glib::RefPtr<LayerTreeStore> layer_tree_store_;

	Glib::RefPtr<LayerParamTreeStore> param_tree_store_;
	
	Glib::RefPtr<Gtk::TreeModelSort> sorted_layer_tree_store_;

//	CellRenderer_TimeTrack *cellrenderer_time_track;

	Gtk::TreeView::Column* column_time_track;

	Gtk::TreeView::Column* column_z_depth;

	CellRenderer_ValueBase *cellrenderer_value;

	sigc::signal<void,sinfg::Layer::Handle> signal_layer_toggle_;

	sigc::signal<void,sinfgapp::ValueDesc,sinfg::ValueBase> signal_edited_value_;

	sigc::signal<bool, int, Gtk::TreeRow, ColumnID> signal_layer_user_click_;

	sigc::signal<bool, int, Gtk::TreeRow, ColumnID> signal_param_user_click_;

	sigc::signal<void,sinfgapp::ValueDesc,sinfg::Waypoint,int> signal_waypoint_clicked_;

	bool disable_amount_changed_signal;

	Gtk::Button *button_raise;
	Gtk::Button *button_lower;
	Gtk::Button *button_duplicate;
	Gtk::Button *button_delete;

	Widget_ValueBase blend_method_widget;
	
	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	
	Gtk::Widget* create_layer_tree();
	Gtk::Widget* create_param_tree();
	
	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_edited_value(const Glib::ustring&path_string,sinfg::ValueBase value);

	void on_layer_toggle(const Glib::ustring& path_string);

	void on_waypoint_clicked(const Glib::ustring &, sinfg::Waypoint, int button);

	void on_waypoint_changed( sinfg::Waypoint waypoint , sinfg::ValueNode::Handle value_node);

	bool on_layer_tree_event(GdkEvent *event);

	bool on_param_tree_event(GdkEvent *event);

	void on_selection_changed();

	void on_dirty_preview();

	void on_amount_value_changed();

	void on_blend_method_changed();

public:
	
	void on_raise_pressed();

	void on_lower_pressed();

	void on_duplicate_pressed();

	void on_delete_pressed();

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	//Gtk::TreeView* get_param_tree_view() { return param_tree_view_; }
	Gtk::TreeView& param_tree_view() { return *param_tree_view_; }
	Gtk::HBox& get_hbox() { return *hbox; }

	Gtk::TreeView& get_layer_tree_view() { return *layer_tree_view_; }
	Gtk::TreeView& get_param_tree_view() { return *param_tree_view_; }

	const Gtk::TreeView& get_layer_tree_view()const { return *layer_tree_view_; }
	const Gtk::TreeView& get_param_tree_view()const { return *param_tree_view_; }
	
	Glib::RefPtr<Gtk::TreeSelection> get_selection() { return get_layer_tree_view().get_selection(); }
	Glib::SignalProxy1< bool,GdkEvent* >  signal_event () { return get_layer_tree_view().signal_event(); }
	
	LayerTree();
	~LayerTree();

	void set_model(Glib::RefPtr<LayerTreeStore> layer_tree_store_);

	void set_time_adjustment(Gtk::Adjustment &adjustment);

	void set_show_timetrack(bool x=true);

	//! Signal called when layer is toggled.
	sigc::signal<void,sinfg::Layer::Handle>& signal_layer_toggle() { return signal_layer_toggle_; }

	//! Signal called with a value has been edited.
	sigc::signal<void,sinfgapp::ValueDesc,sinfg::ValueBase>& signal_edited_value() { return signal_edited_value_; }

	sigc::signal<bool,int, Gtk::TreeRow, ColumnID>& signal_layer_user_click() { return signal_layer_user_click_; }

	sigc::signal<bool,int, Gtk::TreeRow, ColumnID>& signal_param_user_click() { return signal_param_user_click_; }

	sigc::signal<void,sinfgapp::ValueDesc,sinfg::Waypoint,int>& signal_waypoint_clicked() { return signal_waypoint_clicked_; }

	etl::handle<sinfgapp::SelectionManager> get_selection_manager() { return layer_tree_store_->canvas_interface()->get_selection_manager(); }
	
	
	
	void select_layer(sinfg::Layer::Handle layer);
	void select_layers(const LayerList& layer_list);
	void select_all_children_layers(sinfg::Layer::Handle layer);
	void select_all_children(Gtk::TreeModel::Children::iterator iter);
	LayerList get_selected_layers()const;
	sinfg::Layer::Handle get_selected_layer()const;
	void clear_selected_layers();

}; // END of LayerTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
