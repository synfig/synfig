/* === S Y N F I G ========================================================= */
/*!	\file node.cpp
**	\brief Template File
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cstdlib>
#include <cstdio>
#include "node.h"
// #include "nodebase.h"		// this defines a bunch of sigc::slots that are never used

#ifdef HASH_MAP_H
#include HASH_MAP_H
#else
#include <map>
#endif

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

// About BE_FRUGAL_WITH_GUIDS
// If this macro is set, then a GUID will NOT
// be calculated until the first call to get_guid()
// This also means that the node doesn't get
// added to the database until get_guid() is called
// for the first time, or set_guid() is called.
// If it is expensive to calculate GUIDs, then
// this can improve performance a tad in
// some cases. Otherwise, it doesn't change
// much of anything.
#define BE_FRUGAL_WITH_GUIDS 1

#ifndef __sys_clock
	#ifndef _WIN32
		# include <time.h>
		# define __sys_clock	::clock
	#else
		# ifdef __GNUG__
			#  include <time.h>
			#  define __sys_clock	::clock
		# else
			typedef int clock_t;
			extern clock_t _clock();
			#  define CLOCKS_PER_SEC 1000
			#  define __sys_clock	_clock
		# endif // __GNUG__
	#endif // _WIN_32
#endif // __sys_clock

/* === G L O B A L S ======================================================= */

#ifdef HASH_MAP_H
typedef HASH_MAP_CLASS<synfig::GUID,Node*,GUIDHash> GlobalNodeMap;
#else
typedef map<synfig::GUID,Node*> GlobalNodeMap;
#endif

//! A map to store all the GUIDs with a pointer to the Node.
static GlobalNodeMap* global_node_map_;

static GlobalNodeMap& global_node_map()
{
	if(!global_node_map_)
		global_node_map_=new GlobalNodeMap;
	return *global_node_map_;
}

/* === P R O C E D U R E S ================================================= */

synfig::Node*
synfig::find_node(const synfig::GUID& guid)
{
	if(global_node_map().count(guid)==0)
		return 0;
	return global_node_map()[guid];
}

static void
refresh_node(synfig::Node* node, synfig::GUID old_guid)
{
	assert(global_node_map().count(old_guid));
	global_node_map().erase(old_guid);
	assert(!global_node_map().count(old_guid));
	global_node_map()[node->get_guid()]=node;
}

/* === M E T H O D S ======================================================= */

#ifdef _DEBUG
const char *
TimePoint::c_str()const
{
	return get_time().get_string().c_str();
}
#endif

void
TimePoint::absorb(const TimePoint& x)
{
	//! If the Time Point to absorb is itself then bail out
	if(get_guid()==x.get_guid())
		return;
	//! Creates a new GUID with the old and the new one using XOR operator
	set_guid(get_guid()^x.get_guid());
	//! If the current before/after interpolation is NIL use the interpolation
	//! provided by the TimePoint to absorb
	if(get_after()==INTERPOLATION_NIL)
		set_after(x.get_after());
	if(get_before()==INTERPOLATION_NIL)
		set_before(x.get_before());
	//! If the interpolation of the Time Point to absorb is not the same
	//! than the interpolation from the Time Point to absorb then
	//! use UNDEFINED interpolation
	if(get_after()!=x.get_after() && x.get_after()!=INTERPOLATION_NIL)
		set_after(INTERPOLATION_UNDEFINED);
	if(get_before()!=x.get_before() && x.get_before()!=INTERPOLATION_NIL)
		set_before(INTERPOLATION_UNDEFINED);
}

TimePointSet::iterator
TimePointSet::insert(const TimePoint& x)
{
	//! finds a iterator to a Time Point with the same time
	//! \see inline bool operator==(const TimePoint& lhs,const TimePoint& rhs)
	iterator iter(find(x));
	//! If iter is not a the end of the set (we found one Time Point)
	if(iter!=end())
	{
		//! Absorb the time point
		const_cast<TimePoint&>(*iter).absorb(x);
		return iter;
	}
	//! Else, insert it at the first of the set
	return std::set<TimePoint>::insert(x).first;
}


