/* === S Y N F I G ========================================================= */
/*!	\file valuenode.cpp
**	\brief Implementation of the "Placeholder" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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
#include "paramdesc.h"
#include "releases.h"

#include "valuenode_const.h"
#include "valuenode_linear.h"
#include "valuenode_composite.h"
#include "valuenode_reference.h"
#include "valuenode_scale.h"
#include "valuenode_blinecalctangent.h"
#include "valuenode_blinecalcvertex.h"
#include "valuenode_blinecalcwidth.h"
#include "valuenode_blinereversetangent.h"
#include "valuenode_segcalctangent.h"
#include "valuenode_segcalcvertex.h"
#include "valuenode_repeat_gradient.h"
#include "valuenode_stripes.h"
#include "valuenode_range.h"
#include "valuenode_add.h"
#include "valuenode_subtract.h"
#include "valuenode_timedswap.h"
#include "valuenode_twotone.h"
#include "valuenode_bline.h"
#include "valuenode_dynamiclist.h"
#include "valuenode_radialcomposite.h"
#include "valuenode_gradientrotate.h"
#include "valuenode_sine.h"
#include "valuenode_cos.h"
#include "valuenode_atan2.h"
#include "valuenode_exp.h"
#include "valuenode_switch.h"
#include "valuenode_timeloop.h"
#include "valuenode_reciprocal.h"
#include "valuenode_duplicate.h"
#include "valuenode_integer.h"
#include "valuenode_step.h"

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

#define ADD_VALUENODE(class,name,local,version)													\
	(*book_)[name].factory=reinterpret_cast<LinkableValueNode::Factory>(&class::create);		\
	(*book_)[name].check_type=&class::check_type;												\
	(*book_)[name].local_name=local;															\
	(*book_)[name].release_version=version

#define ADD_VALUENODE2(class,name,local,version)												\
	(*book_)[name].factory=reinterpret_cast<LinkableValueNode::Factory>(&class::create_from);	\
	(*book_)[name].check_type=&class::check_type;												\
	(*book_)[name].local_name=local;															\
	(*book_)[name].release_version=version

	ADD_VALUENODE(ValueNode_Linear,			  "linear",			  _("Linear"),			 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Composite,		  "composite",		  _("Composite"),		 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_RadialComposite,  "radial_composite", _("Radial Composite"), RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Reference,		  "reference",		  _("Reference"),		 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Scale,			  "scale",			  _("Scale"),			 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_SegCalcTangent,	  "segcalctangent",	  _("Segment Tangent"),	 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_SegCalcVertex,	  "segcalcvertex",	  _("Segment Vertex"),	 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Stripes,		  "stripes",		  _("Stripes"),			 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Subtract,		  "subtract",		  _("Subtract"),		 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_TwoTone,		  "twotone",		  _("Two-Tone"),		 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_BLine,			  "bline",			  _("BLine"),			 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE2(ValueNode_DynamicList,	  "dynamic_list",	  _("Dynamic List"),	 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_GradientRotate,	  "gradient_rotate",  _("Gradient Rotate"),	 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Sine,			  "sine",			  _("Sine"),			 RELEASE_VERSION_0_61_06);

	ADD_VALUENODE(ValueNode_TimedSwap,		  "timed_swap",		  _("Timed Swap"),		 RELEASE_VERSION_0_61_07); // SVN r610
	ADD_VALUENODE(ValueNode_Repeat_Gradient,  "repeat_gradient",  _("Repeat Gradient"),	 RELEASE_VERSION_0_61_07); // SVN r666
	ADD_VALUENODE(ValueNode_Exp,			  "exp",			  _("Exponential"),		 RELEASE_VERSION_0_61_07); // SVN r739
	ADD_VALUENODE(ValueNode_Add,			  "add",			  _("Add"),				 RELEASE_VERSION_0_61_07); // SVN r742
	ADD_VALUENODE(ValueNode_BLineCalcTangent, "blinecalctangent", _("BLine Tangent"),	 RELEASE_VERSION_0_61_07); // SVN r744
	ADD_VALUENODE(ValueNode_BLineCalcVertex,  "blinecalcvertex",  _("BLine Vertex"),	 RELEASE_VERSION_0_61_07); // SVN r744
	ADD_VALUENODE(ValueNode_Range,			  "range",			  _("Range"),			 RELEASE_VERSION_0_61_07); // SVN r776

	ADD_VALUENODE(ValueNode_Switch,			  "switch",			  _("Switch"),			 RELEASE_VERSION_0_61_08); // SVN r923
	ADD_VALUENODE(ValueNode_Cos,			  "cos",			  _("Cos"),				 RELEASE_VERSION_0_61_08); // SVN r1111
	ADD_VALUENODE(ValueNode_Atan2,			  "atan2",			  _("aTan2"),			 RELEASE_VERSION_0_61_08); // SVN r1132
	ADD_VALUENODE(ValueNode_BLineRevTangent,  "blinerevtangent",  _("Reverse Tangent"),	 RELEASE_VERSION_0_61_08); // SVN r1162
	ADD_VALUENODE(ValueNode_TimeLoop,		  "timeloop",		  _("Time Loop"),		 RELEASE_VERSION_0_61_08); // SVN r1226
	ADD_VALUENODE(ValueNode_Reciprocal,		  "reciprocal",		  _("Reciprocal"),		 RELEASE_VERSION_0_61_08); // SVN r1238
	ADD_VALUENODE(ValueNode_Duplicate,		  "duplicate",		  _("Duplicate"),		 RELEASE_VERSION_0_61_08); // SVN r1267
	ADD_VALUENODE(ValueNode_Integer,		  "fromint",		  _("From Integer"),	 RELEASE_VERSION_0_61_08); // SVN r1267
	ADD_VALUENODE(ValueNode_Step,			  "step",			  _("Step"),			 RELEASE_VERSION_0_61_08); // SVN r1691
	ADD_VALUENODE(ValueNode_BLineCalcWidth,	  "blinecalcwidth",	  _("BLine Width"),		 RELEASE_VERSION_0_61_08); // SVN r1694

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
				synfig::info(_("%s: count:%d name:%s type:%s"),
					iter->first.get_string().c_str(),
					iter->second->count(),
					iter->second->get_name().c_str(),
					ValueBase::type_local_name(iter->second->get_type()).c_str()
				);
			else
				synfig::info(_("%s: id:%s count:%d name:%s type:%s"),
					iter->first.get_string().c_str(),
					iter->second->get_id().c_str(),
					iter->second->count(),
					iter->second->get_name().c_str(),
					ValueBase::type_local_name(iter->second->get_type()).c_str()
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

	if (!check_type(name, x.get_type()) &&
		// the Duplicate ValueNode is an exception - we don't want the
		// user creating it for themselves, so check_type() fails for
		// it even when it is valid
		!(name == "duplicate" && x.get_type() == ValueBase::TYPE_REAL))
	{
		error(_("Bad type: ValueNode '%s' doesn't accept type '%s'"), book()[name].local_name.c_str(), ValueBase::type_local_name(x.get_type()).c_str());
		return 0;
	}

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
}

void
ValueNode::on_changed()
{
	etl::loose_handle<Canvas> parent_canvas = get_parent_canvas();
	if(parent_canvas)
		do						// signal to all the ancestor canvases
			parent_canvas->signal_value_node_changed()(this);
		while (parent_canvas = parent_canvas->parent());
	else if(get_root_canvas())
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

String
ValueNode::get_description(bool show_exported_name)const
{
	String ret(_("ValueNode"));

	if (dynamic_cast<const LinkableValueNode*>(this))
		return (dynamic_cast<const LinkableValueNode*>(this))->get_description(-1, show_exported_name);

	if (show_exported_name && !is_exported())
		show_exported_name = false;

	if (show_exported_name)
		ret += strprintf(" (%s)", get_id().c_str());

	return ret;
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

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter)
		;

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

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter)
		;

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

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter)
		;

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
PlaceholderValueNode::operator()(Time /*t*/)const
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
	// printf("%s:%d clone()\n", __FILE__, __LINE__);
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)
																  {
																	  printf("VALUENODE FOUND VALUENODE\n");
																	  return x;
																  }}

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

