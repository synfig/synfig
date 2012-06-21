/* === S Y N F I G ========================================================= */
/*!	\file keyframe.cpp
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <algorithm>

#include <ETL/stringf>

#include "keyframe.h"
#include "exception.h"
#include "general.h"
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Keyframe::Keyframe():
	time_(0)
{
}

Keyframe::Keyframe(const Time &time):
	time_(time)
{
}

void
Keyframe::set_active(bool x)
{
	if(active_!=x)
	{
		active_=x;
	}
}

Keyframe::~Keyframe()
{
}

void
KeyframeList::dump()const
{
	const_iterator iter;
	int i;
	synfig::info(">>>>>>>>BEGIN KEYFRAME DUMP");
	for(iter=begin(),i=0;iter!=end();++iter,i++)
	{
		synfig::info("#%d, time: %s, desc: %s",i,iter->get_time().get_string().c_str(),iter->get_description().c_str());
	}
	synfig::info("<<<<<<<<END KEYFRAME DUMP");
}

void
KeyframeList::sync()
{
	//synfig::info("PRE-SORT:");
	//dump();
	sort(begin(),end());
	//synfig::info("POST-SORT:");
	//dump();
}

KeyframeList::iterator
KeyframeList::find(const UniqueID &x)
{
	KeyframeList::iterator iter;
	iter=std::find(begin(),end(),x);
	if(iter==end())
		throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find UniqueID %d",x.get_uid()));
	return iter;
}

KeyframeList::const_iterator
KeyframeList::find(const UniqueID &x)const
{
	KeyframeList::const_iterator iter;
	iter=std::find(begin(),end(),x);
	if(iter==end())
		throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find UniqueID %d",x.get_uid()));
	return iter;
}

KeyframeList::iterator
KeyframeList::add(const Keyframe &x)
{
	push_back(x);
	iterator ret(end());
	ret--;
	assert(x==*ret);
	sync();
	return ret;
}

void
KeyframeList::erase(const UniqueID &x)
{
	std::vector<Keyframe>::erase(find(x));
}

KeyframeList::iterator
KeyframeList::find(const Time &x)
{
	KeyframeList::iterator iter;
	iter=binary_find(begin(),end(),x);
	if(iter!=end() && iter->get_time().is_equal(x))
		return iter;
/*	iter++;
	if(iter!=end() && iter->get_time().is_equal(x))
		return iter;
*/
	throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find Keyframe %s",x.get_string().c_str()));
}

KeyframeList::iterator
KeyframeList::find_next(const Time &x)
{
	KeyframeList::iterator iter(binary_find(begin(),end(),x));

	if(iter!=end())
	{
		if(iter->get_time().is_more_than(x))
			return iter;
		++iter;
		if(iter!=end())
		{
			if(iter->get_time().is_more_than(x))
				return iter;
/*			++iter;
			if(iter!=end() && iter->get_time().is_more_than(x))
				return iter;
*/
		}
	}

	throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find next Keyframe %s",x.get_string().c_str()));
}


KeyframeList::iterator
KeyframeList::find_prev(const Time &x)
{
	KeyframeList::iterator iter(binary_find(begin(),end(),x));

	if(iter!=end())
	{
		if(iter->get_time()+Time::epsilon()<x)
			return iter;
		if(iter!=begin() && (--iter)->get_time()+Time::epsilon()<x)
			return iter;
	}
	throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find prev Keyframe %s",x.get_string().c_str()));

}



KeyframeList::const_iterator
KeyframeList::find(const Time &x)const
{
	return const_cast<KeyframeList*>(this)->find(x);
}


KeyframeList::const_iterator
KeyframeList::find_next(const Time &x)const
{
	return const_cast<KeyframeList*>(this)->find_next(x);

}


KeyframeList::const_iterator
KeyframeList::find_prev(const Time &x)const
{
	return const_cast<KeyframeList*>(this)->find_prev(x);

}

void
KeyframeList::find_prev_next(const Time& time, Time &prev, Time &next)const
{
	try { prev=find_prev(time)->get_time(); }
	catch(...) { prev=Time::begin(); }
	try { next=find_next(time)->get_time(); }
	catch(...) { next=Time::end(); }
}

void
KeyframeList::insert_time(const Time& location, const Time& delta)
{
//	synfig::info("KeyframeList::insert_time(): loc=%s, delta=%s",location.get_string().c_str(),delta.get_string().c_str());
	if(!delta)
		return;
	try
	{
		iterator iter(find_next(location));
		for(;iter!=end();++iter)
		{
			iter->set_time(iter->get_time()+delta);
		}
		sync();
	}
	catch(Exception::NotFound) { }
}