Node::Node():
	guid_(0),
	bchanged(true),
	time_last_changed_(__sys_clock()),
	deleting_(false)
{
#ifndef BE_FRUGAL_WITH_GUIDS
	guid_.make_unique();
	assert(guid_);
	assert(!global_node_map().count(guid_));
	global_node_map()[guid_]=this;
#endif
}

Node::~Node()
{
	begin_delete();

	if(guid_)
	{
		assert(global_node_map().count(guid_));
		global_node_map().erase(guid_);
		assert(!global_node_map().count(guid_));
	}
}

void
Node::changed()
{
	time_last_changed_=__sys_clock();
	on_changed();
}


//! Gets the GUID for this value node
const synfig::GUID&
Node::get_guid()const
{
#ifdef BE_FRUGAL_WITH_GUIDS
	if(!guid_)
	{
		const_cast<synfig::GUID&>(guid_).make_unique();
		assert(guid_);
		assert(!global_node_map().count(guid_));
		global_node_map()[guid_]=const_cast<Node*>(this);
	}
#endif

	return guid_;
}

//! Sets the GUID for this value node
void
Node::set_guid(const synfig::GUID& x)
{
	assert(x);

#ifdef BE_FRUGAL_WITH_GUIDS
	if(!guid_)
	{
		guid_=x;
		assert(!global_node_map().count(guid_));
		global_node_map()[guid_]=this;
	}
	else
#endif
	if(guid_!=x)
	{
		synfig::GUID oldguid(guid_);
		guid_=x;
		refresh_node(this, oldguid);
		on_guid_changed(oldguid);
	}
}

int
Node::get_time_last_changed()const
{
	return time_last_changed_;
}

void
Node::add_child(Node*x)
{
	if (getenv("SYNFIG_DEBUG_NODE_PARENT_SET"))
		printf("%s:%d adding %lx (%s) as parent of %lx (%s) (%zd -> ", __FILE__, __LINE__,
			   uintptr_t(this), get_string().c_str(),
			   uintptr_t(x), x->get_string().c_str(),
			   x->parent_set.size());

	x->parent_set.insert(this);

	if (getenv("SYNFIG_DEBUG_NODE_PARENT_SET"))
		printf("%zd)\n", x->parent_set.size());
}

void
Node::remove_child(Node*x)
{
	if(x->parent_set.count(this) == 0)
	{
		if (getenv("SYNFIG_DEBUG_NODE_PARENT_SET"))
			printf("%s:%d %lx (%s) isn't in parent set of %lx (%s)\n", __FILE__, __LINE__,
				   uintptr_t(this), get_string().c_str(),
				   uintptr_t(x), x->get_string().c_str());

		return;
	}

	if (getenv("SYNFIG_DEBUG_NODE_PARENT_SET"))
		printf("%s:%d removing %lx (%s) from parent set of %lx (%s) (%zd -> ", __FILE__, __LINE__,
			   uintptr_t(this), get_string().c_str(),
			   uintptr_t(x), x->get_string().c_str(),
			   x->parent_set.size());

	x->parent_set.erase(this);

	if (getenv("SYNFIG_DEBUG_NODE_PARENT_SET"))
		printf("%zd)\n", x->parent_set.size());
}

int
Node::parent_count()const
{
	return parent_set.size();
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
	if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
	{
		printf("%s:%d Node::on_changed() for %lx (%s); signalling these %zd parents:\n", __FILE__, __LINE__, uintptr_t(this), get_string().c_str(), parent_set.size());
		for (set<Node*>::iterator iter = parent_set.begin(); iter != parent_set.end(); iter++) printf(" %lx (%s)\n", uintptr_t(*iter), (*iter)->get_string().c_str());
		printf("\n");
	}

	bchanged = true;
	signal_changed()();

	std::set<Node*>::iterator iter;
	for(iter=parent_set.begin();iter!=parent_set.end();++iter)
	{
		(*iter)->changed();
	}
}

void
Node::on_guid_changed(synfig::GUID guid)
{
	signal_guid_changed()(guid);
}
