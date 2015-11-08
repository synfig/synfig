/* === S Y N F I G ========================================================= */
/*!	\file valuenode.cpp
**	\brief Implementation of the "Placeholder" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
**  Copyright (c) 2008, 2011 Carlos López
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valuenode.h"
#include "general.h"
#include <synfig/localization.h>
#include "canvas.h"
#include "releases.h"

#include "valuenodes/valuenode_const.h"
#include "valuenodes/valuenode_linear.h"
#include "valuenodes/valuenode_composite.h"
#include "valuenodes/valuenode_reference.h"
#include "valuenodes/valuenode_boneinfluence.h"
#include "valuenodes/valuenode_boneweightpair.h"
#include "valuenodes/valuenode_bone.h"
#include "valuenodes/valuenode_bonelink.h"
#include "valuenodes/valuenode_greyed.h"
#include "valuenodes/valuenode_scale.h"
#include "valuenodes/valuenode_blinecalctangent.h"
#include "valuenodes/valuenode_blinecalcvertex.h"
#include "valuenodes/valuenode_blinecalcwidth.h"
#include "valuenodes/valuenode_blinereversetangent.h"
#include "valuenodes/valuenode_segcalctangent.h"
#include "valuenodes/valuenode_segcalcvertex.h"
#include "valuenodes/valuenode_repeat_gradient.h"
#include "valuenodes/valuenode_stripes.h"
#include "valuenodes/valuenode_range.h"
#include "valuenodes/valuenode_add.h"
#include "valuenodes/valuenode_subtract.h"
#include "valuenodes/valuenode_timedswap.h"
#include "valuenodes/valuenode_twotone.h"
#include "valuenodes/valuenode_bline.h"
#include "valuenodes/valuenode_staticlist.h"
#include "valuenodes/valuenode_wplist.h"
#include "valuenodes/valuenode_dilist.h"
#include "valuenodes/valuenode_dynamiclist.h"
#include "valuenodes/valuenode_radialcomposite.h"
#include "valuenodes/valuenode_gradientrotate.h"
#include "valuenodes/valuenode_sine.h"
#include "valuenodes/valuenode_cos.h"
#include "valuenodes/valuenode_atan2.h"
#include "valuenodes/valuenode_exp.h"
#include "valuenodes/valuenode_switch.h"
#include "valuenodes/valuenode_timeloop.h"
#include "valuenodes/valuenode_reciprocal.h"
#include "valuenodes/valuenode_duplicate.h"
#include "valuenodes/valuenode_integer.h"
#include "valuenodes/valuenode_step.h"
#include "valuenodes/valuenode_vectorangle.h"
#include "valuenodes/valuenode_vectorlength.h"
#include "valuenodes/valuenode_vectorx.h"
#include "valuenodes/valuenode_vectory.h"
#include "valuenodes/valuenode_gradientcolor.h"
#include "valuenodes/valuenode_dotproduct.h"
#include "valuenodes/valuenode_timestring.h"
#include "valuenodes/valuenode_realstring.h"
#include "valuenodes/valuenode_join.h"
#include "valuenodes/valuenode_anglestring.h"
#include "valuenodes/valuenode_intstring.h"
#include "valuenodes/valuenode_log.h"
#include "valuenodes/valuenode_pow.h"
#include "valuenodes/valuenode_compare.h"
#include "valuenodes/valuenode_not.h"
#include "valuenodes/valuenode_and.h"
#include "valuenodes/valuenode_or.h"
#include "valuenodes/valuenode_real.h"
#include "valuenodes/valuenode_average.h"
#include "valuenodes/valuenode_dynamic.h"
#include "valuenodes/valuenode_derivative.h"
#include "valuenodes/valuenode_weightedaverage.h"
#include "valuenodes/valuenode_reverse.h"

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

void
ValueNode::breakpoint()
{
	return;
}

bool
ValueNode::subsys_init()
{
	book_=new LinkableValueNode::Book();

#define ADD_VALUENODE_CREATE(class,name,local,version,create)									\
	(*book_)[name].factory=reinterpret_cast<LinkableValueNode::Factory>(&class::create);		\
	(*book_)[name].check_type=&class::check_type;												\
	(*book_)[name].local_name=local;															\
	(*book_)[name].release_version=version
#define ADD_VALUENODE(class,name,local,version)		ADD_VALUENODE_CREATE(class,name,local,version,create)
#define ADD_VALUENODE2(class,name,local,version)	ADD_VALUENODE_CREATE(class,name,local,version,create_from)

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
	ADD_VALUENODE(ValueNode_BLine,			  "bline",			  _("Spline"),			 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE2(ValueNode_DynamicList,	  "dynamic_list",	  _("Dynamic List"),	 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_GradientRotate,	  "gradient_rotate",  _("Gradient Rotate"),	 RELEASE_VERSION_0_61_06);
	ADD_VALUENODE(ValueNode_Sine,			  "sine",			  _("Sine"),			 RELEASE_VERSION_0_61_06);

	ADD_VALUENODE(ValueNode_TimedSwap,		  "timed_swap",		  _("Timed Swap"),		 RELEASE_VERSION_0_61_07); // SVN r610
	ADD_VALUENODE(ValueNode_Repeat_Gradient,  "repeat_gradient",  _("Repeat Gradient"),	 RELEASE_VERSION_0_61_07); // SVN r666
	ADD_VALUENODE(ValueNode_Exp,			  "exp",			  _("Exponential"),		 RELEASE_VERSION_0_61_07); // SVN r739
	ADD_VALUENODE(ValueNode_Add,			  "add",			  _("Add"),				 RELEASE_VERSION_0_61_07); // SVN r742
	ADD_VALUENODE(ValueNode_BLineCalcTangent, "blinecalctangent", _("Spline Tangent"),	 RELEASE_VERSION_0_61_07); // SVN r744
	ADD_VALUENODE(ValueNode_BLineCalcVertex,  "blinecalcvertex",  _("Spline Vertex"),	 RELEASE_VERSION_0_61_07); // SVN r744
	ADD_VALUENODE(ValueNode_Range,			  "range",			  _("Range"),			 RELEASE_VERSION_0_61_07); // SVN r776

	ADD_VALUENODE(ValueNode_Switch,			  "switch",			  _("Switch"),			 RELEASE_VERSION_0_61_08); // SVN r923
	ADD_VALUENODE(ValueNode_Cos,			  "cos",			  _("Cos"),				 RELEASE_VERSION_0_61_08); // SVN r1111
	ADD_VALUENODE(ValueNode_Atan2,			  "atan2",			  _("aTan2"),			 RELEASE_VERSION_0_61_08); // SVN r1132
	ADD_VALUENODE(ValueNode_BLineRevTangent,  "blinerevtangent",  _("Reverse Tangent"),	 RELEASE_VERSION_0_61_08); // SVN r1162
	ADD_VALUENODE(ValueNode_TimeLoop,		  "timeloop",		  _("Time Loop"),		 RELEASE_VERSION_0_61_08); // SVN r1226
	ADD_VALUENODE(ValueNode_Reciprocal,		  "reciprocal",		  _("Reciprocal"),		 RELEASE_VERSION_0_61_08); // SVN r1238
	ADD_VALUENODE(ValueNode_Duplicate,		  "duplicate",		  _("Duplicate"),		 RELEASE_VERSION_0_61_08); // SVN r1267
	ADD_VALUENODE(ValueNode_Integer,		  "fromint",		  _("Integer"),			 RELEASE_VERSION_0_61_08); // SVN r1267
	ADD_VALUENODE(ValueNode_Step,			  "step",			  _("Step"),			 RELEASE_VERSION_0_61_08); // SVN r1691
	ADD_VALUENODE(ValueNode_BLineCalcWidth,	  "blinecalcwidth",	  _("Spline Width"),		 RELEASE_VERSION_0_61_08); // SVN r1694

	ADD_VALUENODE(ValueNode_VectorAngle,	  "vectorangle",	  _("Vector Angle"),	 RELEASE_VERSION_0_61_09); // SVN r1880
	ADD_VALUENODE(ValueNode_VectorLength,	  "vectorlength",	  _("Vector Length"),	 RELEASE_VERSION_0_61_09); // SVN r1881
	ADD_VALUENODE(ValueNode_VectorX,		  "vectorx",		  _("Vector X"),		 RELEASE_VERSION_0_61_09); // SVN r1882
	ADD_VALUENODE(ValueNode_VectorY,		  "vectory",		  _("Vector Y"),		 RELEASE_VERSION_0_61_09); // SVN r1882
	ADD_VALUENODE(ValueNode_GradientColor,	  "gradientcolor",	  _("Gradient Color"),	 RELEASE_VERSION_0_61_09); // SVN r1885
	ADD_VALUENODE(ValueNode_DotProduct,		  "dotproduct",		  _("Dot Product"),		 RELEASE_VERSION_0_61_09); // SVN r1891
	ADD_VALUENODE(ValueNode_TimeString,		  "timestring",		  _("Time String"),		 RELEASE_VERSION_0_61_09); // SVN r2000
	ADD_VALUENODE(ValueNode_Real,		  	  "fromreal",		  _("Real"),		 	 RELEASE_VERSION_0_64_0); // git 2013-01-12
	ADD_VALUENODE(ValueNode_RealString,		  "realstring",		  _("Real String"),		 RELEASE_VERSION_0_61_09); // SVN r2003
	ADD_VALUENODE(ValueNode_Join,			  "join",			  _("Joined List"),		 RELEASE_VERSION_0_61_09); // SVN r2007
	ADD_VALUENODE(ValueNode_AngleString,	  "anglestring",	  _("Angle String"),	 RELEASE_VERSION_0_61_09); // SVN r2010
	ADD_VALUENODE(ValueNode_IntString,		  "intstring",		  _("Int String"),		 RELEASE_VERSION_0_61_09); // SVN r2010
	ADD_VALUENODE(ValueNode_Logarithm,		  "logarithm",		  _("Logarithm"),		 RELEASE_VERSION_0_61_09); // SVN r2034
	ADD_VALUENODE(ValueNode_Greyed,			  "greyed",			  _("Greyed"),			 RELEASE_VERSION_0_62_00); // SVN r2305
	ADD_VALUENODE(ValueNode_Pow,		      "power",		      _("Power"),		     RELEASE_VERSION_0_62_00); // SVN r2362
	ADD_VALUENODE(ValueNode_Compare,		  "compare",  	 	  _("Compare"),			 RELEASE_VERSION_0_62_00); // SVN r2364
	ADD_VALUENODE(ValueNode_Not,		      "not",			  _("Not"),				 RELEASE_VERSION_0_62_00); // SVN r2364
	ADD_VALUENODE(ValueNode_And,		      "and",			  _("And"),				 RELEASE_VERSION_0_62_00); // SVN r2364
	ADD_VALUENODE(ValueNode_Or,		          "or",			  _("Or"),					 RELEASE_VERSION_0_62_00); // SVN r2364

	ADD_VALUENODE(ValueNode_BoneInfluence,	  "boneinfluence",	  _("Bone Influence"),	 RELEASE_VERSION_0_62_00); 
	ADD_VALUENODE(ValueNode_Bone,			  "bone",			  _("Bone"),			 RELEASE_VERSION_0_62_00); 
	ADD_VALUENODE(ValueNode_Bone_Root,		  "bone_root",		  _("Root Bone"),		 RELEASE_VERSION_0_62_00); 
	ADD_VALUENODE2(ValueNode_StaticList,	  "static_list",	  _("Static List"),		 RELEASE_VERSION_0_62_00); 
	ADD_VALUENODE(ValueNode_BoneWeightPair,	  "boneweightpair",	  _("Bone Weight Pair"), RELEASE_VERSION_0_62_00); 
	ADD_VALUENODE(ValueNode_BoneLink,		  "bone_link",		  _("Bone Link"),		 RELEASE_VERSION_1_0);

	ADD_VALUENODE(ValueNode_WPList,           "wplist",           _("WPList"),           RELEASE_VERSION_0_63_00);
	ADD_VALUENODE(ValueNode_DIList,           "dilist",           _("DIList"),           RELEASE_VERSION_0_63_01);

	ADD_VALUENODE(ValueNode_Average,		  "average",		  _("Average"),			 RELEASE_VERSION_1_0);
	ADD_VALUENODE(ValueNode_WeightedAverage,  "weighted_average", _("Weighted Average"), RELEASE_VERSION_1_0);
	
	ADD_VALUENODE(ValueNode_Dynamic,           "dynamic",         _("Dynamic"),          RELEASE_VERSION_1_0);
	ADD_VALUENODE(ValueNode_Derivative,        "derivative",      _("Derivative"),       RELEASE_VERSION_1_0);
	
	ADD_VALUENODE(ValueNode_Reverse,           "reverse",         _("Reverse"),          RELEASE_VERSION_1_0_2);

#undef ADD_VALUENODE_CREATE
#undef ADD_VALUENODE
#undef ADD_VALUENODE2

	return true;
}

bool
ValueNode::subsys_stop()
{
	delete book_;
	return true;
}

ValueNode::ValueNode(Type &type):type(&type)
{
	value_node_count++;
}

LinkableValueNode::Book&
LinkableValueNode::book()
{
	return *book_;
}

LinkableValueNode::Handle
LinkableValueNode::create(const String &name, const ValueBase& x, Canvas::LooseHandle canvas)
{
	if(!book().count(name))
		return 0;

	if (!check_type(name, x.get_type()))
	{
		error(_("Bad type: ValueNode '%s' doesn't accept type '%s'"), book()[name].local_name.c_str(), x.get_type().description.local_name.c_str());
		return 0;
	}

	return book()[name].factory(x,canvas);
}

bool
LinkableValueNode::check_type(const String &name, Type &x)
{
	// the BoneRoot and Duplicate ValueNodes are exceptions - we don't want the
	// user creating them for themselves, so check_type() fails for
	// them even when it is valid
	if((name == "bone_root" && x == type_bone_object) ||
	   (name == "duplicate" && x == type_real))
		return true;

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
		// Fix 2412072: remove the previous link from the parent_set unless one of the other links is also
		// using it when we convert a value to 'switch', both 'on' and 'off' are linked to the same valuenode
		// if we then disconnect one of the two, the one we disconnect is set to be a new valuenode_const
		// and the previously shared value is removed from the parent set even though the other is still
		// using it
		if(previous)
		{
			int size = link_count(), index;
			for (index=0; index < size; ++index)
			{
				if (i == index) continue;
				if (get_link(index) == previous)
					break;
			}
			if (index == size)
				remove_child(previous.get());
		}
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
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
		printf("%s:%d ValueNode::on_changed()\n", __FILE__, __LINE__);

	etl::loose_handle<Canvas> parent_canvas = get_parent_canvas();
	if(parent_canvas)
		do						// signal to all the ancestor canvases
			parent_canvas->signal_value_node_changed()(this);
		while ( (parent_canvas = parent_canvas->parent()) );
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
	if (dynamic_cast<const LinkableValueNode*>(this))
		return dynamic_cast<const LinkableValueNode*>(this)->get_description(-1, show_exported_name);

	String ret(_("ValueNode"));

	if (dynamic_cast<const LinkableValueNode*>(this))
		return (dynamic_cast<const LinkableValueNode*>(this))->get_description(-1, show_exported_name);

	if (show_exported_name && !is_exported())
		show_exported_name = false;

	if (show_exported_name)
		ret += strprintf(" (%s)", get_id().c_str());

	return ret;
}

bool
ValueNode::is_descendant(ValueNode::Handle value_node_dest)
{
    if(!value_node_dest)
        return false;
    if(Handle(this) == value_node_dest)
        return true;

    //! loop through the parents of each node in current_nodes
    set<Node*> node_parents(value_node_dest->parent_set);
    ValueNode::Handle value_node_parent = NULL;
    for (set<Node*>::iterator iter = node_parents.begin(); iter != node_parents.end(); iter++)
    {
        value_node_parent = ValueNode::Handle::cast_dynamic(*iter);
        if(Handle(this) == value_node_parent)
            break;
    }

    return value_node_dest->parent_count() ? is_descendant(value_node_parent) : false;
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
ValueNodeList::find(const String &id, bool might_fail)
{
	iterator iter;

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter)
		;

	if(iter==end())
	{
		if (!might_fail) ValueNode::breakpoint();
		throw Exception::IDNotFound("ValueNode in ValueNodeList: "+id);
	}

	return *iter;
}

ValueNode::ConstHandle
ValueNodeList::find(const String &id, bool might_fail)const
{
	const_iterator iter;

	if(id.empty())
		throw Exception::IDNotFound("Empty ID");

	for(iter=begin();iter!=end() && id!=(*iter)->get_id();++iter)
		;

	if(iter==end())
	{
		if (!might_fail) ValueNode::breakpoint();
		throw Exception::IDNotFound("ValueNode in ValueNodeList: "+id);
	}

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
		value_node=find(id, true);
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
		ValueNode::RHandle other_value_node=find(value_node->get_id(), true);
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

String
PlaceholderValueNode::get_string()const
{
	return String("PlaceholderValueNode: ") + get_guid().get_string();
}

ValueNode::Handle
PlaceholderValueNode::clone(Canvas::LooseHandle canvas, const GUID& deriv_guid)const
{
	ValueNode* ret(new PlaceholderValueNode());
	ret->set_guid(get_guid()^deriv_guid);
	ret->set_parent_canvas(canvas);
	return ret;
}

PlaceholderValueNode::Handle
PlaceholderValueNode::create(Type &type)
{
	if (getenv("SYNFIG_DEBUG_PLACEHOLDER_VALUENODE"))
		printf("%s:%d PlaceholderValueNode::create\n", __FILE__, __LINE__);
	return new PlaceholderValueNode(type);
}

ValueBase
PlaceholderValueNode::operator()(Time /*t*/)const
{
	assert(0);
	return ValueBase();
}

