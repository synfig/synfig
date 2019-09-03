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


typedef etl::range<int> RangeInt;
typedef etl::range<Real> Range;


class RectInt : public etl::rect<int>
{
public:
	typedef etl::rect<int> baserect;

	using baserect::set_point;
	using baserect::expand;
	using baserect::set;

	static RectInt zero()
	{
		return RectInt(
			0,
			0,
			0,
			0
		);
	}

	RectInt(): baserect(0, 0, 0, 0) { }

	RectInt(const PointInt& x) { set_point(x); }

	RectInt(const PointInt& min, const PointInt& max) { set_point(min); expand(max); }

	RectInt(const value_type &x1,const value_type &y1)	{ set_point(x1,y1); }

	RectInt(const value_type &x1,const value_type &y1,
			const value_type &x2,const value_type &y2)
	{
		set_point(x1,y1);
		expand(x2,y2);
	}

	void set_point(const PointInt& max) { set_point(max[0],max[1]);	}

	RectInt& expand(const PointInt& max) { expand(max[0],max[1]); return *this; }

	RectInt& expand(const int& r) { minx-=r; miny-=r; maxx+=r; maxy+=r; return *this; }

	RectInt& expand_x(const int& r) { minx-=r; maxx+=r; return *this; }

	RectInt& expand_y(const int& r) { miny-=r; maxy+=r; return *this; }

	RectInt& set(const PointInt& min,const PointInt& max) { set(min[0],min[1],max[0],max[1]); return *this; }

	PointInt get_min()const { return PointInt(minx,miny); }
	PointInt get_max()const { return PointInt(maxx,maxy); }
	VectorInt get_size()const { return get_max() - get_min(); }
	value_type get_width()const { return maxx - minx; }
	value_type get_height()const { return maxy - miny; }

	bool is_inside(const PointInt& x) { return x[0]>=minx && x[0]<maxx && x[1]>=miny && x[1]<maxy; }

	int area()const
	{
		return (maxx-minx)*(maxy-miny);
	}

	// Operators

	RectInt& operator+=(const VectorInt& rhs)
	{
		minx+=rhs[0]; miny+=rhs[1];
		maxx+=rhs[0]; maxy+=rhs[1];
		return *this;
	}

	RectInt& operator-=(const VectorInt& rhs)
	{
		minx-=rhs[0]; miny-=rhs[1];
		maxx-=rhs[0]; maxy-=rhs[1];
		return *this;
	}

	RectInt& operator*=(const int& rhs)
	{
		minx*=rhs; miny*=rhs;
		maxx*=rhs; maxy*=rhs;
		return *this;
	}

	RectInt& operator/=(int rhs)
	{
		minx/=rhs; miny/=rhs;
		maxx/=rhs; maxy/=rhs;
		return *this;
	}

	RectInt& operator&=(const RectInt& rhs)
	{
		if(rhs.valid() && valid())
			etl::set_intersect(*this,*this,rhs);
		else
			*this=zero();
		return *this;
	}

	RectInt& operator|=(const RectInt& rhs)
	{
		if(rhs.valid()>0 && valid()>0)
			etl::set_union(*this,*this,rhs);
		else
		{
			if(area()<rhs.area())
				*this=rhs;
		}
		return *this;
	}

	RectInt operator+(const VectorInt& rhs)const { return RectInt(*this)+=rhs; }

	RectInt operator-(const VectorInt& rhs)const { return RectInt(*this)-=rhs; }

	RectInt operator*(const int& rhs)const { return RectInt(*this)*=rhs; }

	RectInt operator/(const int& rhs)const { return RectInt(*this)/=rhs; }

	RectInt operator&(const RectInt& rhs)const { return RectInt(*this)&=rhs; }

	RectInt operator|(const RectInt& rhs)const { return RectInt(*this)|=rhs; }

	bool operator&&(const RectInt& rhs)const { return valid() && rhs.valid() && etl::intersect(*this, rhs); }

	bool operator==(const RectInt &rhs)const { return get_min() == rhs.get_min() && get_max() == rhs.get_max(); }

