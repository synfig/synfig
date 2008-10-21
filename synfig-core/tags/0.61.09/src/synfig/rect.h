/* === S Y N F I G ========================================================= */
/*!	\file rect.h
**	\brief Rectangle Class
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RECT_H
#define __SYNFIG_RECT_H

/* === H E A D E R S ======================================================= */

#include <ETL/rect>
#include "real.h"
#include "vector.h"
#include <limits>
#include <cmath>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Rect : public etl::rect<Real>
{
public:

	using etl::rect<Real>::set_point;
	using etl::rect<Real>::expand;
	using etl::rect<Real>::set;

	static Rect full_plane();

	static Rect horizontal_strip(const value_type &y1, const value_type &y2);
	static Rect vertical_strip(const value_type &x1, const value_type &x2);

	static Rect zero()
	{
		return Rect(
			0,
			0,
			0,
			0
		);
	}

	Rect() { }

	Rect(const Point& x) { set_point(x); }

	Rect(const Point& min, const Point& max) { set_point(min); expand(max); }

	Rect(const value_type &x1,const value_type &y1)	{ set_point(x1,y1); }

	Rect(const value_type &x1,const value_type &y1,
			const value_type &x2,const value_type &y2)
	{
		set_point(x1,y1);
		expand(x2,y2);
	}

	void set_point(const Point& max) { set_point(max[0],max[1]);	}

	Rect& expand(const Point& max) { expand(max[0],max[1]); return *this; }

	Rect& expand(const Real& r) { minx-=r; miny-=r; maxx+=r; maxy+=r; return *this; }

	Rect& expand_x(const Real& r) { minx-=r; maxx+=r; return *this; }

	Rect& expand_y(const Real& r) { miny-=r; maxy+=r; return *this; }

	Rect& set(const Point& min,const Point& max) { set(min[0],min[1],max[0],max[1]); return *this; }

	Point get_min()const { return Point(minx,miny); }
	Point get_max()const { return Point(maxx,maxy); }

	bool is_inside(const Point& x) { return x[0]>minx && x[0]<maxx && x[1]>miny && x[1]<maxy; }

	Real area()const
	{
		return (maxx-minx)*(maxy-miny);
	}

	// Operators

	Rect& operator+=(const Vector& rhs)
	{
		minx+=rhs[0]; miny+=rhs[1];
		maxx+=rhs[0]; maxy+=rhs[1];
		return *this;
	}

	Rect& operator-=(const Vector& rhs)
	{
		minx-=rhs[0]; miny-=rhs[1];
		maxx-=rhs[0]; maxy-=rhs[1];
		return *this;
	}

	Rect& operator*=(const Real& rhs)
	{
		minx*=rhs; miny*=rhs;
		maxx*=rhs; maxy*=rhs;
		return *this;
	}

	Rect& operator/=(Real rhs)
	{
		rhs=1.0/rhs; // Avoid doing several divisions
		minx*=rhs; miny*=rhs;
		maxx*=rhs; maxy*=rhs;
		return *this;
	}

	Rect& operator&=(const Rect& rhs)
	{
		if(rhs.area()>0.00000001 && area()>0.00000001)
			etl::set_intersect(*this,*this,rhs);
		else
			*this=zero();
		return *this;
	}

	Rect& operator|=(const Rect& rhs)
	{
		if(rhs.area()>0.00000001 && area()>0.00000001)
			etl::set_union(*this,*this,rhs);
		else
		{
			if(area()<rhs.area())
				*this=rhs;
		}
		return *this;
	}

	Rect operator+(const Vector& rhs)const { return Rect(*this)+=rhs; }

	Rect operator-(const Vector& rhs)const { return Rect(*this)-=rhs; }

	Rect operator*(const Real& rhs)const { return Rect(*this)*=rhs; }

	Rect operator/(const Real& rhs)const { return Rect(*this)/=rhs; }

	Rect operator&(const Rect& rhs)const { return Rect(*this)&=rhs; }

	Rect operator|(const Rect& rhs)const { return Rect(*this)|=rhs; }

	bool operator&&(const Rect& rhs)const { return etl::intersect(*this, rhs); }

	bool operator==(const Rect &rhs)const { return get_min() == rhs.get_min() && get_max() == rhs.get_max(); }

	bool operator!=(const Rect &rhs)const { return get_min() != rhs.get_min() || get_max() != rhs.get_max(); }

	bool is_valid()const { return valid(); }
}; // END of class Rect

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
