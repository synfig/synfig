/* === S Y N F I G ========================================================= */
/*!	\file node.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "node.h"

#include <cstdlib>
#include <map>

#include "synfig/general.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

namespace {
	class GlobalNodeMap {
	public:
		typedef std::map<GUID, Node*> Map;
	
	private:
		std::mutex mutex;
		Map map;
	
	public:
		Node* get(const GUID &guid) {
			std::lock_guard<std::mutex> lock(mutex);
			Map::iterator i = map.find(guid);
			return i == map.end() ? nullptr : i->second;
		}

		bool add(const GUID &guid, Node *node) {
			assert(guid);
			assert(node);
			
			std::lock_guard<std::mutex> lock(mutex);
			if (map.count(guid) > 0)
				return false;
			map[guid] = node;
			return true;
		}

		void remove(const GUID &guid, Node *node) {
			assert(guid);
			assert(node);
			
			std::lock_guard<std::mutex> lock(mutex);
			Map::iterator i = map.find(guid);
			assert(i != map.end() && i->second == node);
			map.erase(i);
		}

		void move(const GUID &guid, const GUID &oldguid, Node *node) {
			assert(guid);
			assert(node);
			if (guid == oldguid) {
				assert(get(guid) == node);
				return;
			}
			assert(oldguid);
			
			std::lock_guard<std::mutex> lock(mutex);
			Map::iterator i = map.find(oldguid);
			assert(i != map.end() && i->second == node);
			map.erase(i);
			
			assert(!map.count(guid));
			map[guid] = node;
		}
	};
}

// A map to store all the GUIDs with a pointer to the Node.
static GlobalNodeMap& global_node_map()
{
	static GlobalNodeMap map;
	return map;
}

/* === P R O C E D U R E S ================================================= */

Node* synfig::find_node(const GUID &guid) {
	return global_node_map().get(guid);
}

/* === M E T H O D S ======================================================= */

void
TimePoint::absorb(const TimePoint& x)
{
	// If the Time Point to absorb is itself then bail out
	if(get_guid()==x.get_guid())
		return;
	// Creates a new GUID with the old and the new one using XOR operator
	set_guid(get_guid()^x.get_guid());
	// If the current before/after interpolation is NIL use the interpolation
	// provided by the TimePoint to absorb
	if(get_after()==INTERPOLATION_NIL)
		set_after(x.get_after());
	if(get_before()==INTERPOLATION_NIL)
		set_before(x.get_before());
	// If the interpolation of the Time Point to absorb is not the same
	// than the interpolation from the Time Point to absorb then
	// use UNDEFINED interpolation
	if(get_after()!=x.get_after() && x.get_after()!=INTERPOLATION_NIL)
		set_after(INTERPOLATION_UNDEFINED);
	if(get_before()!=x.get_before() && x.get_before()!=INTERPOLATION_NIL)
		set_before(INTERPOLATION_UNDEFINED);
}

TimePointSet::iterator
TimePointSet::insert(const TimePoint& x)
{
	// finds a iterator to a Time Point with the same time
	// \see inline bool operator==(const TimePoint& lhs,const TimePoint& rhs)
	iterator iter(find(x));
	// If iter is not a the end of the set (we found one Time Point)
	if(iter!=end())
	{
		// Absorb the time point
		const_cast<TimePoint&>(*iter).absorb(x);
		return iter;
	}
	// Else, insert it at the first of the set
	return std::set<TimePoint>::insert(x).first;
}


Node::Node():
	guid_(GUID::zero()),
	bchanged(true),
	time_last_changed_(clock()),
	deleting_(false)
{
}

Node::~Node()
{
	begin_delete();
	if(guid_)
		global_node_map().remove(guid_, this);
}

void
Node::changed()
{
	time_last_changed_= clock();
	on_changed();
}

void
Node::child_changed(const Node *x)
{
	on_child_changed(x);
}

const synfig::GUID&
Node::get_guid()const
{
	std::lock_guard<std::mutex> lock(guid_mutex_);
	if(!guid_)
	{
		bool added = false;
		do {
			guid_.make_unique();
			added = global_node_map().add(guid_, const_cast<Node*>(this));
		} while (!added);
	}
	return guid_;
}

void
Node::set_guid(const synfig::GUID& x)
{
	assert(x);
	std::lock_guard<std::mutex> lock(guid_mutex_);
	if (guid_ == x) return;
	
	if (!guid_) {
		guid_ = x;
		global_node_map().add(guid_, const_cast<Node*>(this));
	} else
	if (guid_ != x) {
		GUID old(guid_);
		guid_ = x;
		global_node_map().move(guid_, old, this);
		on_guid_changed(old);
	}
}

int
Node::get_time_last_changed()const
{
	return time_last_changed_;
}

void
Node::add_parent(Node* new_parent)
{
	std::lock_guard<std::mutex> lock(parent_set_mutex_);

	DEBUG_LOG("SYNFIG_DEBUG_NODE_PARENT_SET",
		"%s:%d adding %p (%s) as parent of %p (%s) (%zu -> ", __FILE__, __LINE__,
			   new_parent, new_parent->get_string().c_str(),
			   this, get_string().c_str(),
			   parent_set.size());

	parent_set.insert(new_parent);

	DEBUG_LOG("SYNFIG_DEBUG_NODE_PARENT_SET",
		"%zu)\n", parent_set.size());
}

void
Node::add_child(Node*x)
{
	if (!x)
		error("Adding a null child node?!");
	else
		x->add_parent(this);
}

void
Node::remove_parent(Node* parent)
{
	std::lock_guard<std::mutex> lock(parent_set_mutex_);

	if(parent_set.count(parent) == 0)
	{
		if (DEBUG_GETENV("SYNFIG_DEBUG_NODE_PARENT_SET"))
			warning("%s:%d %p (%s) isn't in parent set of %p (%s)\n", __FILE__, __LINE__,
				   parent, parent->get_string().c_str(),
				   this, get_string().c_str());

		return;
	}

	DEBUG_LOG("SYNFIG_DEBUG_NODE_PARENT_SET",
		"%s:%d removing %p (%s) from parent set of %p (%s) (%zu -> ", __FILE__, __LINE__,
			   parent, parent->get_string().c_str(),
			   this, get_string().c_str(),
			   parent_set.size());

	parent_set.erase(parent);

	DEBUG_LOG("SYNFIG_DEBUG_NODE_PARENT_SET",
		"%zu)\n", parent_set.size());
}

void
Node::remove_child(Node*x)
{
	if (!x)
		error("Internal error: Removing a null child node?!");
	else
		x->remove_parent(this);
}

std::size_t
Node::parent_count()const
{
	return parent_set.size();
}

bool
Node::is_child_of(const Node *x) const
{
	std::lock_guard<std::mutex> lock(parent_set_mutex_);
	return parent_set.count(const_cast<Node*>(x)) != 0;
}

Node*
Node::get_first_parent() const
{
	std::lock_guard<std::mutex> lock(parent_set_mutex_);
	if (parent_set.empty())
		return nullptr;
	return *parent_set.begin();
}

void Node::foreach_parent(const ConstForeachFunc &func) const
{
	std::lock_guard<std::mutex> lock(parent_set_mutex_);
	for (const Node* node : parent_set)
		if (func(node))
			return;
}

void Node::foreach_parent(const ForeachFunc &func)
{
	std::lock_guard<std::mutex> lock(parent_set_mutex_);
	for (Node* node : parent_set)
		if (func(node))
			return;
}

const Node::time_set &
Node::get_times() const
{
	if(bchanged)
	{
		times.clear();
		get_times_vfunc(times);
		bchanged = false;
	}

	//set the output set...
	return times;
}

void
Node::begin_delete()
{
	if(!deleting_)
	{
		deleting_=true; signal_deleted()();
	}
}

void
Node::on_changed()
{
	if (DEBUG_GETENV("SYNFIG_DEBUG_ON_CHANGED"))
	{
		std::lock_guard<std::mutex> lock(parent_set_mutex_);
		printf("%s:%d Node::on_changed() for %p (%s); signaling these %zd parents:\n", __FILE__, __LINE__, this, get_string().c_str(), parent_set.size());
		for (std::set<Node*>::iterator iter = parent_set.begin(); iter != parent_set.end(); ++iter) printf(" %p (%s)\n", *iter, (*iter)->get_string().c_str());
		printf("\n");
	}

	bchanged = true;
	signal_changed()();

	std::lock_guard<std::mutex> lock(parent_set_mutex_);
	std::set<Node*>::iterator iter;
	for(iter=parent_set.begin();iter!=parent_set.end();++iter)
	{
		(*iter)->child_changed(this);
	}
}

void
Node::on_child_changed(const Node *x)
{
	signal_child_changed()(x);
	changed();
}

void
Node::on_guid_changed(synfig::GUID guid)
{
	signal_guid_changed()(guid);
}
