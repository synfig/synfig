/* === S Y N F I G ========================================================= */
/*!	\file layerparamtreestore.h
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

#ifndef __SYNFIG_STUDIO_LAYERPARAMTREESTORE_H
#define __SYNFIG_STUDIO_LAYERPARAMTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gui/trees/canvastreestore.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class LayerTree;

class LayerParamTreeStore : public CanvasTreeStore
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:
	typedef std::list<synfig::Layer::Handle> LayerList;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	//! TreeModel for the layer parameters
	class Model : public CanvasTreeStore::Model
	{
	public:

		Gtk::TreeModelColumn<synfig::ParamDesc>	param_desc;

		Gtk::TreeModelColumn<bool>	is_inconsistent;
		Gtk::TreeModelColumn<bool>	is_toplevel;

		Model()
		{
			add(param_desc);
			add(is_inconsistent);
			add(is_toplevel);
		}
	};

	Model model;


	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	int queued;

	LayerTree* layer_tree;

	LayerList layer_list;

	sigc::connection queue_connection;

	std::list<sigc::connection> changed_connection_list;

	sigc::signal<void> signal_changed_;

	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

protected:
	virtual void  get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const;
	virtual void set_value_impl (const Gtk::TreeModel::iterator& row, int column, const Glib::ValueBase& value);
	using CanvasTreeStore::set_row;
	virtual void set_row(Gtk::TreeRow row,synfigapp::ValueDesc value_desc);

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_value_node_child_added(synfig::ValueNode::Handle value_node,synfig::ValueNode::Handle child);
	void on_value_node_child_removed(synfig::ValueNode::Handle value_node,synfig::ValueNode::Handle child);

	void on_value_node_added(synfig::ValueNode::Handle value_node);
	void on_value_node_deleted(synfig::ValueNode::Handle value_node);
	virtual void on_value_node_changed(synfig::ValueNode::Handle value_node);
	virtual void on_value_node_renamed(synfig::ValueNode::Handle value_node);
	void on_value_node_replaced(synfig::ValueNode::Handle replaced_value_node,synfig::ValueNode::Handle new_value_node);
	void on_layer_param_changed(synfig::Layer::Handle handle,synfig::String param_name);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	LayerParamTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_,
		LayerTree* layer_tree);
	~LayerParamTreeStore();

	void rebuild();

	void refresh();

	void queue_refresh();

	void queue_rebuild();

	void refresh_row(Gtk::TreeModel::Row &row);

	//! \brief Search for a value descriptor on the parameter tree.
	//! On success get the node reference of the synfigapp::ValueDesc on the tree
	//! \Param[in]  value_desc The value to search for
	//! \Param[out] iter The node reference of the value in the tree
	//! \Return     Failed or success to find
	bool find_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter);

	//! \brief Search for a value descriptor on the parameter tree from a given treenode level
	//! On success get the node reference of the synfigapp::ValueDesc on the tree
	//! \Param[in]  value_desc The value to search for
	//! \Param[out] iter The node reference of the value in the tree
	//! \Param[in] child_iter The node level to look from
	//! \Return     Failed or success to find
	bool find_value_desc(const synfigapp::ValueDesc& value_desc, Gtk::TreeIter& iter, const Gtk::TreeNodeChildren child_iter);

	sigc::signal<void>& signal_changed() { return signal_changed_; }

	void changed() { signal_changed_(); }

	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:

	static Glib::RefPtr<LayerParamTreeStore> create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_, LayerTree*layer_tree);
}; // END of class LayerParamTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
