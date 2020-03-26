/* === S Y N F I G ========================================================= */
/*!	\file valuenode_dynamiclist.cpp
**	\brief Implementation of the "Dynamic List" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**  Copyright (c) 2011 Carlos López
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

#include "valuenode_dynamiclist.h"
#include "valuenode_const.h"
#include "valuenode_composite.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/exception.h>
#include <vector>
#include <list>
#include <algorithm>
#include <synfig/canvas.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_DynamicList, RELEASE_VERSION_0_61_06, "dynamic_list", "Dynamic List")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_DynamicList::ListEntry::ListEntry():
	index(0)
{
}

ValueNode_DynamicList::ListEntry::ListEntry(const ValueNode::Handle &value_node):
	value_node(value_node),
	index(0)
{
}

ValueNode_DynamicList::ListEntry::ListEntry(const ValueNode::Handle &value_node,Time begin, Time end):
	value_node(value_node)
{
	add(begin,false);
	add(end,false);
	add((begin+end)*0.5,true);
}

ValueNode_DynamicList::ListEntry::ActivepointList::iterator
ValueNode_DynamicList::ListEntry::add(Time time, bool status, int priority)
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList::iterator iterator;

	//! \optimize
	Activepoint ap(time,status,priority);
	ap.set_parent_index(get_index());
	ap.set_parent_value_node(get_parent_value_node());
	timing_info.push_back(ap);
	iterator iter(--iterator(timing_info.end()));
	timing_info.sort();

	return iter;
}

ValueNode_DynamicList::ListEntry::ActivepointList::iterator
ValueNode_DynamicList::ListEntry::add(const Activepoint &x)
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList::iterator iterator;

	//! \optimize
	Activepoint ap(x);
	ap.set_parent_index(get_index());
	ap.set_parent_value_node(get_parent_value_node());
	timing_info.push_back(ap);
	iterator iter(--iterator(timing_info.end()));
	timing_info.sort();

	return iter;
}

void
ValueNode_DynamicList::reindex()
{
	int i(0);

	std::vector<ListEntry>::iterator iter;

	for(iter=list.begin();iter!=list.end();++iter)
	{
		assert(iter->value_node);
		if(iter->index!=i || iter->get_parent_value_node().get()!=this)
		{
			ActivepointList::iterator iter2;

			if(iter->timing_info.size()) // is this line really necessary?
			for(iter2=iter->timing_info.begin();iter2!=iter->timing_info.end();++iter2)
			{
				iter2->set_parent_index(i);
				iter2->set_parent_value_node(this);
			}
			iter->index=i;
			iter->set_parent_value_node(this);
		}
	}
}

ValueNode_DynamicList::ListEntry
ValueNode_DynamicList::create_list_entry(int index, Time time, Real origin)
{
	ValueNode_DynamicList::ListEntry ret;
	int c(link_count());
	synfig::ValueBase prev,next;

	if(c)
		index=index%c;
	else
		index=0;

	assert(index>=0);

	ret.index=index;
	ret.set_parent_value_node(this);

	if(c)
	{
		next=(*list[index].value_node)(time);

		if(index!=0)
			prev=(*list[index-1].value_node)(time);
		else
		{
			if(get_loop())
			prev=(*list[link_count()-1].value_node)(time);
			else
			{
				prev=next;
			}
		}
	}

	Type &type(get_contained_type());
	if (type == type_vector)
	{
		if(c)
		{
			Vector a(prev.get(Vector())), b(next.get(Vector()));
			ret.value_node=ValueNode_Const::create((b-a)*origin+a);
		}
		else
		{
			ret.value_node=ValueNode_Const::create(Vector());
		}
	}
	else
	if (type == type_real)
	{
		if(c)
		{
			Real a(prev.get(Real())), b(next.get(Real()));
			ret.value_node=ValueNode_Const::create((b-a)*origin+a);
		}
		else
		{
			ret.value_node=ValueNode_Const::create(Real());
		}
	}
	else
	if (type == type_color)
	{
		if(c)
		{
			Color a(prev.get(Color())), b(next.get(Color()));
			ret.value_node=ValueNode_Composite::create((b-a)*origin+a);
		}
		else
		{
			ret.value_node=ValueNode_Const::create(Color());
		}
	}
	else
	if (type == type_angle)
	{
		if(c)
		{
			Angle a(prev.get(Angle())), b(next.get(Angle()));
			ret.value_node=ValueNode_Const::create((b-a)*origin+a);
		}
		else
		{
			ret.value_node=ValueNode_Const::create(Angle());
		}
	}
	else
	if (type == type_time)
	{
		if(c)
		{
			Time a(prev.get(Time())), b(next.get(Time()));
			ret.value_node=ValueNode_Const::create((b-a)*origin+a);
		}
		else
		{
			ret.value_node=ValueNode_Const::create(Time());
		}
	}
	else
	{
		ret.value_node=ValueNode_Const::create(get_contained_type());
	}

	ret.value_node->set_parent_canvas(get_parent_canvas());
	return ret;
}

ValueNode_DynamicList::ListEntry::ActivepointList::iterator
ValueNode_DynamicList::ListEntry::find(const UniqueID& x)
{
	return std::find(timing_info.begin(),timing_info.end(),x);
}

ValueNode_DynamicList::ListEntry::ActivepointList::const_iterator
ValueNode_DynamicList::ListEntry::find(const UniqueID& x)const
{
	return std::find(timing_info.begin(),timing_info.end(),x);
}

void
ValueNode_DynamicList::ListEntry::erase(const UniqueID& x)
{
	timing_info.erase(find(x));
}

ValueNode_DynamicList::ListEntry::ActivepointList::iterator
ValueNode_DynamicList::ListEntry::find(const Time& x)
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::iterator iter;

	for(iter=timing_info.begin();iter!=timing_info.end();++iter)
		if(iter->time==x)
			return iter;

	throw Exception::NotFound("ValueNode_DynamicList::ListEntry::find():"+x.get_string());
}

ValueNode_DynamicList::ListEntry::ActivepointList::const_iterator
ValueNode_DynamicList::ListEntry::find(const Time& x)const
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::const_iterator iter;

	for(iter=timing_info.begin();iter!=timing_info.end();++iter)
		if(iter->time==x)
			return iter;

	throw Exception::NotFound("ValueNode_DynamicList::ListEntry::find()const:"+x.get_string());
}

ValueNode_DynamicList::ListEntry::ActivepointList::iterator
ValueNode_DynamicList::ListEntry::find_next(const Time& x)
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::iterator iter;

	for(iter=timing_info.begin();iter!=timing_info.end();++iter)
		if(iter->time>x)
			return iter;

	throw Exception::NotFound("ValueNode_DynamicList::ListEntry::find_next():"+x.get_string());
}

ValueNode_DynamicList::ListEntry::ActivepointList::const_iterator
ValueNode_DynamicList::ListEntry::find_next(const Time& x)const
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::const_iterator iter;

	for(iter=timing_info.begin();iter!=timing_info.end();++iter)
		if(iter->time>x)
			return iter;

	throw Exception::NotFound("ValueNode_DynamicList::ListEntry::find_next()const:"+x.get_string());
}

ValueNode_DynamicList::ListEntry::ActivepointList::iterator
ValueNode_DynamicList::ListEntry::find_prev(const Time& x)
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::iterator iter;
	iter=timing_info.end();
	do
	{
		--iter;
		if(iter->time<x)
			return iter;
	}
	while(iter!=timing_info.begin());

	throw Exception::NotFound("ValueNode_DynamicList::ListEntry::find_prev():"+x.get_string());
}

ValueNode_DynamicList::ListEntry::ActivepointList::const_iterator
ValueNode_DynamicList::ListEntry::find_prev(const Time& x)const
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::const_iterator iter;
	iter=timing_info.end();
	do
	{
		--iter;
		if(iter->time<x)
			return iter;
	}
	while(iter!=timing_info.begin());

	throw Exception::NotFound("ValueNode_DynamicList::ListEntry::find_prev()const:"+x.get_string());
}

int
ValueNode_DynamicList::ListEntry::find(const Time& begin,const Time& end,std::vector<Activepoint*>& selected)
{
	Time curr_time(begin);
	int ret(0);

	// try to grab first waypoint
	try
	{
		ActivepointList::iterator iter;
		iter=find(curr_time);
		selected.push_back(&*iter);
		ret++;
	}
	catch(...) { }

	try
	{
		ActivepointList::iterator iter;
		while(true)
		{
			iter=find_next(curr_time);
			curr_time=iter->get_time();
			if(curr_time>=end)
				break;
			selected.push_back(&*iter);
			ret++;
		}
	}
	catch(...) { }

	return ret;
}

float
ValueNode_DynamicList::ListEntry::amount_at_time(const Time &t,bool *rising)const
{
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	if(timing_info.empty())
		return 1.0f;

	try
	{
		ActivepointList::const_iterator iter;
		iter=find(t);
		return iter->state?1.0f:0.0f;
	}
	catch(...) { }

	ActivepointList::const_iterator prev_iter;
	ActivepointList::const_iterator next_iter;

	try	{ prev_iter=find_prev(t); }
	catch(...) { return find_next(t)->state?1.0f:0.0f; }

	try	{ next_iter=find_next(t); }
	catch(...) { return prev_iter->state?1.0f:0.0f; }

	if(next_iter->state==prev_iter->state)
		return next_iter->state?1.0f:0.0f;

	if(rising)*rising=next_iter->state;

	if(next_iter->state==true)
		return float((t-prev_iter->time)/(next_iter->time-prev_iter->time));

	return float((next_iter->time-t)/(next_iter->time-prev_iter->time));
}

Activepoint
ValueNode_DynamicList::ListEntry::new_activepoint_at_time(const Time& time)const
{
	Activepoint activepoint;

	activepoint.set_state(status_at_time(time));
	activepoint.set_priority(0);

	return activepoint;
}

bool
ValueNode_DynamicList::ListEntry::status_at_time(const Time &t)const
{
	//typedef synfig::ValueNode_DynamicList::ListEntry::Activepoint Activepoint;
	typedef synfig::ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

	ActivepointList::const_iterator entry_iter;
	ActivepointList::const_iterator prev_iter;
	bool state(true);

	// New "symmetric" state mechanism
	if(!timing_info.empty())
	{
		if(timing_info.size()==1)
			state=timing_info.front().state;
		else
		{
			//! \optimize Perhaps we should use a binary search...?
			// This will give us the first activepoint that is after t.
			for(entry_iter=timing_info.begin();entry_iter!=timing_info.end();++entry_iter)
			{
				if(entry_iter->time==t)
				{
					// If we hit the entry right on the nose, then we don't
					// have to do anything more
					return entry_iter->state;
				}
				if(entry_iter->time>t)
					break;

			}
			prev_iter=entry_iter;
			prev_iter--;

			// ie:
			//
			//		|-------|---t---|-------|
			//	   prev_iter^		^entry_iter

			if(entry_iter==timing_info.end())
			{
				state=prev_iter->state;
			}
			else
			if(entry_iter==timing_info.begin())
			{
				state=entry_iter->state;
			}
			else
			if(entry_iter->priority==prev_iter->priority)
			{
				state=entry_iter->state || prev_iter->state;
			}
			else
			if(entry_iter->priority>prev_iter->priority)
			{
				state=entry_iter->state;
			}
			else
			{
				state=prev_iter->state;
			}
		}
	}
	return state;
}




/*void
ValueNode_DynamicList::add(const ValueNode::Handle &value_node, int index)
{
	ListEntry list_entry(value_node);
	list_entry.timing_info.size();

	if(index<0 || index>=(int)list.size())
	{
		list.push_back(list_entry);
	}
	else
	{
		list.insert(list.begin()+index,list_entry);
	}

	add_child(value_node.get());
	reindex();
	//changed();

	if(get_parent_canvas())
		get_parent_canvas()->signal_value_node_child_added()(this,value_node);
	else if(get_root_canvas() && get_parent_canvas())
		get_root_canvas()->signal_value_node_child_added()(this,value_node);
}*/

