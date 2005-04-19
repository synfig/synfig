/* === S Y N F I G ========================================================= */
/*!	\file gradient.h
**	\brief Color Gradient Class
**
**	$Id: gradient.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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
	
	GradientCPoint() { }
	GradientCPoint(const Real &pos, const Color &color):pos(pos),color(color) { }
}; // END of class GradientCPoint

	
/*! \class Gradient
**	\brief Color Gradient Class
*/
class Gradient : public std::vector<GradientCPoint>
{
public:
	typedef GradientCPoint CPoint;
private:
	
public:
	Gradient() { }
	
	//! Two-Tone Color Gradient Convience Constructor
	Gradient(const Color &c1, const Color &c2);

	//! Three-Tone Color Gradient Convience Constructor
	Gradient(const Color &c1, const Color &c2, const Color &c3);

	//! Alias for sort (Implemented for consistancy)
	void sync() { sort(); }

	//! You should call this function after changing stuff.
	void sort();
	
	Color operator()(const Real &x, float supersample=0)const;

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
