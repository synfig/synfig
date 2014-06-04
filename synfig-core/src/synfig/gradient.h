/* === S Y N F I G ========================================================= */
/*!	\file gradient.h
**	\brief Color Gradient Class
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#ifndef __SYNFIG_GRADIENT_H
#define __SYNFIG_GRADIENT_H

/* === H E A D E R S ======================================================= */

#include "real.h"
#include "color.h"
#include <vector>
#include <utility>
#include "uniqueid.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*! \struct GradientCPoint
**	\brief \todo
*/
struct GradientCPoint : public UniqueID
{
	Real pos;
	Color color;

	bool operator<(const GradientCPoint &rhs)const { return pos<rhs.pos; }
	bool operator<(const Real &rhs)const { return pos<rhs; }

	GradientCPoint(): pos() { }
	GradientCPoint(const Real &pos, const Color &color):pos(pos),color(color) { }
}; // END of class GradientCPoint


/*! \class Gradient
**	\brief Color Gradient Class
*/
using namespace std;
class Gradient
{
public:
	typedef GradientCPoint CPoint;
	typedef vector<CPoint> CPointList;
	typedef CPointList::const_iterator			const_iterator;
	typedef CPointList::iterator				iterator;
	typedef CPointList::const_reverse_iterator	const_reverse_iterator;
	typedef CPointList::reverse_iterator		reverse_iterator;
private:
	CPointList cpoints;
public:
	Gradient() { }

	//! Two-Tone Color Gradient Convenience Constructor
	Gradient(const Color &c1, const Color &c2);

	//! Three-Tone Color Gradient Convenience Constructor
	Gradient(const Color &c1, const Color &c2, const Color &c3);

	//! Alias for sort (Implemented for consistency)
	void sync() { sort(); }

	//! You should call this function after changing stuff.
	void sort();

	void push_back(const CPoint cpoint) { cpoints.push_back(cpoint); }
	iterator erase(iterator iter) { return cpoints.erase(iter); }
	bool empty()const { return cpoints.empty(); }
	size_t size()const { return cpoints.size(); }

	iterator begin() { return cpoints.begin(); }
	iterator end() { return cpoints.end(); }
	reverse_iterator rbegin() { return cpoints.rbegin(); }
	reverse_iterator rend() { return cpoints.rend(); }
	const_iterator begin()const { return cpoints.begin(); }
	const_iterator end()const { return cpoints.end(); }
	const_reverse_iterator rbegin()const { return cpoints.rbegin(); }
	const_reverse_iterator rend()const { return cpoints.rend(); }

	Gradient &operator+=(const Gradient &rhs);
	Gradient &operator-=(const Gradient &rhs);
	Gradient &operator*=(const float    &rhs);
	Gradient &operator/=(const float    &rhs);

	Gradient operator+(const Gradient &rhs)const { return Gradient(*this)+=rhs; }
	Gradient operator-(const Gradient &rhs)const { return Gradient(*this)-=rhs; }
	Gradient operator*(const float    &rhs)const { return Gradient(*this)*=rhs; }
	Gradient operator/(const float    &rhs)const { return Gradient(*this)/=rhs; }

	Color operator()(const Real &x, float supersample=0)const;

	Real mag()const;

	//! Returns the iterator of the CPoint closest to \a x
	iterator proximity(const Real &x);

	//! Returns the const_iterator of the CPoint closest to \a x
	const_iterator proximity(const Real &x)const;

	//! Returns the iterator of the CPoint with UniqueID \a id
	iterator find(const UniqueID &id);

	//! Returns the const_iterator of the CPoint with UniqueID \a id
	const_iterator find(const UniqueID &id)const;
}; // END of class Gradient

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