void
ValueNode_DynamicList::add(const ListEntry &list_entry, int index)
{
	if(index<0 || index>=(int)list.size())
		list.push_back(list_entry);
	else
		list.insert(list.begin()+index,list_entry);
	add_child(list_entry.value_node.get());

	reindex();
	//changed();

	if(get_parent_canvas())
		get_parent_canvas()->signal_value_node_child_added()(this,list_entry.value_node);
	else if(get_root_canvas() && get_parent_canvas())
		get_root_canvas()->signal_value_node_child_added()(this,list_entry.value_node);
}

void
ValueNode_DynamicList::erase(const ValueNode::Handle &value_node_)
{
	ValueNode::Handle value_node(value_node_);

	assert(value_node);
	if(!value_node)
		throw String("ValueNode_DynamicList::erase(): Passed bad value node");

	std::vector<ListEntry>::iterator iter;
	for(iter=list.begin();iter!=list.end();++iter)
		if(iter->value_node==value_node)
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
	reindex();
}


ValueNode_DynamicList::ValueNode_DynamicList(Type &container_type, Canvas::LooseHandle canvas):
	LinkableValueNode(type_list),
	container_type(&container_type),
	loop_(false)
{
	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set parent canvas for dynamic_list %lx to %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(canvas.get()));
	set_parent_canvas(canvas);
}

