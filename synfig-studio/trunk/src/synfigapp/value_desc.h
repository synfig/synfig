/* === S Y N F I G ========================================================= */
/*!	\file value_desc.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_APP_VALUE_DESC_H
#define __SYNFIG_APP_VALUE_DESC_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>
#include <synfig/string.h>
#include <synfig/layer.h>
#include <synfig/value.h>
#include <synfig/valuenode_const.h>
#include <synfig/canvas.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class ValueDesc
{
	// Info for Layer parent
	synfig::Layer::Handle layer;
	synfig::String name;

	// Info for ValueNode parent
	synfig::ValueNode::Handle parent_value_node;
	int index;

	// Info for exported ValueNode
	synfig::Canvas::Handle canvas;

public:
	bool operator==(const ValueDesc &rhs)const
	{
		if((layer||rhs.layer) && layer!=rhs.layer)
			return false;
		if((!name.empty()||!rhs.name.empty()) && name!=rhs.name)
			return false;
		if(layer)
			return true;
		if((canvas||rhs.canvas) && canvas!=rhs.canvas)
			return false;
		if((parent_value_node||rhs.parent_value_node) && parent_value_node!=rhs.parent_value_node)
			return false;
		if((index>-1||rhs.index>-1) && index!=rhs.index)
			return false;
		return true;
	}
	bool operator!=(const ValueDesc &rhs)const
	{
		return !operator==(rhs);
	}


	ValueDesc(synfig::Layer::Handle layer,const synfig::String& param_name):
		layer(layer),
		name(param_name) { }

	ValueDesc(synfig::Layer::LooseHandle layer,const synfig::String& param_name):
		layer(layer),
		name(param_name) { }

	ValueDesc(synfig::LinkableValueNode::Handle parent_value_node,int index):
		parent_value_node(parent_value_node),
		index(index) { }

//	ValueDesc(synfig::LinkableValueNode::Handle parent_value_node,const synfig::String& param_name):
//		parent_value_node(parent_value_node),
//		index(parent_value_node->get_link_index_from_name(param_name)) { }

	ValueDesc(synfig::Canvas::Handle canvas,const synfig::String& name):
		name(name),
		canvas(canvas) { }

	ValueDesc(synfig::ValueNode_Const::Handle parent_value_node):
		parent_value_node(parent_value_node),
		index(-1) { }

	ValueDesc() { }

	bool is_valid()const { return layer || parent_value_node || canvas; }
	operator bool()const { return is_valid(); }

	bool parent_is_layer_param()const { return (bool)layer; }
	bool parent_is_value_node()const { return (bool)parent_value_node; }
	bool parent_is_linkable_value_node()const { return parent_is_value_node() && index>=0; }
	bool parent_is_value_node_const()const { return parent_is_value_node() && index==-1; }
	bool parent_is_canvas()const { return (bool)canvas; }

	bool is_value_node()const { return parent_is_value_node() || parent_is_canvas() || (parent_is_layer_param() && (bool)layer->dynamic_param_list().count(name)); }
	bool is_const()const { return (parent_is_layer_param() && !layer->dynamic_param_list().count(name)) || parent_is_value_node_const(); }

	synfig::Layer::Handle get_layer()const { assert(parent_is_layer_param()); return layer; }
	const synfig::String& get_param_name()const { assert(parent_is_layer_param()); return name; }

	synfig::ValueNode::Handle get_parent_value_node()const { assert(parent_is_value_node()); return parent_value_node; }
	int get_index()const { assert(parent_is_linkable_value_node()); return index; }

	const synfig::String& get_value_node_id()const { assert(parent_is_canvas()); return name; }

	synfig::Canvas::Handle get_canvas()const
	{
		if(canvas)
			return canvas;
		if(layer)
			return layer->get_canvas();
		if(parent_value_node)
			return parent_value_node->get_root_canvas();
		return 0;
	}

	synfig::ValueNode::Handle
	get_value_node()const
	{
		if(parent_is_canvas())
			return canvas->find_value_node(name);
		if(parent_is_layer_param() && layer->dynamic_param_list().count(name))
			return layer->dynamic_param_list().find(name)->second;
		if(parent_is_linkable_value_node())
			return synfig::LinkableValueNode::Handle::cast_reinterpret(parent_value_node)->get_link(index);
//			return reinterpret_cast<synfig::LinkableValueNode*>(parent_value_node.get())->get_link(index);
		return 0;
	}

	synfig::ValueBase
	get_value(synfig::Time time=0)const
	{
		if(parent_is_value_node_const() && parent_value_node)
			// \todo "(*parent_value_node)(0)"?  shouldn't that be "(*parent_value_node)(time)"?
			return (*parent_value_node)(0);
		if(is_value_node() && get_value_node())
			return (*get_value_node())(time);
		if(parent_is_layer_param() && layer)
			return layer->get_param(name);
		return synfig::ValueBase();
	}

	synfig::ValueBase::Type
	get_value_type()const
	{
		synfig::ValueNode::Handle value_node=get_value_node();
		if(value_node)
			return value_node->get_type();
		return get_value().get_type();
	}

	bool
	is_exported()const
	{
		return is_value_node() && get_value_node()->is_exported();
	}

	synfig::String
	get_description(bool show_exported_name = true)const;
}; // END of class ValueDesc

}; // END of namespace synfigapp_instance

/* === E N D =============================================================== */

#endif
