/* === S Y N F I G ========================================================= */
/*!	\file valuenode.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#define SYNFIG_NO_ANGLE

//#define HAS_HASH_MAP 1

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode.h"
#include "general.h"
#include "canvas.h"

#include "valuenode_const.h"
#include "valuenode_linear.h"
#include "valuenode_composite.h"
#include "valuenode_reference.h"
#include "valuenode_scale.h"
#include "valuenode_segcalctangent.h"
#include "valuenode_segcalcvertex.h"
#include "valuenode_stripes.h"
#include "valuenode_subtract.h"
#include "valuenode_timedswap.h"
#include "valuenode_twotone.h"
#include "valuenode_bline.h"
#include "valuenode_dynamiclist.h"
#include "valuenode_radialcomposite.h"
#include "valuenode_gradientrotate.h"
#include "valuenode_sine.h"

#include "layer.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static int value_node_count(0);

static LinkableValueNode::Book *book_;


ValueNode::LooseHandle
synfig::find_value_node(const GUID& guid)
{
	return guid_cast<ValueNode>(guid);
}

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
ValueNode::subsys_init()
{
	book_=new LinkableValueNode::Book();

#define ADD_VALUENODE(c,n,l)  (*book_)[n].factory=reinterpret_cast<LinkableValueNode::Factory>(&c::create);      (*book_)[n].check_type=&c::check_type;(*book_)[n].local_name=l
#define ADD_VALUENODE2(c,n,l) (*book_)[n].factory=reinterpret_cast<LinkableValueNode::Factory>(&c::create_from); (*book_)[n].check_type=&c::check_type;(*book_)[n].local_name=l

	ADD_VALUENODE(ValueNode_Linear,			"linear",			_("Linear")				);x
	ADD_VALUENODE(ValueNode_Composite,		"composite",		_("Composite")			);x
	ADD_VALUENODE(ValueNode_RadialComposite,"radial_composite",	_("Radial Composite")	);x
	ADD_VALUENODE(ValueNode_Reference,		"reference",		_("Reference")			);x
	ADD_VALUENODE(ValueNode_Scale,			"scale",			_("Scale")				);x
	ADD_VALUENODE(ValueNode_SegCalcTangent,	"segcalctangent",	_("Seg Calc Tangent")	);x
	ADD_VALUENODE(ValueNode_SegCalcVertex,	"segcalcvertex",	_("Seg Calc Vertex")	);x
	ADD_VALUENODE(ValueNode_Stripes,		"stripes",			_("Stripes")			);x
	ADD_VALUENODE(ValueNode_Subtract,		"subtract",			_("Subtract")			);x
	//ADD_VALUENODE(ValueNode_TimedSwap,	"timed_swap",		_("Timed Swap")			);x
	ADD_VALUENODE(ValueNode_TwoTone,		"twotone",			_("Two-Tone")			);x
	ADD_VALUENODE(ValueNode_BLine,			"bline",			_("BLine")				);x
	ADD_VALUENODE2(ValueNode_DynamicList,	"dynamic_list",		_("Dynamic List")		);x
	ADD_VALUENODE(ValueNode_GradientRotate,	"gradient_rotate",	_("Gradient Rotate")	);x
	ADD_VALUENODE(ValueNode_Sine,			"sine",				_("Sine")				);x

#undef ADD_VALUENODE
#undef ADD_VALUENODE2

	return true;
}

bool
ValueNode::subsys_stop()
{
	delete book_;
/*	if(global_value_node_map.size() || value_node_count)
	{
		if(value_node_count)
			synfig::error("%d ValueNodes haven't been destroyed yet!",value_node_count);

		if(global_value_node_map.size()!=value_node_count)
			synfig::error("value node count mismatch! map.size()!=value_node_count (%d!=%d)",global_value_node_map.size(),value_node_count);

		GlobalValueNodeMap::iterator iter;
		for(iter=global_value_node_map.begin();iter!=global_value_node_map.end();++iter)
		{
			if(!iter->second->is_exported())
				synfig::info("%s: count:%d name:%s type:%s",
					iter->first.get_string().c_str(),
					iter->second->count(),
					iter->second->get_name().c_str(),
					ValueBase::type_name(iter->second->get_type()).c_str()
				);
			else
				synfig::info("%s: id:%s count:%d name:%s type:%s",
					iter->first.get_string().c_str(),
					iter->second->get_id().c_str(),
					iter->second->count(),
					iter->second->get_name().c_str(),
					ValueBase::type_name(iter->second->get_type()).c_str()
				);
		}
	}
*/
	return true;
}