ValueNode_DynamicList::ValueNode_DynamicList(Type &container_type, Type &type, Canvas::LooseHandle canvas):
	LinkableValueNode(type),
	container_type(&container_type),
	loop_(false)
{
	if (getenv("SYNFIG_DEBUG_SET_PARENT_CANVAS"))
		printf("%s:%d set parent canvas for dynamic_list %lx to %lx\n", __FILE__, __LINE__, uintptr_t(this), uintptr_t(canvas.get()));
	set_parent_canvas(canvas);
}


ValueNode_DynamicList::Handle
ValueNode_DynamicList::create_on_canvas(Type &type, Canvas::LooseHandle canvas)
{
	return new ValueNode_DynamicList(type, canvas);
}

ValueNode_DynamicList::~ValueNode_DynamicList()
{
	unlink_all();
}

ValueNode_DynamicList*
ValueNode_DynamicList::create(const ValueBase &value)
{
	//vector<ValueBase> value_list(value.operator vector<ValueBase>());
	vector<ValueBase> value_list(value.get_list());

	vector<ValueBase>::iterator iter;

	if(value_list.empty())
		return 0;

	ValueNode_DynamicList* value_node(new ValueNode_DynamicList(value_list.front().get_type()));

	// when creating a list of vectors, start it off being looped.
	// I think the only time this is used if for creating polygons,
	// and we want them to be looped by default
	if (value_node->get_contained_type() == type_vector)
		value_node->set_loop(true);

	for(iter=value_list.begin();iter!=value_list.end();++iter)
	{
		ValueNode::Handle item(ValueNode_Const::create(*iter));
		value_node->add(ListEntry(item));
		assert(value_node->list.back().value_node);
	}
	return value_node;
}

