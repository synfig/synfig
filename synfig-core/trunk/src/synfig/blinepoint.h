/* === S Y N F I G ========================================================= */
/*!	\file blinepoint.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_BLINEPOINT_H
#define __SYNFIG_BLINEPOINT_H

/* === H E A D E R S ======================================================= */

#include "vector.h"
#include "uniqueid.h"
#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class BLinePoint : public UniqueID
{
private:
	Point	vertex_;
	Vector	tangent_[2];
	float	width_;
	float	origin_;
	bool	split_tangent_;
	bool	boned_vertex_;
	Point	vertex_setup_;

public:

	BLinePoint():
		vertex_(Point(0,0)),
		width_(0.01),
		origin_(0.0),
		split_tangent_(false),
		boned_vertex_(false),
		vertex_setup_(vertex_)
	{ tangent_[0] = Point(0,0); tangent_[1] = Point(0,0); }

	const Point& get_vertex()const { return vertex_; }
	void set_vertex(const Point& x) { vertex_=x; vertex_setup_=vertex_;}


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

	const bool& get_boned_vertex_flag()const { return boned_vertex_; }
	void set_boned_vertex_flag(bool x=true) { boned_vertex_=x; }

	const Vector& get_vertex_setup()const { return vertex_setup_; }
	void set_vertex_setup(Vector& x) { vertex_setup_=x; }

	void reverse();

}; // END of class BLinePoint

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
