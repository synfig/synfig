/* === S I N F G =========================================================== */
/*!	\file value_desc.h
**	\brief Template Header
**
**	$Id: value_desc.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_VALUE_DESC_H
#define __SINFG_APP_VALUE_DESC_H

/* === H E A D E R S ======================================================= */

#include <sinfg/valuenode.h>
#include <sinfg/string.h>
#include <sinfg/layer.h>
#include <sinfg/value.h>
#include <sinfg/valuenode_const.h>
#include <sinfg/canvas.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class ValueDesc
{
	// Info for Layer parent
	sinfg::Layer::Handle layer;
	sinfg::String name;
	
	// Info for ValueNode parent
	sinfg::ValueNode::Handle parent_value_node;
	int index;

	// Info for exported ValueNode
	sinfg::Canvas::Handle canvas;
	
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

	
	ValueDesc(sinfg::Layer::Handle layer,const sinfg::String& param_name):
		layer(layer),
		name(param_name) { }

	ValueDesc(sinfg::Layer::LooseHandle layer,const sinfg::String& param_name):
		layer(layer),
		name(param_name) { }

	ValueDesc(sinfg::LinkableValueNode::Handle parent_value_node,int index):
		parent_value_node(parent_value_node),
		index(index) { }

//	ValueDesc(sinfg::LinkableValueNode::Handle parent_value_node,const sinfg::String& param_name):
//		parent_value_node(parent_value_node),
//		index(parent_value_node->get_link_index_from_name(param_name)) { }

	ValueDesc(sinfg::Canvas::Handle canvas,const sinfg::String& name):
		name(name),
		canvas(canvas) { }

	ValueDesc(sinfg::ValueNode_Const::Handle parent_value_node):
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
	
	sinfg::Layer::Handle get_layer()const { assert(parent_is_layer_param()); return layer; }
	const sinfg::String& get_param_name()const { assert(parent_is_layer_param()); return name; }
	
	sinfg::ValueNode::Handle get_parent_value_node()const { assert(parent_is_value_node()); return parent_value_node; }
	int get_index()const { assert(parent_is_linkable_value_node()); return index; }
	
	const sinfg::String& get_value_node_id()const { assert(parent_is_canvas()); return name; }

	sinfg::Canvas::Handle get_canvas()const
	{
		if(canvas)
			return canvas;
		if(layer)
			return layer->get_canvas();
		if(parent_value_node)
			return parent_value_node->get_root_canvas();
		return 0;
	}
	
	sinfg::ValueNode::Handle
	get_value_node()const
	{
		if(parent_is_canvas())
			return canvas->find_value_node(name);
		if(parent_is_layer_param() && layer->dynamic_param_list().count(name))
			return layer->dynamic_param_list().find(name)->second;
		if(parent_is_linkable_value_node())
			return sinfg::LinkableValueNode::Handle::cast_reinterpret(parent_value_node)->get_link(index);
//			return reinterpret_cast<sinfg::LinkableValueNode*>(parent_value_node.get())->get_link(index);
		return 0;
	}

	sinfg::ValueBase
	get_value(sinfg::Time time=0)const
	{
		if(parent_is_value_node_const() && parent_value_node)
			return (*parent_value_node)(0);
		if(is_value_node() && get_value_node())
			return (*get_value_node())(time);
		if(parent_is_layer_param() && layer)
			return layer->get_param(name);
		return sinfg::ValueBase();
	}	

	sinfg::ValueBase::Type
	get_value_type()const
	{
		sinfg::ValueNode::Handle value_node=get_value_node();
		if(value_node)
			return value_node->get_type();
		return get_value().get_type();
	}
	
	bool
	is_exported()const
	{
		return is_value_node() && get_value_node()->is_exported();
	}
}; // END of class ValueDesc

}; // END of namespace sinfgapp_instance

/* === E N D =============================================================== */

#endif
