/* === S Y N F I G ========================================================= */
/*!	\file selectionmanager.h
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

#ifndef __SYNFIG_APP_SELECTIONMANAGER_H
#define __SYNFIG_APP_SELECTIONMANAGER_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <set>
#include <ETL/handle>
#include <synfig/layer.h>
#include <synfig/valuenode.h>
#include "value_desc.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class SelectionManager : public etl::shared_object
{
public:
	typedef std::pair<synfig::Layer::Handle,synfig::String> LayerParam;
	typedef std::list<LayerParam> LayerParamList;

	typedef std::list<synfig::Layer::Handle> LayerList;
	typedef std::list<ValueDesc> ChildrenList;
	//typedef std::list<synfig::ValueNode::Handle> ValueNodeList;

	virtual ~SelectionManager() { }

	//! Returns the number of layers selected.
	virtual int get_selected_layer_count()const=0;

	//! Returns a list of the currently selected layers.
	virtual LayerList get_selected_layers()const=0;

	//! Returns the first layer selected or an empty handle if none are selected.
	virtual synfig::Layer::Handle get_selected_layer()const=0;

	//! Sets which layers should be selected
	virtual void set_selected_layers(const LayerList &layer_list)=0;

	//! Sets which layer should be selected. Empty handle if none.
	virtual void set_selected_layer(const synfig::Layer::Handle &layer)=0;

	//! Clears the layer selection list
	virtual void clear_selected_layers()=0;

	virtual LayerList get_expanded_layers()const { return LayerList(); }
	virtual void set_expanded_layers(const LayerList & /* layer_list */) { }


	//! Returns the number of children selected.
	virtual int get_selected_children_count()const=0;

	//! Returns a list of the currently selected children.
	virtual ChildrenList get_selected_children()const=0;

	//! Returns the first children selected or an empty handle if none are selected.
	virtual ChildrenList::value_type get_selected_child()const=0;

	//! Sets which children should be selected
	virtual void set_selected_children(const ChildrenList &children_list)=0;

	//! Sets which children should be selected. Empty handle if none.
	virtual void set_selected_child(const ChildrenList::value_type &children)=0;

	//! Clears the children selection list
	virtual void clear_selected_children()=0;


	//! Returns the number of layer parameters selected.
	virtual int get_selected_layer_parameter_count()const=0;

	//! Returns a list of the currently selected layer parameters.
	virtual LayerParamList get_selected_layer_parameters()const=0;

	//! Returns the first layer parameter selected or an empty handle if none are selected.
	virtual LayerParam get_selected_layer_parameter()const=0;

	//! Sets which layer parameters should be selected
	virtual void set_selected_layer_parameters(const LayerParamList &layer_param_list)=0;

	//! Sets which layer parameter should be selected. Empty handle if none.
	virtual void set_selected_layer_param(const LayerParam &layer_param)=0;

	//! Clears the layer parameter selection list
	virtual void clear_selected_layer_parameters()=0;
}; // END of class SelectionManager

//! A place holding selection manager that does nothing
class NullSelectionManager : public SelectionManager
{
public:
	int get_selected_layer_count()const { return 0; }
	LayerList get_selected_layers()const { return LayerList(); }
	synfig::Layer::Handle get_selected_layer()const { return 0; }
	void set_selected_layers(const LayerList &/*layer_list*/) { return; }
	void set_selected_layer(const synfig::Layer::Handle &/*layer*/) { return; }
	void clear_selected_layers() { return; }


	int get_selected_children_count()const { return 0; }
	ChildrenList get_selected_children()const { return ChildrenList(); }
	ChildrenList::value_type get_selected_child()const { return ChildrenList::value_type(); }
	void set_selected_children(const ChildrenList &/*children_list*/) { return; }
	void set_selected_child(const ChildrenList::value_type &/*child*/) { return; }
	void clear_selected_children() { return; }

	int get_selected_layer_parameter_count()const { return 0; }
	LayerParamList get_selected_layer_parameters()const { return LayerParamList(); }
	LayerParam get_selected_layer_parameter()const { return LayerParam(); }
	void set_selected_layer_parameters(const LayerParamList &/*layer_param_list*/) { return; }
	void set_selected_layer_param(const LayerParam &/*layer_param*/) { return; }
	void clear_selected_layer_parameters() { return; }

}; // END of class NullSelectionManager

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
