/* === S I N F G =========================================================== */
/*!	\file blinepoint.h
**	\brief Template Header
**
**	$Id: blinepoint.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_BLINEPOINT_H
#define __SINFG_BLINEPOINT_H

/* === H E A D E R S ======================================================= */

#include "vector.h"
#include "uniqueid.h"
#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {

class BLinePoint : public UniqueID
{
private:
	Point	vertex_;
	Vector	tangent_[2];
	float	width_;
	float	origin_;
	bool	split_tangent_;

public:

	BLinePoint():
		vertex_(Point(0,0)),
		width_(0.01),
		origin_(0.0),
		split_tangent_(false)
	{ }

	const Point& get_vertex()const { return vertex_; }
	void set_vertex(const Point& x) { vertex_=x; }


	const Vector& get_tangent1()const { return tangent_[0]; }
	const Vector& get_tangent2()const { return split_tangent_?tangent_[1]:tangent_[0]; }
	void set_tangent(const Vector& x) { tangent_[0]=tangent_[1]=x; }
	void set_tangent1(const Vector& x) { tangent_[0]=x; }
	void set_tangent2(const Vector& x) { tangent_[1]=x; }


	const float& get_width()const { return width_; }
	void set_width(float x) { width_=x; }

	// We store the origin offset by 0.5 so that
	// can have the origin set to the default by zeroing
	// out the structure.
	float get_origin()const { return origin_+0.5f; }
	void set_origin(float x) { origin_=x-0.5f; }


	const bool& get_split_tangent_flag()const { return split_tangent_; }
	void set_split_tangent_flag(bool x=true) { split_tangent_=x; }

	void reverse();
	
}; // END of class BLinePoint
	
}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