ValueNode::ValueNode(ValueBase::Type type):type(type)
{
	value_node_count++;
}

LinkableValueNode::Book&
LinkableValueNode::book()
{
	return *book_;
}

LinkableValueNode::Handle
LinkableValueNode::create(const String &name, const ValueBase& x)
{
	if(!book().count(name))
		return 0;
	return book()[name].factory(x);
}

bool
LinkableValueNode::check_type(const String &name, ValueBase::Type x)
{
	if(!book().count(name) || !book()[name].check_type)
		return false;
	return book()[name].check_type(x);
}

bool
LinkableValueNode::set_link(int i,ValueNode::Handle x)
{
	ValueNode::Handle previous(get_link(i));

	if(set_link_vfunc(i,x))
	{
		if(previous)
			remove_child(previous.get());
		add_child(x.get());

		if(!x->is_exported() && get_parent_canvas())
		{
			x->set_parent_canvas(get_parent_canvas());
		}
		changed();
		return true;
	}
	return false;
}

ValueNode::LooseHandle
LinkableValueNode::get_link(int i)const
{
	return get_link_vfunc(i);
}

void
LinkableValueNode::unlink_all()
{
	for(int i=0;i<link_count();i++)
	{
		ValueNode::LooseHandle value_node(get_link(i));
		if(value_node)
			value_node->parent_set.erase(this);
	}
}

ValueNode::~ValueNode()
{
	value_node_count--;

	begin_delete();

	//DEBUGPOINT();
}

void
ValueNode::on_changed()
{
	if(get_parent_canvas())
		get_parent_canvas()->signal_value_node_changed()(this);
	else if(get_root_canvas() && get_parent_canvas())
		get_root_canvas()->signal_value_node_changed()(this);

	Node::on_changed();
}

int
ValueNode::replace(etl::handle<ValueNode> x)
{
	if(x.get()==this)
		return 0;

	while(parent_set.size())
	{
		(*parent_set.begin())->add_child(x.get());
		(*parent_set.begin())->remove_child(this);
		//x->parent_set.insert(*parent_set.begin());
		//parent_set.erase(parent_set.begin());
	}
	int r(RHandle(this).replace(x));
	x->changed();
	return r;
}

void
ValueNode::set_id(const String &x)
{
	if(name!=x)
	{
		name=x;
		signal_id_changed_();
	}
}

ValueNodeList::ValueNodeList():
	placeholder_count_(0)
{
}

bool
ValueNodeList::count(const String &id)const
{
	const_iterator iter;

	if(id.empty())
		return false;

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter);

	if(iter==end())
		return false;

	return true;
}

ValueNode::Handle
ValueNodeList::find(const String &id)
{
	iterator iter;

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter);

	if(iter==end())
		throw Exception::IDNotFound("ValueNode in ValueNodeList: "+id);

	return *iter;
}

ValueNode::ConstHandle
ValueNodeList::find(const String &id)const
{
	const_iterator iter;

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter);

	if(iter==end())
		throw Exception::IDNotFound("ValueNode in ValueNodeList: "+id);

	return *iter;
}

