/* === S I N F G =========================================================== */
/*!	\file childrentreestore.h
**	\brief Template Header
**
**	$Id: childrentreestore.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_CHILDRENTREESTORE_H
#define __SINFG_STUDIO_CHILDRENTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <sinfgapp/canvasinterface.h>
#include "canvastreestore.h"
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <set>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class ChildrenTreeStore : public CanvasTreeStore
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:
	
	//! TreeModel for the layers
	const Model model;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:
	
	Gtk::TreeModel::Row value_node_row;
	Gtk::TreeModel::Row canvas_row;
	
	std::set<sinfg::ValueNode::Handle> changed_set_;

	std::set<sinfg::ValueNode::Handle> replaced_set_;
	
	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:
	
	sigc::connection changed_connection;
	bool execute_changed_queued()const { return !changed_set_.empty() || !replaced_set_.empty(); }
	bool execute_changed_value_nodes();
	void clear_changed_queue() { changed_set_.clear(); replaced_set_.clear(); }
	
	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_value_node_added(sinfg::ValueNode::Handle value_node);
	void on_value_node_deleted(sinfg::ValueNode::Handle value_node);
	void on_value_node_changed(sinfg::ValueNode::Handle value_node);
	void on_value_node_replaced(sinfg::ValueNode::Handle replaced_value_node,sinfg::ValueNode::Handle new_value_node);
	void on_canvas_added(sinfg::Canvas::Handle canvas);
	void on_canvas_removed(sinfg::Canvas::Handle canvas);

	void set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	ChildrenTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);
	~ChildrenTreeStore();

	void rebuild();

	void refresh();

	void rebuild_value_nodes();

	void refresh_value_nodes();

	void rebuild_canvases();

	void refresh_canvases();

	void refresh_row(Gtk::TreeModel::Row &row, bool do_children=false);

	Gtk::TreeModel::Row get_canvas_row()const { return canvas_row; }
	
	Gtk::TreeModel::Row get_value_node_row()const { return value_node_row; }

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:
	
	static Glib::RefPtr<ChildrenTreeStore> create(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_);
}; // END of class ChildrenTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
