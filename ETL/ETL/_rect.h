/*! ========================================================================
** Extended Template Library
** Rectangle Basic Class Implementation
** $Id$
**
** Copyright (c) 2002 Adrian Bentley
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__RECT_H
#define __ETL__RECT_H

/* === H E A D E R S ======================================================= */

#include <functional>
#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

template < typename T >
class rect
{
public: //type niceties
	typedef T	value_type;

public: //representation

	value_type	minx,maxx,miny,maxy;

public: //interface

	rect() {}

	rect(const value_type &x1,const value_type &y1)
	{
		set_point(x1,y1);
	}

	rect(const value_type &x1,const value_type &y1,
			const value_type &x2,const value_type &y2)
	{
		set_point(x1,y1);
		expand(x2,y2);
	}

	rect(const rect<T> &o)
	:minx(o.minx),maxx(o.maxx),miny(o.miny),maxy(o.maxy)
	{}

	template < typename U >
	rect(const rect<U> &o)
	:minx(o.minx),maxx(o.maxx),miny(o.miny),maxy(o.maxy)
	{}

	void set_point(const value_type &x1,const value_type &y1)
	{
		minx = maxx = x1;
		miny = maxy = y1;
	}

	void expand(const value_type &x1,const value_type &y1)
	{
		minx = std::min(minx,x1);
		maxx = std::max(maxx,x1);
		miny = std::min(miny,y1);
		maxy = std::max(maxy,y1);
	}

	void set(const value_type &x1,const value_type &y1,
			const value_type &x2,const value_type &y2)
	{
		minx = x1; maxx = x2;
		miny = y1; maxy = y2;
	}

	//HACK HACK HACK (stupid compiler doesn't like default arguments of any type)
	bool valid() const
	{
		return valid(std::less<T>());
	}

	template < typename F >
	bool valid(const F & func) const
	{
		return func(minx,maxx) && func(miny,maxy);
	}
};

template < typename T, typename F >
inline bool intersect(const rect<T> &r1, const rect<T> &r2, const F & func)
{
	/* We wan to do the edge compare test
			  |-----|
		|------|	 intersecting

		|-----|
				|-----| not intersecting

		So we want to compare the mins of the one against the maxs of the other, and
		visa versa

		by default (exclude edge sharing) less will not be true if they are equal...
	*/

	return func(r1.minx,r2.maxx) &&
           func(r2.minx,r1.maxx) &&
           func(r1.miny,r2.maxy) &&
           func(r2.miny,r1.maxy);
}

template < typename T >
inline bool intersect(const rect<T> &r1, const rect<T> &r2)
{
	return intersect(r1,r2,std::less<T>());
}


template < typename T, typename F >
inline bool contains(const rect<T> &big, const rect<T> &small, const F & func)
{
	return !func(small.minx, big.minx) &&
		   !func(big.maxx, small.maxx) &&
		   !func(small.miny, big.miny) &&
		   !func(big.maxy, small.maxy);
}

template < typename T >
inline bool contains(const rect<T> &big, const rect<T> &small)
{
	return contains(big,small,std::less<T>());
}


template < typename T >
void set_intersect(rect<T> &rout, const rect<T> &r1, const rect<T> &r2)
{
	//takes the intersection of the two rectangles
	rout.minx = std::max(r1.minx,r2.minx);
	rout.miny = std::max(r1.miny,r2.miny);
	rout.maxx = std::min(r1.maxx,r2.maxx);
	rout.maxy = std::min(r1.maxy,r2.maxy);
}

template < typename T >
void set_union(rect<T> &rout, const rect<T> &r1, const rect<T> &r2)
{
	//takes the union of the two rectangles (bounds both... will contain extra info, but that's ok)
	rout.set(
		std::min(r1.minx,r2.minx),
		std::min(r1.miny,r2.miny),
		std::max(r1.maxx,r2.maxx),
		std::max(r1.maxy,r2.maxy));
	/*rect<T> local = r1;
	rout.expand(r2.minx,r2.miny);
	rout.expand(r2.maxx,r2.maxy);
	rout = local;*/
}

_ETL_END_NAMESPACE

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
