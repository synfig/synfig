/* === S I N F G =========================================================== */
/*!	\file distance.h
**	\brief Template Header
**
**	$Id: distance.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

/* === S T A R T =========================================================== */

#ifndef __SINFG_DISTANCE_H
#define __SINFG_DISTANCE_H

/* === H E A D E R S ======================================================= */

#include "real.h"
#include "string.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

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

	Distance(){ }
	Distance(const value_type& value, System system):value_(value),system_(system) { }
	explicit Distance(const sinfg::String& string);
	
	operator Real()const { return value_; }

	Distance& operator=(const Real& rhs) { value_=rhs; return *this; }

	Distance& operator=(const sinfg::String& rhs);

	sinfg::String get_string(int digits=4)const;
	
	const System& get_system()const { return system_; }

	const Real& get()const { return value_; }

	Real get(System system, const RendDesc& rend_desc)const;
	
	void convert(System system, const RendDesc& rend_desc);
	
	Real meters()const;
	Real meters(const RendDesc& rend_desc)const;
	Real units(const RendDesc& rend_desc)const;
	
	static Real meters_to_system(Real x, System target_system);
	static System ident_system(const sinfg::String& str);
	static sinfg::String system_name(System system);
	static sinfg::String system_local_name(System system);

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
	
}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
