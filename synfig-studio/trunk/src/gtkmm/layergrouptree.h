/* === S I N F G =========================================================== */
/*!	\file layergrouptree.h
**	\brief Template Header
**
**	$Id: layergrouptree.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_LAYERGROUPTREE_H
#define __SINFG_STUDIO_LAYERGROUPTREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <sinfgapp/canvasinterface.h>
#include <sinfgapp/value_desc.h>
#include "layergrouptreestore.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg { class Layer; }

namespace studio {

class LayerGroupTree : public Gtk::TreeView
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef std::list<sinfg::Layer::Handle> LayerList;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:
	
	LayerGroupTreeStore::Model model;
	
	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Glib::RefPtr<LayerGroupTreeStore> layer_group_tree_store_;

	Gtk::CellRendererText *cell_renderer_description;

	bool editable_;


	sigc::signal<void,etl::handle<sinfg::Layer> > signal_popup_layer_menu_;

//	sigc::signal<void,LayerList> signal_select_layers_;
	Gtk::TreeView::Column* label_column;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	
	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	bool on_event(GdkEvent *event);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	LayerGroupTree();
	~LayerGroupTree();
	void set_cursor(const Gtk::TreeModel::Path& path, bool start_editing=false);

	Glib::RefPtr<LayerGroupTreeStore> get_model() { return layer_group_tree_store_; }

	sigc::signal<void,etl::handle<sinfg::Layer> >& signal_popup_layer_menu() { return signal_popup_layer_menu_; }

//	sigc::signal<void,LayerList>& signal_select_layers() { return signal_select_layers_; }

	void set_model(Glib::RefPtr<LayerGroupTreeStore> layer_group_tree_store_);

	void set_editable(bool x=true);
	
	bool get_editable()const { return editable_; }
	
	std::list<sinfg::String> get_selected_groups()const;

	LayerList get_selected_layers()const;	
}; // END of LayerGroupTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
