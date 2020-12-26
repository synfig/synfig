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

#include "real.h"
#include "vector.h"
#include <limits>
#include <cmath>
#include <algorithm> //std::min/max

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {


template<typename T>
class range {
public:
	typedef T value_type;
	value_type min, max;

	range(const value_type &min, const value_type &max):
			min(min), max(max) { }
	explicit range(const value_type &x = value_type()):
			range(x, x) { }

	range& set(const value_type &min, const value_type &max)
	{ this->min = min; this->max = max; return *this; }
	range& set(const value_type &x)
	{ return set(x, x); }
	range& expand(const value_type &x) {
		if (x < min) min = x;
		if (max < x) max = x;
		return *this;
	}

	bool valid() const
	{ return min < max; }
	value_type size() const
	{ return max - min; }

	bool operator<(const range &other) const {
		return min == other.min ? max < other.max : min < other.min;
	}
	bool operator==(const range &other) const
	{ return min == other.min && max == other.max; }
	bool operator!=(const range &other) const
	{ return !(*this == other); }
};

typedef range<Real> Range;

template<typename T>
class rect
{
public:
	typedef T value_type;

	value_type minx, miny, maxx, maxy;

	rect():
			minx(), miny(), maxx(), maxy() { }
	rect(const value_type &x, const value_type &y):
			minx(x), miny(y), maxx(x), maxy(y) { }

	template<typename U>
	rect(const rect<U> &o):
			minx(o.minx), miny(o.miny), maxx(o.maxx), maxy(o.maxy) { }
	rect(const rect<T> &o):
			minx(o.minx), miny(o.miny), maxx(o.maxx), maxy(o.maxy) { }

	template<typename F>
	rect(
			const value_type &x0,
			const value_type &y0,
			const value_type &x1,
			const value_type &y1,
			const F &less
	):
			minx(x0), miny(y0), maxx(x0), maxy(y0)
	{ expand(x1, y1, less); }
	rect(
			const value_type &x0,
			const value_type &y0,
			const value_type &x1,
			const value_type &y1
	):
			minx(x0), miny(y0), maxx(x0), maxy(y0)
	{ expand(x1, y1); }

	template<typename F>
	bool valid(const F &less) const
	{ return less(minx, maxx) && less(miny, maxy); }
	bool valid() const
	{ return valid(std::less<T>()); }

	void set(
			const value_type &x0,
			const value_type &y0,
			const value_type &x1,
			const value_type &y1 )
	{ minx = x0; miny = y0; maxx = x1; maxy = y1; }
	void set_point(const value_type &x, const value_type &y)
	{ minx = maxx = x; miny = maxy = y; }

	template<typename F>
	void expand(const value_type &x1, const value_type &y1, const F &less)
	{
		minx = std::min(minx, x1, less);
		miny = std::min(miny, y1, less);
		maxx = std::max(maxx, x1, less);
		maxy = std::max(maxy, y1, less);
	}
	void expand(const value_type &x1, const value_type &y1)
	{ expand(x1, y1, std::less<T>()); }
};


//! We want to do the edge compare test
//!          |-----|
//!    |------|        intersecting
//!
//!    |-----|
//!            |-----| not intersecting
//!
//! So we want to compare the mins of the one against the maxs of the other, and visa versa.
//! By default (exclude edge sharing) less will not be true if they are equal...
template<typename T, typename F>
inline bool rect_intersect(const rect<T> &r1, const rect<T> &r2, const F &less)
{
	return less(r1.minx, r2.maxx) &&
	       less(r2.minx, r1.maxx) &&
	       less(r1.miny, r2.maxy) &&
	       less(r2.miny, r1.maxy);
}
template<typename T>
inline bool rect_intersect(const rect<T> &r1, const rect<T> &r2)
{ return rect_intersect(r1, r2, std::less<T>()); }


template<typename T, typename F>
// MSVC defines `small` type. To avoid confusion, we use the name `small_rect` instead of `small`.
inline bool rect_contains(const rect<T> &big, const rect<T> &small_rect, const F &less)
{
	return !less(small_rect.minx, big.minx) &&
	       !less(big.maxx, small_rect.maxx) &&
	       !less(small_rect.miny, big.miny) &&
	       !less(big.maxy, small_rect.maxy);
}
template<typename T>
inline bool rect_contains(const rect<T> &big, const rect<T> &small_rect)
{ return rect_contains(big, small_rect, std::less<T>()); }


//! Takes the intersection of the two rectangles
template<typename T, typename F>
void rect_set_intersect(rect<T> &rout, const rect<T> &r1, const rect<T> &r2, const F &less)
{
	rout.minx = std::max(r1.minx, r2.minx, less);
	rout.miny = std::max(r1.miny, r2.miny, less);
	rout.maxx = std::min(r1.maxx, r2.maxx, less);
	rout.maxy = std::min(r1.maxy, r2.maxy, less);
}
template<typename T>
void rect_set_intersect(rect<T> &rout, const rect<T> &r1, const rect<T> &r2)
{ return rect_set_intersect(rout, r1, r2, std::less<T>()); }


//! Takes the union of the two rectangles (bounds both... will contain extra info, but that's ok)
template<typename T, typename F>
void rect_set_union(rect<T> &rout, const rect<T> &r1, const rect<T> &r2, const F &less)
{
	rout.minx = std::min(r1.minx, r2.minx, less);
	rout.miny = std::min(r1.miny, r2.miny, less);
	rout.maxx = std::max(r1.maxx, r2.maxx, less);
	rout.maxy = std::max(r1.maxy, r2.maxy, less);
}
template<typename T>
void rect_set_union(rect<T> &rout, const rect<T> &r1, const rect<T> &r2)
{ rect_set_union(rout, r1, r2, std::less<T>()); }


template<typename List, typename T, typename F>
void rects_subtract(List &list, const rect<T> &r, const F &less)
{
	typedef typename List::value_type Rect;

	if (!r.valid(less)) return;
	for(typename List::iterator i = list.begin(); i != list.end(); ) {
		Rect &x = *i;
		Rect y;
		rect_set_intersect(y, x, r, less);
		if (less(y.minx, y.maxx) && less(y.miny, y.maxy)) {
			T rects[][4] = {
					{ x.minx, x.miny, y.minx, x.maxy },
					{ y.maxx, x.miny, x.maxx, x.maxy },
					{ y.minx, x.miny, y.maxx, y.miny },
					{ y.minx, y.maxy, y.maxx, x.maxy } };
			const int count = sizeof(rects)/sizeof(rects[0]);

			i = list.erase(i);
			for(int j = 0; j < count; ++j) {
				if ( less(rects[j][0], rects[j][2])
				     && less(rects[j][1], rects[j][3]) )
				{
					Rect rr;
					rr.minx = rects[j][0];
					rr.miny = rects[j][1];
					rr.maxx = rects[j][2];
					rr.maxy = rects[j][3];
					i = list.insert(i, rr);
					++i;
				}
			}
		} else ++i;
	}
}
template<typename List, typename T>
void rects_subtract(List &list, const rect<T> &r)
{ rects_subtract(list, r, std::less<T>()); }


template<typename List, typename T, typename F>
void rects_add(List &list, const rect<T> &r, const F &less)
{
	if (!r.valid(less)) return;
	rects_subtract(list, r, less);
	list.insert(list.end(), r);
}
template<typename List, typename T>
void rects_add(List &list, const rect<T> &r)
{ rects_add(list, r, std::less<T>()); }


template<typename List, typename F>
void rects_merge(List &list, const F &less)
{
	for(typename List::iterator i = list.begin(); i != list.end(); )
		if (!less(i->minx, i->maxx) || !less(i->miny, i->maxy))
			i = list.erase(i); else ++i;

	bool merged_any = true;
	while(merged_any) {
		merged_any = false;
		for(typename List::iterator i = list.begin(); i != list.end(); )
		{
			bool merged_current = false;
			for(typename List::iterator j = list.begin(); j != list.end(); ++j) {
				if (i == j) continue;

				// merge horizontal
				if ( !less(i->maxx, j->minx) && !less(j->minx, i->maxx)
				     && !less(i->miny, j->miny) && !less(j->miny, i->miny) )
				{
					j->miny = i->miny;
					i = list.erase(i);
					merged_current = true;
					break;
				}

				// merge vertical
				if ( !less(i->minx, j->minx) && !less(j->minx, i->minx)
				     && !less(i->maxy, j->miny) && !less(j->miny, i->maxy) )
				{
					j->miny = i->miny;
					i = list.erase(i);
					merged_current = true;
					break;
				}
			}
			if (merged_current) merged_any = true; else ++i;
		}
	}
}
template<typename List>
void rects_merge(List &list)
{
	typedef typename List::value_type R;
	typedef typename R::value_type T;
	rects_merge(list, std::less<T>());
}


class RectInt : public rect<int>
{
public:
	typedef rect<int> baserect;

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
			rect_set_intersect(*this,*this,rhs);
		else
			*this=zero();
		return *this;
	}

	RectInt& operator|=(const RectInt& rhs)
	{
		if(rhs.valid()>0 && valid()>0)
			rect_set_union(*this,*this,rhs);
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

	bool operator&&(const RectInt& rhs)const { return valid() && rhs.valid() && rect_intersect(*this, rhs); }

	bool operator==(const RectInt &rhs)const { return get_min() == rhs.get_min() && get_max() == rhs.get_max(); }

	bool operator!=(const RectInt &rhs)const { return get_min() != rhs.get_min() || get_max() != rhs.get_max(); }

	bool contains(const RectInt &x)const { return rect_contains(*this, x); }

	bool is_valid()const { return valid(); }

	template<typename List>
	static void merge(List &list)
		{ rects_merge(list); }

	template<typename List>
	void list_add(List &list)
		{ rects_add(list, *this); merge(list); }

	template<typename List>
	void list_subtract(List &list)
		{ rects_subtract(list, *this); merge(list); }

	RectInt multiply_coords(const VectorInt &rhs) const
		{ return RectInt(minx*rhs[0], miny*rhs[1], maxx*rhs[0], maxy*rhs[1]); }
	RectInt divide_coords(const VectorInt &rhs) const
		{ return RectInt(minx/rhs[0], miny/rhs[1], maxx/rhs[0], maxy/rhs[1]); }
}; // END of class RectInt


class Rect : public rect<Real>
{
public:
	typedef rect<Real> baserect;

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

	bool is_inside(const Point& x) const
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
			rect_set_intersect(*this,*this,rhs);
		else
			*this=zero();
		return *this;
	}

	Rect& operator|=(const Rect& rhs)
	{
		if ( rhs.valid() && valid()
		  && rhs.area()>0.00000001 && area()>0.00000001 )
			rect_set_union(*this,*this,rhs);
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

	bool operator&&(const Rect& rhs)const { return valid() && rhs.valid() && rect_intersect(*this, rhs); }

	bool operator==(const Rect &rhs)const { return get_min() == rhs.get_min() && get_max() == rhs.get_max(); }

	bool operator!=(const Rect &rhs)const { return get_min() != rhs.get_min() || get_max() != rhs.get_max(); }

	bool contains(const Rect &x)const { return rect_contains(*this, x, approximate_less<Real>); }

	bool valid()const { return rect<value_type>::valid(approximate_less<Real>); }
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
		{ rects_merge(list, approximate_less<Real>); }

	template<typename List>
	void list_add(List &list)
		{ rects_add(list, *this, approximate_less<Real>); merge(list); }

	template<typename List>
	void list_subtract(List &list)
		{ rects_subtract(list, *this, approximate_less<Real>); merge(list); }

	Rect multiply_coords(const Vector &rhs) const
		{ return Rect(minx*rhs[0], miny*rhs[1], maxx*rhs[0], maxy*rhs[1]); }
	Rect divide_coords(const Vector &rhs) const
		{ return Rect(minx/rhs[0], miny/rhs[1], maxx/rhs[0], maxy/rhs[1]); }
}; // END of class Rect

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
