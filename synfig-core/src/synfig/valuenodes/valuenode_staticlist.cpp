/* === S Y N F I G ========================================================= */
/*!	\file valuenode_staticlist.cpp
**	\brief Implementation of the "StaticList" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "valuenode_staticlist.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include "valuenode_bone.h"
#include <synfig/boneweightpair.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <vector>
#include <list>
#include <algorithm>
#include <synfig/canvas.h>
#include <synfig/pair.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_StaticList, RELEASE_VERSION_0_62_00, "static_list", "Static List")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_StaticList::ListEntry
ValueNode_StaticList::create_list_entry(int index, Time time, Real origin) // line 137
{
	ValueNode_StaticList::ListEntry ret;

	synfig::ValueBase prev,next;

	index=index%link_count();

	assert(index>=0);

	next=(*list[index])(time);

	if(index!=0)
		prev=(*list[index-1])(time);
	else if(get_loop())
		prev=(*list[link_count()-1])(time);
	else
		prev=next;

	Type &type(get_contained_type());
	if (type == type_vector)
	{
		Vector a(prev.get(Vector())), b(next.get(Vector()));
		ret=ValueNode_Const::create((b-a)*origin+a);
	}
	else
	if (type == type_real)
	{
		Real a(prev.get(Real())), b(next.get(Real()));
		ret=ValueNode_Const::create((b-a)*origin+a);
	}
	else
	if (type == type_color)
	{
		Color a(prev.get(Color())), b(next.get(Color()));
		ret=ValueNode_Composite::create((b-a)*origin+a);
	}
	else
	if (type == type_angle)
	{
		Angle a(prev.get(Angle())), b(next.get(Angle()));
		ret=ValueNode_Const::create((b-a)*origin+a);
	}
	else
	if (type == type_time)
	{
		Time a(prev.get(Time())), b(next.get(Time()));
		ret=ValueNode_Const::create((b-a)*origin+a);
	}
	else
	if (type == type_bone_object)
	{
		ValueNode::Handle value_node(list[index]);
		if (ValueNode_Bone::Handle value_node_bone = ValueNode_Bone::Handle::cast_dynamic(value_node))
		{
			Bone new_bone;
			new_bone.set_parent(value_node_bone.get());

			Real length(next.get(Bone()).get_length());
			Real width(next.get(Bone()).get_tipwidth());
			Real depth(next.get(Bone()).get_depth());
			new_bone.set_origin(Point(1.1*length,0));
			new_bone.set_width(width);
			new_bone.set_tipwidth(width);
			new_bone.set_depth(depth);

			ret=ValueNode_Const::create(new_bone, get_parent_canvas());
		}
		else
			ret=ValueNode_Const::create(type_bone_object, get_parent_canvas());
	}
	else
	if (type == type_bone_weight_pair)
	{
		ret=ValueNode_Const::create(BoneWeightPair(Bone(), next.get(BoneWeightPair()).get_weight()), get_parent_canvas());
	}
	else
	if (type == types_namespace::TypePair<Bone, Bone>::instance)
	{
		ValueNode::Handle value_node(list[index]);
		if (ValueNode_Composite::Handle value_node_composite = ValueNode_Composite::Handle::cast_dynamic(value_node))
		{
			ValueNode_Bone::Handle fisrt_bone_node = ValueNode_Bone::Handle::cast_dynamic(value_node_composite->get_link("first"));
			ValueNode_Bone::Handle second_bone_node = ValueNode_Bone::Handle::cast_dynamic(value_node_composite->get_link("second"));
			if (fisrt_bone_node && second_bone_node)
			{
				std::pair<Bone, Bone> new_pair;

				{ // first
					ValueNode_Bone::Handle &value_node_bone = fisrt_bone_node;
					Bone &new_bone = new_pair.first;

					const Bone &bone = (*value_node_bone)(time).get(Bone());
					new_bone.set_parent(value_node_bone.get());
					Real length(bone.get_length());
					Real width(bone.get_tipwidth());
					Real depth(bone.get_depth());
					new_bone.set_origin(Point(1.1*length,0));
					new_bone.set_width(width);
					new_bone.set_tipwidth(width);
					new_bone.set_depth(depth);
				}

				{ // second
					ValueNode_Bone::Handle &value_node_bone = second_bone_node;
					Bone &new_bone = new_pair.second;

					const Bone &bone = (*value_node_bone)(time).get(Bone());
					new_bone.set_parent(value_node_bone.get());
					Real length(bone.get_length());
					Real width(bone.get_tipwidth());
					Real depth(bone.get_depth());
					new_bone.set_origin(Point(1.1*length,0));
					new_bone.set_width(width);
					new_bone.set_tipwidth(width);
					new_bone.set_depth(depth);
				}

				ret=ValueNode_Const::create(new_pair, get_parent_canvas());
			}
		}
		else
			ret=ValueNode_Const::create(type_bone_object, get_parent_canvas());
	}
	else
	{
		ret=ValueNode_Const::create(get_contained_type(), get_parent_canvas());
	}

	ret->set_parent_canvas(get_parent_canvas());

	return ret;
}

void
ValueNode_StaticList::add(const ValueNode::Handle &value_node, int index) // line 470
{
	if(index<0 || index>=(int)list.size())
	{
		if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
			printf("%s:%d vvv adding valuenode to end of static list\n", __FILE__, __LINE__);

		list.push_back(value_node);

		if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
			printf("%s:%d ^^^ done adding valuenode\n", __FILE__, __LINE__);
	}
	else
	{
		if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
			printf("%s:%d vvv inserting valuenode into static list at %d\n", __FILE__, __LINE__, index);

		list.insert(list.begin()+index,value_node);

		if (getenv("SYNFIG_DEBUG_BONE_REFCOUNT"))
			printf("%s:%d ^^^ done inserting valuenode\n", __FILE__, __LINE__);
	}

	add_child(value_node.get());
	//changed();

	if(get_parent_canvas())
		get_parent_canvas()->signal_value_node_child_added()(this,value_node);
	else if(get_root_canvas() && get_parent_canvas())
		get_root_canvas()->signal_value_node_child_added()(this,value_node);
}

void
ValueNode_StaticList::erase(const ListEntry &value_node_) // line 513
{
	ValueNode::Handle value_node(value_node_);

	assert(value_node);
	if(!value_node)
		throw String("ValueNode_StaticList::erase(): Passed bad value node");

	std::vector<ReplaceableListEntry>::iterator iter;
	for(iter=list.begin();iter!=list.end();++iter)
		if(*iter==value_node)
		{
			list.erase(iter);
			if(value_node)
			{
				remove_child(value_node.get());
				// changed to fix bug 1420091 - it seems that when a .sif file containing a bline layer encapsulated inside
				// another layer, get_parent_canvas() is false and get_root_canvas() is true, but when we come to erase a
				// vertex, both are true.  So the signal is sent to the parent, but the signal wasn't sent to the parent
				// when it was added.  This probably isn't the right fix, but it seems to work for now.  Note that the same
				// strange "if (X) else if (Y && X)" code is also present in the two previous functions, above.

				// if(get_parent_canvas())
				// 	get_parent_canvas()->signal_value_node_child_removed()(this,value_node);
				// else if(get_root_canvas() && get_parent_canvas())
				//	get_root_canvas()->signal_value_node_child_removed()(this,value_node);
				if(get_non_inline_ancestor_canvas())
					get_non_inline_ancestor_canvas()->invoke_signal_value_node_child_removed(this,value_node);
				else
					printf("%s:%d == can't get non_inline_ancestor_canvas - parent canvas = %lx\n", __FILE__, __LINE__, uintptr_t(get_parent_canvas().get()));
			}
			break;
		}
}

ValueNode_StaticList::ValueNode_StaticList(Type &container_type, Canvas::LooseHandle canvas): // line 548
	LinkableValueNode(type_list),
	container_type(&container_type),
	loop_(false)
{
	if (getenv("SYNFIG_DEBUG_STATICLIST_CONSTRUCTORS"))
		printf("%s:%d ValueNode_StaticList::ValueNode_StaticList() construct %lx\n", __FILE__, __LINE__, uintptr_t(this));

	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set parent canvas for static_list %lx to %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(canvas.get()));
	set_parent_canvas(canvas);
}

ValueNode_StaticList::~ValueNode_StaticList()
{
#ifdef _DEBUG
	if (getenv("SYNFIG_DEBUG_STATICLIST_CONSTRUCTORS"))
	{
		printf("\n%s:%d ------------------------------------------------------------------------\n", __FILE__, __LINE__);
		printf("%s:%d ~ValueNode_StaticList()\n", __FILE__, __LINE__);
		printf("%s:%d ------------------------------------------------------------------------\n\n", __FILE__, __LINE__);
	}
#endif

	ValueNode_Bone::show_bone_map(get_root_canvas(), __FILE__, __LINE__, "deleting staticlist");
	unlink_all();
}

ValueNode_StaticList::Handle
ValueNode_StaticList::create_on_canvas(Type &type, Canvas::LooseHandle canvas)
{
	return new ValueNode_StaticList(type, canvas);
}

ValueNode_StaticList*
ValueNode_StaticList::create(const ValueBase &value)
{
	vector<ValueBase> value_list(value.get_list());

	vector<ValueBase>::iterator iter;

	if(value_list.empty())
		return 0;

	Type &type(value.get_contained_type());
	ValueNode_StaticList* value_node(new ValueNode_StaticList(type));

	// when creating a list of vectors, start it off being looped.
	// I think the only time this is used if for creating polygons,
	// and we want them to be looped by default
	if (value_node->get_contained_type() == type_vector)
		value_node->set_loop(true);

	for(iter=value_list.begin();iter!=value_list.end();++iter)
	{
		// TODO: both cases is identical, see constructor of ValueNode_Const
		if (type == type_bone_object)
		{
			ValueNode::Handle item(ValueNode_Bone::create(*iter));
			value_node->add(item);
		}
		else
		{
			ValueNode::Handle item(ValueNode_Const::create(*iter));
			value_node->add(item);
		}
	}
	return value_node;
}

ValueBase
ValueNode_StaticList::operator()(Time t)const // line 596
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<ValueBase> ret_list;
	std::vector<ReplaceableListEntry>::const_iterator iter;

	assert(*container_type != type_nil);

	for(iter=list.begin();iter!=list.end();++iter)
		if((*iter)->get_type()==*container_type)
			ret_list.push_back((**iter)(t));
		else
			synfig::warning(string("ValueNode_StaticList::operator()():")+_("List type/item type mismatch, throwing away mismatch"));

	if(list.empty())
		synfig::warning(string("ValueNode_StaticList::operator()():")+_("No entries in list"));
	else
	if(ret_list.empty())
		synfig::warning(string("ValueNode_StaticList::operator()():")+_("No entries in ret_list"));

	return ret_list;
}

bool
ValueNode_StaticList::set_link_vfunc(int i,ValueNode::Handle x) // line 628
{
	assert(i>=0);

	if((unsigned)i>=list.size())
		return false;
	if(x->get_type()!=*container_type)
		return false;
	list[i]=x;
	return true;
}

ValueNode::LooseHandle
ValueNode_StaticList::get_link_vfunc(int i)const // line 641
{
	assert(i>=0);

	if((unsigned)i>=list.size())
		return 0;
	return list[i];
}

int
ValueNode_StaticList::link_count()const // line 651
{
	return list.size();
}

String
ValueNode_StaticList::link_local_name(int i)const // line 657
{
	assert(i>=0 && i<link_count());

	return etl::strprintf(_("Item %03d"),i+1);
}

ValueNode::Handle
ValueNode_StaticList::clone(Canvas::LooseHandle canvas, const GUID& deriv_guid)const
{
	ValueNode_Bone::show_bone_map(get_root_canvas(), __FILE__, __LINE__, "before cloning staticlist");

	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }

	ValueNode_StaticList* ret=dynamic_cast<ValueNode_StaticList*>(create_new());
	ret->set_guid(get_guid()^deriv_guid);

	puts(get_contained_type().description.name.c_str());

	std::map<const ValueNode*, ValueNode::Handle> clone_map;
	const bool is_actually_skeleton_tree = get_contained_type() == type_bone_object;
	const types_namespace::TypePair<Bone, Bone>& type_bone_pair = types_namespace::TypePair<Bone, Bone>::instance;
	const bool is_actually_skeleton_deform = get_contained_type() == type_bone_pair;

	for(std::vector<ReplaceableListEntry>::const_iterator iter=list.begin();iter!=list.end();++iter)
		if((*iter)->is_exported())
			ret->add(*iter);
		else {
			ValueNode::Handle item_clone = (*iter)->clone(canvas, deriv_guid);
			ret->add(item_clone);
			if (is_actually_skeleton_tree)
				clone_map[iter->get()] = item_clone;
			else if (is_actually_skeleton_deform) {
				ValueNode_Composite::Handle value_node_composite =
					ValueNode_Composite::Handle::cast_dynamic(*iter);
				if (value_node_composite) {
					clone_map[value_node_composite->get_link("first").get()] = ValueNode_Composite::Handle::cast_static(item_clone)->get_link("first").get();
					clone_map[value_node_composite->get_link("second").get()] = ValueNode_Composite::Handle::cast_static(item_clone)->get_link("second").get();
				}
			}
		}

	if (is_actually_skeleton_tree || is_actually_skeleton_deform) {
		// fix referencing to old items (eg. bone parenting)
		for (std::vector<ReplaceableListEntry>::const_iterator iter=list.begin(), ret_iter=ret->list.begin(); iter!=list.end();++iter, ++ret_iter) {
			if ((*iter)->is_exported())
				continue;
			const Type& type = (*iter)->get_type();
			if (type == type_bone_object || type == type_bone_pair)
				ValueNode_Bone::fix_bones_referenced_by(*iter, *ret_iter, false, clone_map);
		}
	}

	ret->set_loop(get_loop());
	ret->set_parent_canvas(canvas);

	ValueNode_Bone::show_bone_map(get_root_canvas(), __FILE__, __LINE__, "after cloning staticlist");

	return ret;
}

String
ValueNode_StaticList::link_name(int i)const // line 693
{
	return strprintf("item%04d",i);
}

int
ValueNode_StaticList::get_link_index_from_name(const String &name)const // line 699
{
	for(int i = 0; i < link_count(); ++i)
		if (link_name(i) == name) return i;
	throw Exception::BadLinkName(name);
}



bool
ValueNode_StaticList::check_type(Type &type) // line 717
{
	return type==type_list;
}

LinkableValueNode::Vocab
ValueNode_StaticList::get_children_vocab_vfunc() const
{
	LinkableValueNode::Vocab ret;
	for(unsigned int i=0; i<list.size();i++)
	{
		ret.push_back(ParamDesc(ValueBase(),strprintf("item%04d",i))
			.set_local_name(etl::strprintf(_("Item %03d"),i+1))
		);
	}

	return ret;
}

void
ValueNode_StaticList::set_member_canvas(etl::loose_handle<Canvas> canvas) // line 723
{
	for (vector<ReplaceableListEntry>::iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
			printf("%s:%d set parent canvas of member (%lx) to (%lx)\n", __FILE__, __LINE__, uintptr_t((*iter).get()), uintptr_t(canvas.get()));
		(*iter)->set_parent_canvas(canvas);
	}
}

Type&
ValueNode_StaticList::get_contained_type()const // line 730
{
	return *container_type;
}

LinkableValueNode*
ValueNode_StaticList::create_new() const
{
	return new ValueNode_StaticList(*container_type);
}

#ifdef _DEBUG
void
ValueNode_StaticList::ref()const
{
	if (getenv("SYNFIG_DEBUG_STATICLIST_REFCOUNT"))
		printf("%s:%d %lx   ref staticlist %*s -> %2d\n", __FILE__, __LINE__, uintptr_t(this), (count()*2), "", count()+1);

	LinkableValueNode::ref();
}

bool
ValueNode_StaticList::unref()const
{
	if (getenv("SYNFIG_DEBUG_STATICLIST_REFCOUNT"))
		printf("%s:%d %lx unref staticlist %*s%2d <-\n", __FILE__, __LINE__, uintptr_t(this), ((count()-1)*2), "", count()-1);

	return LinkableValueNode::unref();
}
#endif
