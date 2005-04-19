/* === S I N F G =========================================================== */
/*!	\file rect.h
**	\brief Rectangle Class
**
**	$Id: rect.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_RECT_H
#define __SINFG_RECT_H

/* === H E A D E R S ======================================================= */

#include <ETL/rect>
#include "real.h"
#include "vector.h"
#include <limits>
#include <cmath>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class Rect : public etl::rect<Real>
{
public:

	using etl::rect<Real>::set_point;
	using etl::rect<Real>::expand;
	using etl::rect<Real>::set;

	static Rect full_plane();

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

	bool is_valid()const { return valid(); }
}; // END of class Rect

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