	bool operator!=(const RectInt &rhs)const { return get_min() != rhs.get_min() || get_max() != rhs.get_max(); }

	bool contains(const RectInt &x)const { return etl::contains(*this, x); }

	bool is_valid()const { return valid(); }

	template<typename List>
	static void merge(List &list)
		{ etl::rects_merge(list); }

	template<typename List>
	void list_add(List &list)
		{ etl::rects_add(list, *this); merge(list); }

	template<typename List>
	void list_subtract(List &list)
		{ etl::rects_subtract(list, *this); merge(list); }

	RectInt multiply_coords(const VectorInt &rhs) const
		{ return RectInt(minx*rhs[0], miny*rhs[1], maxx*rhs[0], maxy*rhs[1]); }
	RectInt divide_coords(const VectorInt &rhs) const
		{ return RectInt(minx/rhs[0], miny/rhs[1], maxx/rhs[0], maxy/rhs[1]); }
}; // END of class RectInt


class Rect : public etl::rect<Real>
{
public:
	typedef etl::rect<Real> baserect;

	using baserect::set_point;
	using baserect::expand;
	using baserect::set;

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

	static Rect infinite()
	{
		return Rect(
			-INFINITY,
			-INFINITY,
			INFINITY,
			INFINITY
		);
	}

	Rect(): baserect(0, 0, 0, 0) { }

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
	Vector get_size()const { return get_max() - get_min(); }
	value_type get_width()const { return maxx - minx; }
	value_type get_height()const { return maxy - miny; }

	bool is_inside(const Point& x)
	{
		return approximate_less_or_equal(minx, x[0])
			&& approximate_less_or_equal(x[0], maxx)
			&& approximate_less_or_equal(miny, x[1])
			&& approximate_less_or_equal(x[1], maxy);
	}

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
		if ( rhs.valid() && valid()
		  && rhs.area()>0.00000001 && area()>0.00000001 )
			etl::set_intersect(*this,*this,rhs);
		else
			*this=zero();
		return *this;
	}

	Rect& operator|=(const Rect& rhs)
	{
		if ( rhs.valid() && valid()
		  && rhs.area()>0.00000001 && area()>0.00000001 )
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

	bool operator&&(const Rect& rhs)const { return valid() && rhs.valid() && etl::intersect(*this, rhs); }

	bool operator==(const Rect &rhs)const { return get_min() == rhs.get_min() && get_max() == rhs.get_max(); }

	bool operator!=(const Rect &rhs)const { return get_min() != rhs.get_min() || get_max() != rhs.get_max(); }

	bool contains(const Rect &x)const { return etl::contains(*this, x, approximate_less<Real>); }

	bool valid()const { return etl::rect<value_type>::valid(approximate_less<Real>); }
	bool is_valid()const { return valid(); }
	bool is_nan_or_inf()const
	{
		return std::isnan(minx)
			|| std::isnan(miny)
			|| std::isinf(maxx)
			|| std::isinf(maxy);
	}

	bool is_full_infinite()const
	{
		return std::isinf(minx)
			&& std::isinf(miny)
			&& std::isinf(maxx)
			&& std::isinf(maxy)
			&& minx < maxx
			&& miny < maxy;
	}

	template<typename List>
	static void merge(List &list)
		{ etl::rects_merge(list, approximate_less<Real>); }

	template<typename List>
	void list_add(List &list)
		{ etl::rects_add(list, *this, approximate_less<Real>); merge(list); }

	template<typename List>
	void list_subtract(List &list)
		{ etl::rects_subtract(list, *this, approximate_less<Real>); merge(list); }

	Rect multiply_coords(const Vector &rhs) const
		{ return Rect(minx*rhs[0], miny*rhs[1], maxx*rhs[0], maxy*rhs[1]); }
	Rect divide_coords(const Vector &rhs) const
		{ return Rect(minx/rhs[0], miny/rhs[1], maxx/rhs[0], maxy/rhs[1]); }
}; // END of class Rect

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