ValueNode::Handle
ValueNodeList::surefind(const String &id)
{
	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	ValueNode::Handle value_node;

	try
	{
		value_node=find(id);
	}
	catch(Exception::IDNotFound)
	{
		value_node=PlaceholderValueNode::create();
		value_node->set_id(id);
		push_back(value_node);
		placeholder_count_++;
	}

	return value_node;
}

bool
ValueNodeList::erase(ValueNode::Handle value_node)
{
	assert(value_node);

	iterator iter;

	for(iter=begin();iter!=end();++iter)
		if(value_node.get()==iter->get())
		{
			std::list<ValueNode::RHandle>::erase(iter);
			if(PlaceholderValueNode::Handle::cast_dynamic(value_node))
				placeholder_count_--;
			return true;
		}
	return false;
}

bool
ValueNodeList::add(ValueNode::Handle value_node)
{
	if(!value_node)
		return false;
	if(value_node->get_id().empty())
		return false;

	try
	{
		ValueNode::RHandle other_value_node=find(value_node->get_id());
		if(PlaceholderValueNode::Handle::cast_dynamic(other_value_node))
		{
			other_value_node->replace(value_node);
			placeholder_count_--;
			return true;
		}

		return false;
	}
	catch(Exception::IDNotFound)
	{
		push_back(value_node);
		return true;
	}

	return false;
}

void
ValueNodeList::audit()
{
	iterator iter,next;

	for(next=begin(),iter=next++;iter!=end();iter=next++)
		if(iter->count()==1)
			std::list<ValueNode::RHandle>::erase(iter);
}


String
PlaceholderValueNode::get_name()const
{
	return "placeholder";
}

String
PlaceholderValueNode::get_local_name()const
{
	return _("Placeholder");
}

ValueNode*
PlaceholderValueNode::clone(const GUID& deriv_guid)const
{
	ValueNode* ret(new PlaceholderValueNode());
	ret->set_guid(get_guid()^deriv_guid);
	return ret;
}

PlaceholderValueNode::Handle
PlaceholderValueNode::create(ValueBase::Type type)
{
	return new PlaceholderValueNode(type);
}

ValueBase
PlaceholderValueNode::operator()(Time t)const
{
	assert(0);
	return ValueBase();
}

PlaceholderValueNode::PlaceholderValueNode(ValueBase::Type type):
	ValueNode(type)
{
}

ValueNode*
LinkableValueNode::clone(const GUID& deriv_guid)const
{
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }

	int i;
	LinkableValueNode *ret=create_new();
	ret->set_guid(get_guid()^deriv_guid);

	for(i=0;i<link_count();i++)
	{
		ValueNode::Handle link=get_link_vfunc(i);
		if(!link->is_exported())
		{
			ValueNode::LooseHandle value_node(find_value_node(link->get_guid()^deriv_guid));
			if(!value_node)
				value_node=link->clone(deriv_guid);
			ret->set_link(i,value_node);
		}
		else
			ret->set_link(i,link);
	}

	return ret;
}

String
ValueNode::get_relative_id(etl::loose_handle<const Canvas> x)const
{
	assert(is_exported());
	assert(canvas_);

	if(x.get()==canvas_.get())
		return get_id();

	return canvas_->_get_relative_id(x)+':'+get_id();
}

void
ValueNode::set_parent_canvas(etl::loose_handle<Canvas> x)
{
	canvas_=x; if(x) root_canvas_=x->get_root();
}

void
ValueNode::set_root_canvas(etl::loose_handle<Canvas> x)
{
	root_canvas_=x->get_root();
}

void LinkableValueNode::get_times_vfunc(Node::time_set &set) const
{
	ValueNode::LooseHandle	h;

	int size = link_count();

	//just add it to the set...
	for(int i=0; i < size; ++i)
	{
		h = get_link(i);

		if(h)
		{
			const Node::time_set &tset = h->get_times();
			set.insert(tset.begin(),tset.end());
		}
	}
}
