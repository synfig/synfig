/* === S Y N F I G ========================================================= */
/*!	\file value_desc.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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
#include <synfig/valuenode_animated.h>
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
	int index;					// -2 if it's a waypoint, -1 if it's const, >=0 if it's LinkableValueNode
	synfig::Time waypoint_time;

	// Info for exported ValueNode
	synfig::Canvas::Handle canvas;

	// Info for visual editon
	synfig::Real scalar;

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
		if((parent_value_node||rhs.parent_value_node) && (parent_value_node!=rhs.parent_value_node))
			return false;
		if(scalar!=rhs.scalar)
			return false;
		if(index!=rhs.index)
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

	ValueDesc(synfig::LinkableValueNode::Handle parent_value_node,int index, synfig::Real s=1.0):
		parent_value_node(parent_value_node),
		index(index),
		scalar(s) { }

//	ValueDesc(synfig::LinkableValueNode::Handle parent_value_node,const synfig::String& param_name):
//		parent_value_node(parent_value_node),
//		index(parent_value_node->get_link_index_from_name(param_name)) { }

	ValueDesc(synfig::ValueNode_Animated::Handle parent_value_node,synfig::Time waypoint_time):
		parent_value_node(parent_value_node),
		index(-2),
		waypoint_time(waypoint_time) { }

	ValueDesc(synfig::Canvas::Handle canvas,const synfig::String& name):
		name(name),
		canvas(canvas) { }

	ValueDesc(synfig::ValueNode_Const::Handle parent_value_node):
		parent_value_node(parent_value_node),
		index(-1) { }

	ValueDesc() { }

	// Instrocpection members
	bool
	is_valid()const
		{ return layer || parent_value_node || canvas; }
	operator bool()const { return is_valid(); }

	bool
	parent_is_layer_param()const
		{ return (bool)layer; }
	bool
	parent_is_value_node()const
		{ return (bool)parent_value_node; }
	bool
	parent_is_linkable_value_node()const
		{ return parent_is_value_node() && index>=0; }
	bool
	parent_is_value_node_const()const
		{ return parent_is_value_node() && index==-1; }
	bool
	parent_is_waypoint()const
		{ return parent_is_value_node() && index==-2; }
	bool
	parent_is_canvas()const
		{ return (bool)canvas; }
	bool
	is_value_node()const
		{ return parent_is_value_node() || parent_is_canvas() || (parent_is_layer_param() && (bool)layer->dynamic_param_list().count(name)); }
	bool
	is_const()const
		{ return
		(parent_is_layer_param() && !layer->dynamic_param_list().count(name))
		||
		parent_is_value_node_const()
		||
		(parent_is_linkable_value_node() && synfig::ValueNode_Const::Handle::cast_dynamic(get_value_node()));
		}
	
	bool
	is_animated()const
		{ return
			(parent_is_layer_param()
			 &&
			 layer->dynamic_param_list().count(name)
			 &&
			synfig::ValueNode_Animated::Handle::cast_dynamic(layer->dynamic_param_list().find(name)->second)
			 )
			||
			(parent_is_canvas()
			 &&
			 synfig::ValueNode_Animated::Handle::cast_dynamic(get_value_node())
			);
		}
	
	// Get members
	synfig::Layer::Handle
	get_layer()const
		{ assert(parent_is_layer_param()); return layer; }
	
	const synfig::String&
	get_param_name()const
		{ assert(parent_is_layer_param()); return name; }

	synfig::ValueNode::Handle
	get_parent_value_node()const
		{ assert(parent_is_value_node()); return parent_value_node; }
	
	int
	get_index()const
		{ assert(parent_is_linkable_value_node()); return index; }
	
	synfig::Real
	get_scalar()const
	{ assert(parent_is_linkable_value_node()); return scalar; }
	
	synfig::String
	get_name()const
		{ assert(parent_is_linkable_value_node()); return (synfig::LinkableValueNode::Handle::cast_reinterpret(parent_value_node))->link_name(index); }
	
	synfig::Time
	get_waypoint_time()const
		{ assert(parent_is_waypoint()); return waypoint_time; }

	const synfig::String&
	get_value_node_id()const
		{ assert(parent_is_canvas()); return name; }

	synfig::Canvas::Handle
	get_canvas()const
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
			return canvas->find_value_node(name,false);
		if(parent_is_layer_param() && layer->dynamic_param_list().count(name))
			return layer->dynamic_param_list().find(name)->second;
		if(parent_is_linkable_value_node())
			return (synfig::LinkableValueNode::Handle::cast_reinterpret(parent_value_node))->get_link(index);
//			return reinterpret_cast<synfig::LinkableValueNode*>(parent_value_node.get())->get_link(index);
		if(parent_is_value_node_const())
			return parent_value_node;
		if(parent_is_waypoint())
			return (synfig::ValueNode_Animated::Handle::cast_reinterpret(parent_value_node))->find(waypoint_time)->get_value_node();
		return 0;
	}

	synfig::ValueBase
	get_value(synfig::Time time=0)const
	{
		// if the value is constant, return that constant value (at *any* time, it doesn't matter which)
		if(parent_is_value_node_const())
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
	
	synfig::Interpolation
	get_interpolation()const
	{
		if(parent_is_layer_param() && is_const())
			return get_value().get_interpolation();
		else
			return get_value_node()->get_interpolation();
	}

	bool
	get_static()const
	{
		if(is_const())
		   return get_value().get_static();
		return false;
	}
}; // END of class ValueDesc

}; // END of namespace synfigapp_instance

/* === E N D =============================================================== */

#endif
