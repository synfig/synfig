/* === S Y N F I G ========================================================= */
/*!	\file valuenode_dynamiclist.h
**	\brief Header file for implementation of the "Dynamic List" valuenode conversion.
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2011 Carlos López
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

#ifndef __SYNFIG_VALUENODE_DYNAMICLIST_H
#define __SYNFIG_VALUENODE_DYNAMICLIST_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include <list>

#include <synfig/valuenode.h>
#include <synfig/time.h>
#include <synfig/uniqueid.h>
#include <synfig/activepoint.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
class ValueNode_BLine;
class Canvas;

/*! \class ValueNode_DynamicList
**	\brief Animated List ValueNode
**
**	This ValueNode was originally set up to have a list
**	of ValueNodes and their associated "on" and "off" points.
**	ie: Any time that was directly after an "on" point,
**	the item would be "on", and any time that was directly
**	after an "off" point would be "off". This is pretty intuitive.
**	However, it does have its problems.
**
**	The problems arise when we introduce the concept of a
**	Keyframe. Keyframes can be manipulated via the Synfig
**	Application Library. They allow the user to quickly
**	and "automagically" rearrange an animation by moving
**	the associated keyframes. With the old way that the
**	"on" and "off" points were handled, this task became
**	overly complicated.
**
**	What is required is a "symmetric" system of describing
**	"on" and "off" points. Instead of the point representing
**	the state of the item after that point in time, we have
**	the point represent <i>only that frame</i>. The state
**	of the item is calculated by looking at the points
**	around it: If either (or both) points are "on", then the
**	current state is "on". Otherwise, the point is "off"
**
**	This may be a bit confusing at first, but it is required
**	if we want the keyframe mechanism to "just work".
*/
class ValueNode_DynamicList : public LinkableValueNode
{
public:

	/*! \class ListEntry
	**	\brief Contains a potential list item, and associated timing information
	**
	**	This structure contains a RHandle to a ValueNode,
	**	as well as the associated on/off timing information
	**	which determines when this item is included in the list.
	**
	**	The timing information is stored in the member <tt>timing_info</tt>.
	*/
	struct ListEntry : public UniqueID
	{
		friend class ValueNode_DynamicList;
		friend class ValueNode_BLine;
		friend class ValueNode_WPList;
		friend class ValueNode_DIList;
	public:
		typedef synfig::Activepoint Activepoint;

		typedef std::list<Activepoint> ActivepointList;

		typedef std::pair<ActivepointList::iterator,bool>		findresult;
		typedef std::pair<ActivepointList::const_iterator,bool>	const_findresult;


	private:
		mutable Node::time_set	times;
	public:
		ValueNode::RHandle value_node;

		ActivepointList timing_info;

	private:
		int index;
		etl::loose_handle<ValueNode> parent_;
		void set_parent_value_node(const etl::loose_handle<ValueNode> &x) { parent_=x; }

	public:

		int get_index()const { return index; }


		bool status_at_time(const Time &x)const;

		float amount_at_time(const Time &x, bool *rising=0)const;

		ActivepointList::iterator add(Time time, bool status, int priority=0);
		ActivepointList::iterator add(const Activepoint &x);

		findresult find_uid(const UniqueID& x);
		const_findresult find_uid(const UniqueID& x)const;

		findresult find_time(const Time& x);
		const_findresult find_time(const Time& x)const;

		ActivepointList::iterator find(const UniqueID& x);
		ActivepointList::const_iterator find(const UniqueID& x)const;
		ActivepointList::iterator find(const Time& x);
		ActivepointList::const_iterator find(const Time& x)const;
		ActivepointList::iterator find_prev(const Time& x);
		ActivepointList::const_iterator find_prev(const Time& x)const;
		ActivepointList::iterator find_next(const Time& x);
		ActivepointList::const_iterator find_next(const Time& x)const;

		Activepoint new_activepoint_at_time(const Time& x)const;

		ActivepointList::iterator add(Time time)
			{ return add(time, status_at_time(time)); }

		void erase(const UniqueID& x);

		int find(const Time& begin,const Time& end,std::vector<Activepoint*>& list);

		const synfig::Node::time_set	&get_times() const;

		const etl::loose_handle<ValueNode> &get_parent_value_node()const { return parent_; }

		ListEntry();
		ListEntry(const ValueNode::Handle &value_node);
		ListEntry(const ValueNode::Handle &value_node,Time begin, Time end);
	}; // END of struct ValueNode_DynamicList::ListEntry

	typedef etl::handle<ValueNode_DynamicList> Handle;
	typedef etl::handle<const ValueNode_DynamicList> ConstHandle;

protected:
	ValueNode_DynamicList(Type &container_type=type_nil, etl::loose_handle<Canvas> canvas = 0);
	ValueNode_DynamicList(Type &container_type, Type &type, etl::loose_handle<Canvas> canvas = 0);

	Type *container_type;

	bool loop_;


public:
	std::vector<ListEntry> list;

public:

	void add(const ValueNode::Handle &value_node, int index=-1) { add(ListEntry(value_node), index); }
	void add(const ListEntry &value_node, int index=-1);
	void erase(const ValueNode::Handle &value_node);
	void reindex();

	int find_next_valid_entry(int x, Time t)const;
	int find_prev_valid_entry(int x, Time t)const;

	virtual ValueNode::LooseHandle get_link_vfunc(int i)const;

	virtual int link_count()const;

	virtual String link_name(int i)const;

 	virtual ValueBase operator()(Time t)const;

	virtual ~ValueNode_DynamicList();

	virtual String link_local_name(int i)const;
	virtual int get_link_index_from_name(const String &name)const;

	virtual String get_name()const;
	virtual String get_local_name()const;

	bool get_loop()const { return loop_; }
	void set_loop(bool x) { loop_=x; }

	void set_member_canvas(etl::loose_handle<Canvas>);

	Type& get_contained_type()const;


	// TODO: better type-checking
	template <typename iterator> static Handle
	create_from_list(iterator begin, iterator end)
	{
		Handle ret = create_on_canvas((*begin)->get_type());
		for(;begin!=end;++begin)
			ret->add(ListEntry(*begin));
		return ret;
	}

	void insert_time(const Time& location, const Time& delta);
	//void manipulate_time(const Time& old_begin,const Time& old_end,const Time& new_begin,const Time& new_end);

	virtual ValueNode::Handle clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid=GUID())const;

	virtual ListEntry create_list_entry(int index, Time time=0, Real origin=0.5);

protected:

	virtual bool set_link_vfunc(int i,ValueNode::Handle x);
	LinkableValueNode* create_new()const;

	virtual void get_times_vfunc(Node::time_set &set) const;

public:
	/*! \note The construction parameter (\a id) is the type that the list
	**	contains, rather than the type that it will yield
	**	(which is ValueBase::TYPE_LIST)
	*/
	static Handle create_on_canvas(Type &type=type_nil, etl::loose_handle<Canvas> canvas = 0);
	using synfig::LinkableValueNode::get_link_vfunc;
	using synfig::LinkableValueNode::set_link_vfunc;
	static bool check_type(Type &type);
	static ValueNode_DynamicList* create(const ValueBase &x=type_gradient);
	virtual Vocab get_children_vocab_vfunc()const;
}; // END of class ValueNode_DynamicList

typedef ValueNode_DynamicList::ListEntry::Activepoint Activepoint;
typedef ValueNode_DynamicList::ListEntry::ActivepointList ActivepointList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
