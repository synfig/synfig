/* === S Y N F I G ========================================================= */
/*!	\file time.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_TIME_H
#define __SYNFIG_TIME_H

/* === H E A D E R S ======================================================= */

#include <cmath>

#include "string.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class Time
**	\todo writeme
**	\see TimeFormat, time_to_string(), string_to_time()
*/
class Time
{
public:
	typedef double value_type;
	typedef long long ticks_type;

	/*!	\enum Format
	**	\todo writeme
	**	\see Time, get_string() */
	enum Format
	{
		FORMAT_NORMAL=0,		//!< Represents the default method of printing the time
		FORMAT_NOSPACES=(1<<0),	//!< Remove any whitespace
		FORMAT_FULL=(1<<1),		//!< Do not remove units that have "zero" value
		FORMAT_VIDEO=(1<<2),	//!< Use the HH:MM:SS.FF format
		FORMAT_FRAMES=(1<<3),	//!< Use the FF format (frames only)

		FORMAT_END=(1<<4)		//!< \internal Not used
	}; // END of enum Format

private:
	value_type value_;

	static value_type epsilon_() { return static_cast<value_type>(0.0005); }

public:
	Time(): value_() { }

	Time(const value_type &x):value_(x) { }

	Time(int x):value_(x) { }

	Time(int hour, int minute, float second):value_(static_cast<value_type>(hour*3600 + minute*60 + second)) { }

	//! Constructs Time from the given string.
	/*!	\note If the string references frames, then the
	**	frame rate (\a fps) should be provided from the
	**	correct source. (Which is most likely the RendDesc
	**	of the current Canvas)
	**	The frame count will be ignored if the
	**	FPS is not given. */
	explicit Time(const String &string, float fps=0);

	//! Marks the exclusive negative boundary of time
	static const Time begin() { return static_cast<synfig::Time>(-32767.0f*512.0f); }

	//! Marks the exclusive positive boundary of time
	static const Time end() { return static_cast<synfig::Time>(32767.0f*512.0f); }

	//! Marks zero time
	static const Time zero() { return static_cast<synfig::Time>(0); }

	//! The amount of allowable error in calculations
	static Time epsilon() { return static_cast<synfig::Time>(epsilon_()); }

	//! The duration of discrete tick used for values comparison
	static value_type tick() { return static_cast<value_type>(0.1*epsilon_()); }

	//! Returns a string describing the current time value
	/*!	\see Format */
	String get_string(float fps=0, Time::Format format=FORMAT_NORMAL) const;
	std::string get_string(Time::Format format) const;

#ifdef _DEBUG
	const char *c_str()const;
#endif

	//! \writeme
	bool is_valid()const;

	//! Rounds time to the nearest frame for the given frame rate, \a fps
	Time round(float fps)const;

	//! The discrete representation to use in std::map and std::set
	ticks_type ticks() const
		{ return (long long)::round(value_/(tick())); }

	bool is_equal(const Time& rhs) const { return ticks() == rhs.ticks(); }
	bool is_less_than(const Time& rhs) const { return ticks() < rhs.ticks(); }
	bool is_more_than(const Time& rhs) const { return rhs.is_less_than(*this); }

	operator value_type()const { return value_; }

	template<typename U> bool operator<(const U& rhs)const { return value_<rhs; }
	template<typename U> bool operator>(const U& rhs)const { return value_>rhs; }
	template<typename U> bool operator<=(const U& rhs)const { return value_<=rhs; }
	template<typename U> bool operator>=(const U& rhs)const { return value_>=rhs; }
	template<typename U> bool operator==(const U& rhs)const { return value_==rhs; }
	template<typename U> bool operator!=(const U& rhs)const { return value_!=rhs; }

	bool operator<(const Time& rhs)const { return is_less_than(rhs); }
	bool operator>(const Time& rhs)const { return is_more_than(rhs); }
	bool operator<=(const Time& rhs)const { return !is_more_than(rhs); }
	bool operator>=(const Time& rhs)const { return !is_less_than(rhs); }
	bool operator==(const Time& rhs)const { return is_equal(rhs); }
	bool operator!=(const Time& rhs)const { return !is_equal(rhs); }

	template<typename U> const Time& operator+=(const U &rhs) { value_+=static_cast<value_type>(rhs); return *this; }
	template<typename U> const Time& operator-=(const U &rhs) { value_-=static_cast<value_type>(rhs); return *this; }
	template<typename U> const Time& operator*=(const U &rhs) { value_*=static_cast<value_type>(rhs); return *this; }
	template<typename U> const Time& operator/=(const U &rhs) { value_/=static_cast<value_type>(rhs); return *this; }

	template<typename U> Time operator+(const U &rhs)const { return value_+static_cast<value_type>(rhs); }
	template<typename U> Time operator-(const U &rhs)const { return value_-static_cast<value_type>(rhs); }
	template<typename U> Time operator*(const U &rhs)const { return value_*static_cast<value_type>(rhs); }
	template<typename U> Time operator/(const U &rhs)const { return value_/static_cast<value_type>(rhs); }

	Time operator-()const { return -value_; }
}; // END of class Time

//! This operator allows the combining of Time::Format flags using the '|' operator
/*!	\see Time::Format, Time::get_string() */
inline Time::Format operator|(Time::Format lhs, Time::Format rhs)
	{ return static_cast<Time::Format>((int)lhs|(int)rhs); }

//! This operator is for checking Time::Format flags.
/*! Don't think of it as "less then or equal to", but think of it
**	like an arrow. Is \a rhs inside of \a lhs ?
**	\see Time::Format, Time::get_string() */
inline bool operator<=(Time::Format lhs, Time::Format rhs)
	{ return (static_cast<int>(lhs) & static_cast<int>(rhs))==static_cast<int>(rhs); }

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
