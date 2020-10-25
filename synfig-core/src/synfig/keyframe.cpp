/* === S Y N F I G ========================================================= */
/*!	\file keyframe.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include <algorithm>

#include "keyframe.h"
#include "general.h"
#include <synfig/localization.h>
#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Keyframe::Keyframe():
	time_(0),
	active_(true),
	has_waypoint_model_(false)
{
}

Keyframe::Keyframe(const Time &time):
	time_(time),
	active_(true),
	has_waypoint_model_(false)
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


void
Keyframe::apply_model(const Waypoint::Model &x)
{
    waypoint_model_.set_priority(x.get_priority());
    waypoint_model_.set_before(x.get_before());
    waypoint_model_.set_after(x.get_after());
    waypoint_model_.set_tension(x.get_tension());
    waypoint_model_.set_continuity(x.get_continuity());
    waypoint_model_.set_bias(x.get_bias());
    waypoint_model_.set_temporal_tension(x.get_temporal_tension());

    waypoint_model_.set_priority_flag(x.get_priority_flag());
    waypoint_model_.set_before_flag(x.get_before_flag());
    waypoint_model_.set_after_flag(x.get_after_flag());
    waypoint_model_.set_tension_flag(x.get_tension_flag());
    waypoint_model_.set_continuity_flag(x.get_continuity_flag());
    waypoint_model_.set_bias_flag(x.get_bias_flag());
    waypoint_model_.set_temporal_tension_flag(x.get_temporal_tension_flag());

    has_waypoint_model_ = true;
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

bool
KeyframeList::find(const UniqueID &x, KeyframeList::iterator &out)
{
	out = std::find(begin(), end(), x);
	return out != end();
	/*KeyframeList::iterator iter;
	iter=std::find(begin(),end(),x);
	if(iter==end())
		throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find UniqueID %d",x.get_uid()));
	return iter;*/
}

/*KeyframeList::const_iterator
KeyframeList::find(const UniqueID &x)const
{
	KeyframeList::const_iterator iter;
	iter=std::find(begin(),end(),x);
	if(iter==end())
		throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find UniqueID %d",x.get_uid()));
	return iter;
}*/

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
	KeyframeList::iterator iter;
	if (find(x, iter)) std::vector<Keyframe>::erase(iter);
}

bool
KeyframeList::find(const Time &x, KeyframeList::iterator &out)
{
	KeyframeList::iterator iter;
	iter=binary_find(begin(),end(),x);
	if(iter!=end() && iter->get_time().is_equal(x)) {
		out = iter;
		return true;
	}

	//throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find Keyframe %s",x.get_string().c_str()));
	return false;
}

bool
KeyframeList::find_next(const Time &x, KeyframeList::iterator &out, bool ignore_disabled)
{
	KeyframeList::iterator iter(binary_find(begin(),end(),x));

	while (iter!=end())
	{
		if 
		(
			iter->get_time().is_more_than(x)
			&& 
			( !ignore_disabled || iter->active() )
		) {
			out = iter;
			return true;
		}
		++iter;
	}

	//throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find next Keyframe %s",x.get_string().c_str()));
	return false;
}


bool 
KeyframeList::find_prev(const Time &x, KeyframeList::iterator &out, bool ignore_disabled)
{
	KeyframeList::iterator iter(binary_find(begin(),end(),x));

	if(iter!=end())
	{
		while(iter!=begin())
		{
			if
			( 
				iter->get_time().is_less_than(x)
				&&
				( !ignore_disabled || iter->active() ) 
			) {
				out = iter;
				return true;
			}
			--iter;
		};
		if
		( 
			iter->get_time().is_less_than(x)
			&&
			( !ignore_disabled || iter->active() ) 
		) {
				out = iter;
				return true;
		}
	}

	return false;
	//throw Exception::NotFound(strprintf("KeyframeList::find(): Can't find prev Keyframe %s",x.get_string().c_str()));

}



/*KeyframeList::const_iterator
KeyframeList::find(const Time &x)const
{
	return const_cast<KeyframeList*>(this)->find(x);
}


KeyframeList::const_iterator
KeyframeList::find_next(const Time &x, bool ignore_disabled)const
{
	return const_cast<KeyframeList*>(this)->find_next(x, ignore_disabled);

}


KeyframeList::const_iterator
KeyframeList::find_prev(const Time &x, bool ignore_disabled)const
{
	return const_cast<KeyframeList*>(this)->find_prev(x, ignore_disabled);

}*/

void
KeyframeList::find_prev_next(const Time& time, Time &prev, Time &next, bool ignore_disabled)
{
	KeyframeList::iterator iter;
	if (find_prev(time, iter, ignore_disabled))
		prev = iter->get_time();
	else
		prev=Time::begin();
	
	if (find_next(time, iter, ignore_disabled))
		next = iter->get_time();
	else
		next=Time::end();
}

void
KeyframeList::insert_time(const Time& location, const Time& delta)
{
//	synfig::info("KeyframeList::insert_time(): loc=%s, delta=%s",location.get_string().c_str(),delta.get_string().c_str());
	if(!delta)
		return;

	KeyframeList::iterator iter;
	if (find_next(location, iter, false)) {
		for(;iter!=end();++iter)
		{
			iter->set_time(iter->get_time()+delta);
		}
		sync();
	}
}