String
LinkableValueNode::get_description(int index, bool show_exported_name)const
{
	String description;

	if (index == -1)
	{
		if (show_exported_name && is_exported())
			description += strprintf(" (%s)", get_id().c_str());
	}
	else
	{
		description = String(":") + link_local_name(index);

		if (show_exported_name)
		{
			ValueNode::LooseHandle link(get_link(index));
			if (link->is_exported())
				description += strprintf(" (%s)", link->get_id().c_str());
		}
	}

	const synfig::Node* node = this;
	LinkableValueNode::ConstHandle parent_linkable_vn = 0;

	// walk up through the valuenodes trying to find the layer at the top
	while (!node->parent_set.empty() && !dynamic_cast<const Layer*>(node))
	{
		LinkableValueNode::ConstHandle linkable_value_node(dynamic_cast<const LinkableValueNode*>(node));
		if (linkable_value_node)
		{
			String link;
			int cnt = linkable_value_node->link_count();
			for (int i = 0; i < cnt; i++)
				if (linkable_value_node->get_link(i) == parent_linkable_vn)
				{
					link = String(":") + linkable_value_node->link_local_name(i);
					break;
				}

			description = linkable_value_node->get_local_name() + link + (parent_linkable_vn?">":"") + description;
		}
		node = *node->parent_set.begin();
		parent_linkable_vn = linkable_value_node;
	}

	Layer::ConstHandle parent_layer(dynamic_cast<const Layer*>(node));
	if(parent_layer)
	{
		String param;
		const Layer::DynamicParamList &dynamic_param_list(parent_layer->dynamic_param_list());
		// loop to find the parameter in the dynamic parameter list - this gives us its name
		for (Layer::DynamicParamList::const_iterator iter = dynamic_param_list.begin(); iter != dynamic_param_list.end(); iter++)
			if (iter->second == parent_linkable_vn)
				param = String(":") + parent_layer->get_param_local_name(iter->first);
		description = strprintf("(%s)%s>%s",
								parent_layer->get_non_empty_description().c_str(),
								param.c_str(),
								description.c_str());
	}

	return description;
}
