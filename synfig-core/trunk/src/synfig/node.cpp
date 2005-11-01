/* === S Y N F I G ========================================================= */
/*!	\file node.cpp
**	\brief Template File
**
**	$Id: node.cpp,v 1.5 2005/01/07 03:29:12 darco Exp $
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

/* === H E A D E R S ======================================================= */

#define HASH_MAP_H <ext/hash_map>

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "node.h"
#include "proto/nodebase.h"

#ifdef HASH_MAP_H
#include HASH_MAP_H
using namespace __gnu_cxx;
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
# endif
#endif
#endif

/* === G L O B A L S ======================================================= */

#ifdef HASH_MAP_H
typedef hash_map<GUID,Node*,GUIDHash> GlobalNodeMap;
#else
typedef map<GUID,Node*> GlobalNodeMap;
#endif

static GlobalNodeMap* global_node_map_;

static GlobalNodeMap& global_node_map()
{
	if(!global_node_map_)
		global_node_map_=new GlobalNodeMap;
	return *global_node_map_;
}

/* === P R O C E D U R E S ================================================= */

synfig::Node*
synfig::find_node(const GUID& guid)
{
	if(global_node_map().count(guid)==0)
		return 0;
	return global_node_map()[guid];
}

static void
refresh_node(synfig::Node* node, GUID old_guid)
{
	assert(global_node_map().count(old_guid));
	global_node_map().erase(old_guid);
	assert(!global_node_map().count(old_guid));
	global_node_map()[node->get_guid()]=node;
}

/* === M E T H O D S ======================================================= */

void
TimePoint::absorb(const TimePoint& x)
{
	if(get_guid()==x.get_guid())
		return;
	set_guid(get_guid()^x.get_guid());
	
	if(get_after()==INTERPOLATION_NIL)
		set_after(x.get_after());
	if(get_before()==INTERPOLATION_NIL)
		set_before(x.get_before());
	
	if(get_after()!=x.get_after() && x.get_after()!=INTERPOLATION_NIL)
		set_after(INTERPOLATION_UNDEFINED);
	if(get_before()!=x.get_before() && x.get_before()!=INTERPOLATION_NIL)
		set_before(INTERPOLATION_UNDEFINED);	
}

TimePointSet::iterator
TimePointSet::insert(const TimePoint& x)
{
	iterator iter(find(x));
	if(iter!=end())
	{
		const_cast<TimePoint&>(*iter).absorb(x);
		return iter;
	}
	return std::set<TimePoint>::insert(x).first;
}















Node::Node():
	guid_(0),
	bchanged(true),
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
const GUID&
Node::get_guid()const
{
#ifdef BE_FRUGAL_WITH_GUIDS
	if(!guid_)
	{
		const_cast<GUID&>(guid_).make_unique();
		assert(guid_);
		assert(!global_node_map().count(guid_));
		global_node_map()[guid_]=const_cast<Node*>(this);	
	}
#endif
	
	return guid_;
}

//! Sets the GUID for this value node
void
Node::set_guid(const GUID& x)
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
		GUID oldguid(guid_);
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
	x->parent_set.insert(this);
}

void
Node::remove_child(Node*x)
{
	if(x->parent_set.count(this)) x->parent_set.erase(this);
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
	bchanged = true;
	signal_changed()();

	std::set<Node*>::iterator iter;
	for(iter=parent_set.begin();iter!=parent_set.end();++iter)
	{
		(*iter)->changed();
	}
}

void
Node::on_guid_changed(GUID guid)
{
	signal_guid_changed()(guid);
}
