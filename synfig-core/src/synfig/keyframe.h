/* === S Y N F I G ========================================================= */
/*!	\file keyframe.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_KEYFRAME_H
#define __SYNFIG_KEYFRAME_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <vector>
#include "string.h"
#include "time.h"
#include "uniqueid.h"
#include "guid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! \writeme
class Keyframe :  public UniqueID
{
private:

	Time time_;
	String desc_;
	GUID guid_;

	/*! \c true if the keyframe is active, \c false if it is to be skipped (marker)
	**	\see set_active(), enable(), disable, active()
	*/
	bool active_;

public:

	Keyframe();

	Keyframe(const Time &time);

	~Keyframe();

	void set_time(Time x) { time_=x; }

	Time get_time()const { return time_; }

	void set_description(String x) { desc_=x; }

	String get_description()const { return desc_; }

	const GUID& get_guid()const { return guid_; }
	void set_guid(const GUID& x) { guid_=x; }

	//! Enables the keframe (Making it \em active)
	void enable() { set_active(true); }

	//! Disables the keyframe  (Making it \em inactive)
	/*! When keyframe is disabled, it will be acting as time marker. */
	void disable() { set_active(false); }
	
	//! Sets the 'active' flag for the LaKeyframe to the state described by \a x
	/*! When keyframe is disabled, it will be acting as time marker. */
	void set_active(bool x);

	//! Returns the status of the 'active' flag
	bool active()const { return active_; }

	using UniqueID::operator<;
	using UniqueID::operator==;
	using UniqueID::operator!=;
	using UniqueID::operator=;

	bool operator<(const Keyframe &rhs)const { return time_<rhs.time_; }
	bool operator<(const Time &rhs)const { return time_<rhs; }

//	bool operator==(const Keyframe &rhs)const { return id_==rhs.id_; }
	bool operator==(const Time &rhs)const { return time_==rhs; }

//	bool operator!=(const Keyframe &rhs)const { return id_!=rhs.id_; }
	bool operator!=(const Time &rhs)const { return time_!=rhs; }
}; // END of class Keyframe

class KeyframeList : public std::vector<Keyframe>
{

public:

	iterator add(const Keyframe &x);

	void erase(const UniqueID &x);

	iterator find(const UniqueID &x);

	const_iterator find(const UniqueID &x)const;

	//! Finds the keyframe at an exact point in time
	iterator find(const Time &x);

	//! Finds the keyframe after that point in time
	iterator find_next(const Time &x, bool ignore_disabled = true);

	//! Finds the keyframe before that point in time
	iterator find_prev(const Time &x, bool ignore_disabled = true);

	const_iterator find(const Time &x)const;
	const_iterator find_next(const Time &x, bool ignore_disabled = true)const;
	const_iterator find_prev(const Time &x, bool ignore_disabled = true)const;

	void find_prev_next(const Time& time, Time &prev, Time &next, bool ignore_disabled = true)const;

	void insert_time(const Time& location, const Time& delta);

	void dump()const;
	void sync();
};

//typedef std::list<Keyframe> KeyframeList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
