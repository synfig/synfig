/* === S I N F G =========================================================== */
/*!	\file layerparamtreestore.h
**	\brief Template Header
**
**	$Id: layerparamtreestore.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_LAYERPARAMTREESTORE_H
#define __SINFG_STUDIO_LAYERPARAMTREESTORE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treestore.h>
#include <sinfgapp/canvasinterface.h>
#include "canvastreestore.h"
#include <sinfg/value.h>
#include <sinfg/valuenode.h>
#include <sinfg/paramdesc.h>

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
	typedef std::list<sinfg::Layer::Handle> LayerList;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:
	
	//! TreeModel for the layer parameters
	class Model : public CanvasTreeStore::Model
	{
	public:

		Gtk::TreeModelColumn<sinfg::ParamDesc>	param_desc;

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
	virtual void set_row(Gtk::TreeRow row,sinfgapp::ValueDesc value_desc);

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	void on_value_node_child_added(sinfg::ValueNode::Handle value_node,sinfg::ValueNode::Handle child);
	void on_value_node_child_removed(sinfg::ValueNode::Handle value_node,sinfg::ValueNode::Handle child);

	void on_value_node_added(sinfg::ValueNode::Handle value_node);
	void on_value_node_deleted(sinfg::ValueNode::Handle value_node);
	virtual void on_value_node_changed(sinfg::ValueNode::Handle value_node);
	void on_value_node_replaced(sinfg::ValueNode::Handle replaced_value_node,sinfg::ValueNode::Handle new_value_node);
	void on_layer_param_changed(sinfg::Layer::Handle handle,sinfg::String param_name);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	
	LayerParamTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_,
		LayerTree* layer_tree);
	~LayerParamTreeStore();

	void rebuild();

	void refresh();

	void queue_refresh();

	void queue_rebuild();

	void refresh_row(Gtk::TreeModel::Row &row);

	sigc::signal<void>& signal_changed() { return signal_changed_; }

	void changed() { signal_changed_(); }
	
	/*
 -- ** -- S T A T I C   P U B L I C   M E T H O D S ---------------------------
	*/

public:
	
	static Glib::RefPtr<LayerParamTreeStore> create(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_, LayerTree*layer_tree);
}; // END of class LayerParamTreeStore

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
