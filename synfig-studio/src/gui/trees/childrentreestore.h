/* === S Y N F I G ========================================================= */
/*!	\file trees/childrentreestore.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_STUDIO_CHILDRENTREESTORE_H
#define __SYNFIG_STUDIO_CHILDRENTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gui/trees/canvastreestore.h>
#include <set>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfigapp/canvasinterface.h>

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

	std::set<synfig::ValueNode::Handle> changed_set_;

	std::set<synfig::ValueNode::Handle> replaced_set_;

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

	void on_value_node_added(synfig::ValueNode::Handle value_node);
	void on_value_node_deleted(synfig::ValueNode::Handle value_node);
	void on_value_node_changed(synfig::ValueNode::Handle value_node);
	void on_value_node_renamed(synfig::ValueNode::Handle value_node);
	void on_value_node_replaced(synfig::ValueNode::Handle replaced_value_node,synfig::ValueNode::Handle new_value_node);
	void on_canvas_added(synfig::Canvas::Handle canvas);
	void on_canvas_removed(synfig::Canvas::Handle canvas);

	void set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	ChildrenTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_);
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

	static Glib::RefPtr<ChildrenTreeStore> create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_);
}; // END of class ChildrenTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
