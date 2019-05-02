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
#include "waypoint.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \class Keyframe
**  \brief Keyframe is used to record the state of the animation at that point (time_)
*
* A Keyframe can be described, activated or disabled and have an associated Waypoint::Model.
* Common comparison operators can be used for Keyframes operation ==, <, != .
* \see Keyframe::set_description(String x), Keyframe::get_description(), Keyframe::enable(), Keyframe::disable ()
*/
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

	Waypoint::Model waypoint_model_;
    /*! \c true a waypoint model has been affected, \c false when created
    **  \see apply_model(const Waypoint::Model &x)
    */
	bool has_waypoint_model_;

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

	const Waypoint::Model &get_waypoint_model()const { return waypoint_model_; }
	//! Keep a trace of the associated waypoint Model.
	void apply_model(const Waypoint::Model &x);
    //! Returns the status of the 'waypoint model' flag
	bool has_model() const {return has_waypoint_model_; }
}; // END of class Keyframe

class KeyframeList : public std::vector<Keyframe>
{

public:

	iterator add(const Keyframe &x);

	void erase(const UniqueID &x);

	bool find(const UniqueID &x, KeyframeList::iterator &out);

	//const_iterator find(const UniqueID &x)const;

	//! Finds the keyframe at an exact point in time
	bool find(const Time &x, KeyframeList::iterator &out);

	//! Finds the keyframe after that point in time
	bool find_next(const Time &x, KeyframeList::iterator &out, bool ignore_disabled = true);

	//! Finds the keyframe before that point in time
	bool find_prev(const Time &x, KeyframeList::iterator &out, bool ignore_disabled = true);

/*	const_iterator find(const Time &x)const;
	const_iterator find_next(const Time &x, bool ignore_disabled = true)const;
	const_iterator find_prev(const Time &x, bool ignore_disabled = true)const;*/

	void find_prev_next(const Time& time, Time &prev, Time &next, bool ignore_disabled = true);

	void insert_time(const Time& location, const Time& delta);

	void dump()const;
	void sync();
};

//typedef std::list<Keyframe> KeyframeList;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
