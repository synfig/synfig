/* === S Y N F I G ========================================================= */
/*!	\file distance.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_DISTANCE_H
#define __SYNFIG_DISTANCE_H

/* === H E A D E R S ======================================================= */

#include "real.h"
#include "string.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class RendDesc;

class Distance
{
public:
	typedef Real value_type;

	enum System
	{
		SYSTEM_UNITS,		//!<
		SYSTEM_PIXELS,		//!<

		SYSTEM_POINTS,		//!<
		SYSTEM_INCHES,		//!<
		SYSTEM_METERS,		//!<
		SYSTEM_MILLIMETERS,	//!<
		SYSTEM_CENTIMETERS,	//!<

		SYSTEM_END			//!< \internal
	};

	class BadSystem { };

private:
	value_type value_;

	System system_;


public:

	Distance(): value_(), system_() { }
	Distance(const value_type& value, System system):value_(value),system_(system) { }
	explicit Distance(const synfig::String& string);

	operator Real()const { return value_; }

	Distance& operator=(const Real& rhs) { value_=rhs; return *this; }

	Distance& operator=(const synfig::String& rhs);

	synfig::String get_string(int digits=4)const;

	const System& get_system()const { return system_; }

	const Real& get()const { return value_; }

	Real get(System system, const RendDesc& rend_desc)const;

	void convert(System system, const RendDesc& rend_desc);

	Real meters()const;
	Real meters(const RendDesc& rend_desc)const;
	Real units(const RendDesc& rend_desc)const;

	static Real meters_to_system(Real x, System target_system);
	static System ident_system(const synfig::String& str);
	static synfig::String system_name(System system);
	static synfig::String system_local_name(System system);

	const Distance& operator+=(const Distance &rhs) { value_+=meters_to_system(rhs.meters(),system_); return *this; }
	const Distance& operator-=(const Distance &rhs) { value_-=meters_to_system(rhs.meters(),system_); return *this; }

	const Distance& operator+=(const float &rhs) { value_+=rhs; return *this; }
	const Distance& operator-=(const float &rhs) { value_-=rhs; return *this; }
	const Distance& operator*=(const float &rhs) { value_*=rhs; return *this; }
	const Distance& operator/=(const float &rhs) { value_/=rhs; return *this; }

/*
	template<typename U> const Time& operator+=(const U &rhs) { value_+=rhs; return *this; }
	template<typename U> const Time& operator-=(const U &rhs) { value_-=rhs; return *this; }
	template<typename U> const Time& operator*=(const U &rhs) { value_*=rhs; return *this; }
	template<typename U> const Time& operator/=(const U &rhs) { value_/=rhs; return *this; }

	template<typename U> Time operator+(const U &rhs)const { return value_+rhs; }
	template<typename U> Time operator-(const U &rhs)const { return value_-rhs; }
	template<typename U> Time operator*(const U &rhs)const { return value_*rhs; }
	template<typename U> Time operator/(const U &rhs)const { return value_/rhs; }

	Time operator-()const { return -value_; }
*/
}; // END of class Distance

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