ValueBase
ValueNode_DynamicList::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	std::vector<ValueBase> ret_list;
	std::vector<ListEntry>::const_iterator iter;

	assert(container_type);

	for(iter=list.begin();iter!=list.end();++iter)
	{
		bool state(iter->status_at_time(t));

		if(state)
		{
			if(iter->value_node->get_type()==*container_type)
				ret_list.push_back((*iter->value_node)(t));
			else
			{
				synfig::warning(string("ValueNode_DynamicList::operator()():")+_("List type/item type mismatch, throwing away mismatch"));
			}
		}
	}

	if(list.empty())
		synfig::warning(string("ValueNode_DynamicList::operator()():")+_("No entries in list"));
	else
	if(ret_list.empty())
		synfig::warning(string("ValueNode_DynamicList::operator()():")+_("No entries in ret_list"));

	return ret_list;
}

bool
ValueNode_DynamicList::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0);

	if((unsigned)i>=list.size())
		return false;
	if(x->get_type()!=*container_type)
		return false;
	list[i].value_node=x;
	return true;
}

ValueNode::LooseHandle
ValueNode_DynamicList::get_link_vfunc(int i)const
{
	assert(i>=0);

	if((unsigned)i>=list.size())
		return 0;
	return list[i].value_node;
}

int
ValueNode_DynamicList::link_count()const
{
	return list.size();
}

String
ValueNode_DynamicList::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());

	return etl::strprintf(_("Item %03d"),i+1);
}

ValueNode::Handle
ValueNode_DynamicList::clone(Canvas::LooseHandle canvas, const GUID& deriv_guid)const
{
	{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }

	ValueNode_DynamicList* ret=dynamic_cast<ValueNode_DynamicList*>(create_new());
	ret->set_guid(get_guid()^deriv_guid);

	std::vector<ListEntry>::const_iterator iter;

	for(iter=list.begin();iter!=list.end();++iter)
	{
		if(iter->value_node->is_exported())
			ret->add(*iter);
		else
		{
			ListEntry list_entry(*iter);
			//list_entry.value_node=find_value_node(iter->value_node->get_guid()^deriv_guid).get();
			//if(!list_entry.value_node)
			list_entry.value_node=iter->value_node->clone(canvas, deriv_guid);
			ret->add(list_entry);
			//ret->list.back().value_node=iter->value_node.clone();
		}
	}
	ret->set_loop(get_loop());
	ret->set_parent_canvas(canvas);
	return ret;
}

String
ValueNode_DynamicList::link_name(int i)const
{
	return strprintf("item%04d",i);
}

int
ValueNode_DynamicList::get_link_index_from_name(const String &name)const
{
	for(int i = 0; i < link_count(); ++i)
		if (link_name(i) == name) return i;
	throw Exception::BadLinkName(name);
}



bool
ValueNode_DynamicList::check_type(Type &type)
{
	return type==type_list;
}

void
ValueNode_DynamicList::set_member_canvas(etl::loose_handle<Canvas> canvas)
{
	for (vector<ListEntry>::iterator iter = list.begin(); iter != list.end(); iter++)
		iter->value_node->set_parent_canvas(canvas);
}

Type&
ValueNode_DynamicList::get_contained_type()const
{
	return *container_type;
}

LinkableValueNode*
ValueNode_DynamicList::create_new()const
{
	return new ValueNode_DynamicList(*container_type);
}

int
ValueNode_DynamicList::find_next_valid_entry(int orig_item, Time t)const
{
	int curr_item;

	for(curr_item=orig_item+1;curr_item!=orig_item;curr_item++)
	{
		if(curr_item==(int)list.size())
		{
			curr_item=0;
			continue;
		}
		if(list[curr_item].status_at_time(t))
			return curr_item;
	}
	return curr_item;
}

int
ValueNode_DynamicList::find_prev_valid_entry(int orig_item, Time t)const
{
	int curr_item;

	for(curr_item=orig_item-1;curr_item!=orig_item;curr_item--)
	{
		if(curr_item==-1)
		{
			curr_item=list.size();
			continue;
		}
		if(list[curr_item].status_at_time(t))
			return curr_item;
	}
	return curr_item;
}

const synfig::Node::time_set	& ValueNode_DynamicList::ListEntry::get_times() const
{
	synfig::ActivepointList::const_iterator 	j = timing_info.begin(),
											end = timing_info.end();

	//must remerge with all the other values because we don't know if we've changed...
	if (!value_node) {
		static const synfig::Node::time_set empty_set {};
		synfig::error("ValueNode Dynamic List invalid");
		return empty_set;
	}
	times = value_node->get_times();

	for(; j != end; ++j)
	{
		TimePoint t;
		t.set_time(j->get_time());
		t.set_guid(j->get_guid());

		times.insert(t);
	}

	return times;
}

void ValueNode_DynamicList::get_times_vfunc(Node::time_set &set) const
{
	//add in the active points
	int size = list.size();

	//rebuild all the info...
	for(int i = 0; i < size; ++i)
	{
		const Node::time_set & tset= list[i].get_times();
		set.insert(tset.begin(),tset.end());
	}
}


//new find functions that don't throw
struct timecmp
{
	Time t;

	timecmp(const Time &c) :t(c) {}

	bool operator()(const Activepoint &rhs) const
	{
		return t.is_equal(rhs.get_time());
	}
};

ValueNode_DynamicList::ListEntry::findresult ValueNode_DynamicList::ListEntry::find_uid(const UniqueID& x)
{
	findresult f;
	f.second = false;

	f.first = std::find(timing_info.begin(),timing_info.end(),x);

	if(f.first != timing_info.end())
	{
		f.second = true;
	}

	return f;
}

ValueNode_DynamicList::ListEntry::const_findresult ValueNode_DynamicList::ListEntry::find_uid(const UniqueID& x) const
{
	const_findresult f;
	f.second = false;

	f.first = std::find(timing_info.begin(),timing_info.end(),x);

	if(f.first != timing_info.end())
	{
		f.second = true;
	}

	return f;
}

ValueNode_DynamicList::ListEntry::findresult ValueNode_DynamicList::ListEntry::find_time(const Time& x)
{
	findresult f;
	f.second = false;

	f.first = std::find_if(timing_info.begin(),timing_info.end(),timecmp(x));

	if(f.first != timing_info.end())
	{
		f.second = true;
	}

	return f;
}

ValueNode_DynamicList::ListEntry::const_findresult ValueNode_DynamicList::ListEntry::find_time(const Time& x)const
{
	const_findresult f;
	f.second = false;

	f.first = std::find_if(timing_info.begin(),timing_info.end(),timecmp(x));

	if(f.first != timing_info.end())
	{
		f.second = true;
	}

	return f;
}

void
ValueNode_DynamicList::insert_time(const Time& location, const Time& delta)
{
	if(!delta)
		return;

	std::vector<ListEntry>::iterator iter(list.begin());
	for(;iter!=list.end();++iter)
	{
		try
		{
			ListEntry& item(*iter);

			ActivepointList::iterator iter(item.find_next(location));
			for(;iter!=item.timing_info.end();++iter)
			{
				iter->set_time(iter->get_time()+delta);
			}
		}
		catch(Exception::NotFound&) { }
	}
	changed();
}

LinkableValueNode::Vocab
ValueNode_DynamicList::get_children_vocab_vfunc()const
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