PlaceholderValueNode::PlaceholderValueNode(Type &type):
	ValueNode(type)
{
}

ValueNode::Handle
LinkableValueNode::clone(Canvas::LooseHandle canvas, const GUID& deriv_guid)const
{
	{
		ValueNode* x(find_value_node(get_guid()^deriv_guid).get());
		if(x)
			return x;
	}

	int i;
	LinkableValueNode *ret=create_new();
	ret->set_guid(get_guid()^deriv_guid);

	for(i=0;i<link_count();i++)
	{
		ValueNode::Handle link=get_link_vfunc(i);
		if(!link->is_exported())
		{
			ValueNode::Handle value_node(find_value_node(link->get_guid()^deriv_guid));
			if(!value_node)
				value_node=link->clone(canvas, deriv_guid);
			ret->set_link(i,value_node);
		}
		else
			ret->set_link(i,link);
	}

	ret->set_parent_canvas(canvas);
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

etl::loose_handle<Canvas>
ValueNode::get_parent_canvas()const
{
	if (getenv("SYNFIG_DEBUG_GET_PARENT_CANVAS"))
		printf("%s:%d get_parent_canvas of %lx is %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(canvas_.get()));

	return canvas_;
}

etl::loose_handle<Canvas>
ValueNode::get_root_canvas()const
{
	if (getenv("SYNFIG_DEBUG_GET_PARENT_CANVAS"))
		printf("%s:%d get_root_canvas of %lx is %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(root_canvas_.get()));

	return root_canvas_;
}

etl::loose_handle<Canvas>
ValueNode::get_non_inline_ancestor_canvas()const
{
	etl::loose_handle<Canvas> parent(get_parent_canvas());

	if (parent)
	{
		etl::loose_handle<Canvas> ret(parent->get_non_inline_ancestor());

		if (getenv("SYNFIG_DEBUG_GET_PARENT_CANVAS"))
			printf("%s:%d get_non_inline_ancestor_canvas of %lx is %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(ret.get()));

		return ret;
	}

	return parent;
}

void
ValueNode::set_parent_canvas(etl::loose_handle<Canvas> x)
{
	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set_parent_canvas of %lx to %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(x.get()));

	canvas_=x;

	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d now %lx\n", __FILE__, __LINE__, uintptr_t(canvas_.get()));

	if(x) set_root_canvas(x);
}

void
ValueNode::set_root_canvas(etl::loose_handle<Canvas> x)
{
	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set_root_canvas of %lx to %lx - ", __FILE__, __LINE__, uintptr_t(this), uintptr_t(x.get()));

	root_canvas_=x->get_root();

	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("now %lx\n", uintptr_t(root_canvas_.get()));
}

String
ValueNode::get_string()const
{
	return String("ValueNode: ") + get_description();
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
		description = strprintf(" Linkable ValueNode (%s)", get_local_name().c_str());
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

String
LinkableValueNode::get_description(bool show_exported_name)const
{
	return get_description(-1, show_exported_name);
}

String
LinkableValueNode::link_name(int i)const
{
	Vocab vocab(get_children_vocab());
	Vocab::iterator iter(vocab.begin());
	int j=0;
	for(;iter!=vocab.end() && j<i; iter++, j++) {};
	return iter!=vocab.end()?iter->get_name():String();
}

String
LinkableValueNode::link_local_name(int i)const
{
	Vocab vocab(get_children_vocab());
	Vocab::iterator iter(vocab.begin());
	int j=0;
	for(;iter!=vocab.end() && j<i; iter++, j++){};
	return iter!=vocab.end()?iter->get_local_name():String();
}

int
LinkableValueNode::get_link_index_from_name(const String &name)const
{
	Vocab vocab(get_children_vocab());
	Vocab::iterator iter(vocab.begin());
	int j=0;
	for(; iter!=vocab.end(); iter++, j++)
		if(iter->get_name()==name) return j;
	throw Exception::BadLinkName(name);
}

int
LinkableValueNode::link_count()const
{
	return get_children_vocab().size();
}

LinkableValueNode::Vocab
LinkableValueNode::get_children_vocab()const
{
	return get_children_vocab_vfunc();
}

void
LinkableValueNode::set_children_vocab(const Vocab &newvocab)
{
	children_vocab.assign(newvocab.begin(),newvocab.end());
}

void
LinkableValueNode::set_root_canvas(etl::loose_handle<Canvas> x)
{
	ValueNode::set_root_canvas(x);
	for(int i = 0; i < link_count(); ++i)
		get_link(i)->set_root_canvas(x);
}
