/*! ========================================================================
** Extended Template Library
** Rectangle Basic Class Implementation
** $Id$
**
** Copyright (c) 2002 Adrian Bentley
** ......... ... 2018 Ivan Mahonin
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

namespace etl {

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
		return min < other.min ? true
			 : other.min < min ? false
			 : max < other.max;
	}
	bool operator==(const range &other) const
		{ return min == other.min && max == other.max; }
	bool operator!=(const range &other) const
		{ return !(*this == other); }
};

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
inline bool intersect(const rect<T> &r1, const rect<T> &r2, const F &less)
{
	return less(r1.minx, r2.maxx) &&
		   less(r2.minx, r1.maxx) &&
		   less(r1.miny, r2.maxy) &&
		   less(r2.miny, r1.maxy);
}
template<typename T>
inline bool intersect(const rect<T> &r1, const rect<T> &r2)
	{ return intersect(r1, r2, std::less<T>()); }


template<typename T, typename F>
inline bool contains(const rect<T> &big, const rect<T> &small, const F &less)
{
	return !less(small.minx, big.minx) &&
		   !less(big.maxx, small.maxx) &&
		   !less(small.miny, big.miny) &&
		   !less(big.maxy, small.maxy);
}
template<typename T>
inline bool contains(const rect<T> &big, const rect<T> &small)
	{ return contains(big, small, std::less<T>()); }


//! Takes the intersection of the two rectangles
template<typename T, typename F>
void set_intersect(rect<T> &rout, const rect<T> &r1, const rect<T> &r2, const F &less)
{
	rout.minx = std::max(r1.minx, r2.minx, less);
	rout.miny = std::max(r1.miny, r2.miny, less);
	rout.maxx = std::min(r1.maxx, r2.maxx, less);
	rout.maxy = std::min(r1.maxy, r2.maxy, less);
}
template<typename T>
void set_intersect(rect<T> &rout, const rect<T> &r1, const rect<T> &r2)
	{ return set_intersect(rout, r1, r2, std::less<T>()); }


//! Takes the union of the two rectangles (bounds both... will contain extra info, but that's ok)
template<typename T, typename F>
void set_union(rect<T> &rout, const rect<T> &r1, const rect<T> &r2, const F &less)
{
	rout.minx = std::min(r1.minx, r2.minx, less);
	rout.miny = std::min(r1.miny, r2.miny, less);
	rout.maxx = std::max(r1.maxx, r2.maxx, less);
	rout.maxy = std::max(r1.maxy, r2.maxy, less);
}
template<typename T>
void set_union(rect<T> &rout, const rect<T> &r1, const rect<T> &r2)
	{ set_union(rout, r1, r2, std::less<T>()); }


template<typename List, typename T, typename F>
void rects_subtract(List &list, const rect<T> &r, const F &less)
{
	typedef typename List::value_type Rect;

	if (!r.valid(less)) return;
	for(typename List::iterator i = list.begin(); i != list.end(); ) {
		Rect &x = *i;
		Rect y;
		set_intersect(y, x, r, less);
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

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif
