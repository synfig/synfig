/* === S Y N F I G ========================================================= */
/*!	\file value_desc.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2009 Nikita Kitaev
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

#ifndef __SYNFIG_APP_VALUE_DESC_H
#define __SYNFIG_APP_VALUE_DESC_H

/* === H E A D E R S ======================================================= */

#include <synfig/valuenode.h>
#include <synfig/string.h>
#include <synfig/layer.h>
#include <synfig/value.h>
#include <synfig/valuenodes/valuenode_animated.h>
#include <synfig/valuenodes/valuenode_const.h>
#include <synfig/canvas.h>
#include <synfig/interpolation.h>
#include "synfigapp_export.h"

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
	static const int IS_WAYPOINT = -2;
	static const int IS_CONST = -1;
	int index;					// -2 if it's a waypoint, -1 if it's const, >=0 if it's LinkableValueNode
	synfig::Time waypoint_time;

	// Info for exported ValueNode
	synfig::Canvas::Handle canvas;
	//! if an exported valuenode, keep tracking name changes
	sigc::connection id_changed_connection;
	//! When value node id (name) changes, update internal reference
	void on_id_changed();

	// Info for sub-value of parent ValueDesc
	std::vector<synfig::String> sub_names;

	ValueDesc *parent_desc;
	int links_count;

	SYNFIGAPP_EXPORT static const ValueDesc blank;

	static ValueDesc* init_parent(const ValueDesc& parent)
	{
		if (!parent.is_valid()) return nullptr;
		ValueDesc *p = new ValueDesc(parent);
		p->links_count++;
		return p;
	}

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
		if(index!=rhs.index)
			return false;
		if(sub_names!=rhs.sub_names)
			return false;
		return true;
	}

	bool operator!=(const ValueDesc &rhs)const
	{
		return !operator==(rhs);
	}

	ValueDesc& operator=(const ValueDesc &other)
	{
		if (this == &other)
			return *this;
			
		layer = other.layer;
		name = other.name;
		parent_value_node = other.parent_value_node;
		index = other.index;
		waypoint_time = other.waypoint_time;
		canvas = other.canvas;
		sub_names = other.sub_names;
		if (parent_desc && 0 >= --parent_desc->links_count)
			delete parent_desc;
		parent_desc = other.parent_desc;
		if (parent_desc) parent_desc->links_count++;

		if (id_changed_connection.connected())
			id_changed_connection.disconnect();
		if (other.id_changed_connection.connected() && is_valid())
			id_changed_connection = get_value_node()->signal_id_changed().connect(sigc::mem_fun(*this, &ValueDesc::on_id_changed));

		return *this;
	}

	ValueDesc(synfig::Layer::Handle layer,const synfig::String& param_name,const ValueDesc &parent = blank):
		layer(layer),
		name(param_name),
		index(IS_CONST),
		parent_desc(init_parent(parent)),
		links_count(0)
	{ }

	ValueDesc(synfig::Layer::LooseHandle layer,const synfig::String& param_name,const ValueDesc &parent = blank):
		layer(layer),
		name(param_name),
		index(IS_CONST),
		parent_desc(init_parent(parent)),
		links_count(0)
	{ }

	ValueDesc(synfig::LinkableValueNode::Handle parent_value_node,int index,const ValueDesc &parent = blank):
		parent_value_node(parent_value_node),
		index(index),
		parent_desc(init_parent(parent)),
		links_count(0)
	{ }

	ValueDesc(synfig::ValueNode_Animated::Handle parent_value_node,synfig::Time waypoint_time,const ValueDesc &parent = blank):
		parent_value_node(parent_value_node),
		index(IS_WAYPOINT),
		waypoint_time(waypoint_time),
		parent_desc(init_parent(parent)),
		links_count(0)
	{ }

	ValueDesc(synfig::Canvas::Handle canvas,const synfig::String& name,const ValueDesc &parent = blank):
		name(name),
		index(IS_CONST),
		canvas(canvas),
		parent_desc(init_parent(parent)),
		links_count(0)
	{
		id_changed_connection = get_value_node()->signal_id_changed().connect(sigc::mem_fun(*this, &ValueDesc::on_id_changed));
	}

	ValueDesc(synfig::ValueNode_Const::Handle parent_value_node,const ValueDesc &parent = blank):
		parent_value_node(parent_value_node),
		index(IS_CONST),
		parent_desc(init_parent(parent)),
		links_count(0)
	{ }

	ValueDesc(const ValueDesc &parent, const synfig::String &sub_name):
		layer(parent.layer),
		name(parent.name),
		parent_value_node(parent.parent_value_node),
		index(parent.index),
		waypoint_time(parent.waypoint_time),
		canvas(parent.canvas),
		parent_desc(init_parent(parent)),
		links_count(0)
	{
		assert(!sub_name.empty());
		sub_names.reserve(parent.sub_names.size() + 1);
		sub_names.insert(sub_names.end(), parent.sub_names.begin(), parent.sub_names.end());
		sub_names.push_back(sub_name);
	}

	// copy
	ValueDesc(const ValueDesc &other):
		layer(other.layer),
		name(other.name),
		parent_value_node(other.parent_value_node),
		index(other.index),
		waypoint_time(other.waypoint_time),
		canvas(other.canvas),
		sub_names(other.sub_names),
		parent_desc(other.parent_desc),
		links_count(0)
	{
		if (other.id_changed_connection.connected() && is_valid())
			id_changed_connection = get_value_node()->signal_id_changed().connect(sigc::mem_fun(*this, &ValueDesc::on_id_changed));
		if (parent_desc) parent_desc->links_count++;
	}

	ValueDesc():
		index(IS_CONST), parent_desc(nullptr), links_count(0) { }

	~ValueDesc()
	{
		assert(links_count == 0);
		if (id_changed_connection.connected())
			id_changed_connection.disconnect();
		if (parent_desc && 0 >= --parent_desc->links_count)
			delete parent_desc;
	}

	// Introspection members
	bool
	is_valid()const
		{ return layer || parent_value_node || (canvas && !name.empty()); }

	operator bool()const { return is_valid(); }

	bool
	parent_is_layer()const
		{ return layer; }
	bool
	parent_is_value_node()const
		{ return parent_value_node; }
	bool
	parent_is_linkable_value_node()const
		{ return parent_is_value_node() && index>=0; }
	bool
	parent_is_value_node_const()const
		{ return parent_is_value_node() && index==IS_CONST; }
	bool
	parent_is_waypoint()const
		{ return parent_is_value_node() && index==IS_WAYPOINT; }
	bool
	parent_is_canvas()const
		{ return canvas; }
	bool
	parent_is_value_desc()const
		{ return !sub_names.empty(); }

	bool
	is_value_node()const
		{ return (parent_is_value_node()
		      || parent_is_canvas()
			  || (parent_is_layer() && layer->dynamic_param_list().count(name) > 0))
              && is_valid();
		}
	bool
	is_const()const
		{ return (parent_is_layer() && !layer->dynamic_param_list().count(name))
		      || parent_is_value_node_const()
		      || (parent_is_linkable_value_node() && synfig::ValueNode_Const::Handle::cast_dynamic(get_value_node()));
		}
	
	bool
	is_animated()const
		{ return ( parent_is_layer()
			    && layer->dynamic_param_list().count(name)
			    && synfig::ValueNode_Animated::Handle::cast_dynamic(layer->dynamic_param_list().find(name)->second))
			 ||  ( parent_is_canvas()
			    && synfig::ValueNode_Animated::Handle::cast_dynamic(get_value_node()));
		}

	bool
	is_parent_desc_declared()const
		{ return parent_desc; }
	
	// Get members
	const ValueDesc& get_sub_parent_desc()const
		{ return parent_desc ? *parent_desc : blank; }
	const ValueDesc& get_origin_desc()const
		{ return parent_is_value_desc() ? get_sub_parent_desc().get_origin_desc() : *this ; }
	const ValueDesc& get_parent_desc()const
		{ return get_origin_desc().get_sub_parent_desc().get_origin_desc(); }

	synfig::Layer::Handle
	get_layer()const
		{ assert(parent_is_layer()); return layer; }
	
	const synfig::String&
	get_param_name()const
		{ assert(parent_is_layer()); return name; }

	bool
	find_param_desc(synfig::ParamDesc &out_param_desc)const {
		assert(parent_is_layer());
		if (layer) {
			synfig::Layer::Vocab vocab = layer->get_param_vocab();
			for(synfig::Layer::Vocab::iterator i = vocab.begin(); i != vocab.end(); i++)
				if (i->get_name() == name)
					{ out_param_desc = *i; return true; }
		}
		return false;
	}

	synfig::ValueNode::Handle
	get_parent_value_node()const
		{ assert(parent_is_value_node()); return parent_value_node; }
	
	int
	get_index()const
		{ assert(parent_is_linkable_value_node()); return index; }

	synfig::String
	get_name()const
		{ assert(parent_is_linkable_value_node()); return (synfig::LinkableValueNode::Handle::cast_reinterpret(parent_value_node))->link_name(index); }
	
	const std::vector<synfig::String>&
	get_sub_names()const
		{ assert(parent_is_value_desc()); return sub_names; }

	const synfig::String&
	get_sub_name()const
		{ assert(parent_is_value_desc()); return sub_names.front(); }

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
		return nullptr;
	}

	synfig::ValueNode::Handle
	get_value_node()const
	{
		if (!is_value_node()) return nullptr;

		if(parent_is_canvas())
			return canvas->find_value_node(name,false);
		if(parent_is_layer() && layer->dynamic_param_list().count(name))
			return layer->dynamic_param_list().find(name)->second;
		if(parent_is_linkable_value_node())
			return (synfig::LinkableValueNode::Handle::cast_reinterpret(parent_value_node))->get_link(index);
//			return reinterpret_cast<synfig::LinkableValueNode*>(parent_value_node.get())->get_link(index);
		if(parent_is_value_node_const())
			return parent_value_node;
		if(parent_is_waypoint())
			return (synfig::ValueNode_Animated::Handle::cast_reinterpret(parent_value_node))->find(waypoint_time)->get_value_node();
		return nullptr;
	}

	synfig::ValueBase
	get_value(synfig::Time time=0)const
	{
		// if the value is constant, return that constant value (at *any* time, it doesn't matter which)
		if(parent_is_value_node_const())
			return (*parent_value_node)(0);
		if(is_value_node() && get_value_node())
			if (!parent_is_canvas() || !name.empty())
				return (*get_value_node())(time);
		if(parent_is_layer() && layer)
			return layer->get_param(name);
		return synfig::ValueBase();
	}

	synfig::Type&
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
		if(parent_is_layer() && is_const())
			return get_value().get_interpolation();
		else
		{
		    synfig::ValueNode::Handle value=get_value_node();
		    if (value)
			return value->get_interpolation();
		    else
			return synfig::Interpolation(5);
		}
	}

	bool
	get_static()const
	{
		if(is_const())
		   return get_value().get_static();
		return false;
	}

	synfig::GUID get_guid()const
	{
		if (parent_is_value_desc())
			return get_sub_parent_desc().get_guid() % synfig::GUID::hasher(get_sub_names().back());
		if (is_value_node())
			return get_value_node()->get_guid();
		if (parent_is_layer())
			return get_layer()->get_guid() % synfig::GUID::hasher(get_param_name());
		assert(!is_valid());
		return synfig::GUID::zero();
	}

	synfig::String get_guid_string()const
	{
		return get_guid().get_string();
	}

	//! sub_name should be NOT empty
	ValueDesc create_sub_value(const synfig::String &sub_name)const
	{
		return ValueDesc(*this, sub_name);
	}

	//! @return copy of it self when sub_name is empty string
	ValueDesc get_sub_value(const synfig::String &sub_name)const
	{
		return sub_name.empty() ? *this : ValueDesc(*this, sub_name);
	}
}; // END of class ValueDesc

}; // END of namespace synfigapp_instance

/* === E N D =============================================================== */

#endif
