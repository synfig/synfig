/* === S Y N F I G ========================================================= */
/*!	\file bone.cpp
**	\brief Bone File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Carlos López & Chirs Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "bone.h"
#include "guid.h"
#include "valuenodes/valuenode_bone.h"
#include <ETL/stringf>
#include <cmath>
#include <inttypes.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

//! Default constructor
Bone::Bone():
	origin_(Point(0,0)),
	angle_(Angle::deg(0.0)),
	scalelx_(1.0),
	scalex_(1.0),
	length_(1.0),
	width_(0.1),
	tipwidth_(0.1),
	depth_(0.0),
	parent_(nullptr)
{
	if (getenv("SYNFIG_DEBUG_NEW_BONES"))
		printf("%s:%d new bone\n", __FILE__, __LINE__);
}

//!Constructor by origin and tip
Bone::Bone(const Point &o, const Point &t):
	origin_(o),
	angle_((t-o).angle()),
	scalelx_(1.0),
	scalex_(1.0),
	length_(1.0),
	width_(0.3),
	tipwidth_(0.3),
	depth_(0.0),
	parent_(nullptr)
{
	if (getenv("SYNFIG_DEBUG_NEW_BONES"))
		printf("%s:%d new bone\n", __FILE__, __LINE__);
}

//!Constructor by origin, angle, length, strength, parent bone (default = no parent)
Bone::Bone(const String &n, const Point &o, const Angle &a, const Real &l, ValueNode_Bone* p):
	name_(n),
	origin_(o),
	angle_(a),
	scalelx_(1.0),
	scalex_(1.0),
	length_(l),
	width_(0.3),
	tipwidth_(0.3),
	depth_(0.0),
	parent_(p)
{
	if (getenv("SYNFIG_DEBUG_NEW_BONES"))
		printf("%s:%d new bone\n", __FILE__, __LINE__);
}

const ValueNode_Bone*
Bone::get_parent()const
{
	return parent_;
}

void
Bone::set_parent(const ValueNode_Bone* parent)
{
	parent_ = parent;
}

//! get_tip() member function
//!@return The tip Point of the bone (calculated) based on
//! tip=origin+[length,0]*Scalex(scalex*scalelx,0)*Rotate(alpha)
Point
Bone::get_tip()
{
	Matrix s, r, sr;
	s.set_scale(scalex_*scalelx_,0);
	r.set_rotate(angle_);
	sr=s*r;
	return (Point)sr.get_transformed(Vector(length_,0));
}

//!Get the string of the Bone
//!@return String type. A string representation of the bone
//!components.
synfig::String
Bone::get_string()const
{
	return strprintf("N=%s O=(%.4f %.4f) a=%.4f slx=%.4f sx=%.4f l=%.4f w=%.4f tw=%.4f or=%.4f P=%lx",
					 name_.c_str(),
					 origin_[0], origin_[1],
					 Angle::deg(angle_).get(),
					 scalelx_, scalex_, length_, width_, tipwidth_, depth_, uintptr_t(parent_));
}

bool
Bone::is_root()const
{
	return get_parent()->is_root();
}

Bone::Shape
Bone::get_shape() const
{
	Matrix matrix = get_animated_matrix();
	Vector origin = matrix.get_transformed(Vector(0.0, 0.0));
	Vector direction = matrix.get_transformed(Vector(1.0, 0.0), false).norm();
	Real length = get_length() * get_scalelx();

	if (length < 0) {
		length *= -1;
		direction *= -1;
	}

	Shape shape;
	shape.p0 = origin;
	shape.p1 = origin + direction * length;

	shape.r0 = fabs(get_width());
	shape.r1 = fabs(get_tipwidth());

	return shape;
}

Real
Bone::distance_to_shape_center_percent(const Shape &shape, const Vector &x)
{
	static const Real precision = 0.000000001;

	const Vector &p0 = shape.p0;
	const Vector &p1 = shape.p1;
	Real r0 = fabs(shape.r0);
	Real r1 = fabs(shape.r1);

	Real length = (p1 - p0).mag();

	Real percent_p0 = r0 > precision ? 1.0 - (x - p0).mag()/r0 : 0.0;
	Real percent_p1 = r1 > precision ? 1.0 - (x - p1).mag()/r1 : 0.0;

	// check line
	Real percent_line = 0.0;
	if (length + precision > fabs(r1 - r0))
	{
		Real cos0 = (r0 - r1)/length;
		Real cos1 = -cos0;

		Real sin0 = sqrt(1 + precision - cos0*cos0);
		Real sin1 = sin0;

		Real ll = length - r0*cos0 - r1*cos1;
		Vector direction = (p1 - p0)/length;
		Vector pp0(p0 + direction * (r0*cos0));
		Vector pp1(p0 + direction * (length - r1*cos1));
		Real rr0 = r0*sin0;
		Real rr1 = r1*sin1;

		Real pos_at_line = (x - pp0)*direction/ll;
		if (pos_at_line > 0.0 && pos_at_line < 1.0)
		{
			Real distance = fabs((x - pp0)*direction.perp());
			Real max_distance = rr0*(1.0 - pos_at_line) + rr1*pos_at_line;
			if (max_distance > 0.0) percent_line = 1.0 - distance/max_distance;
		}
	}

	Real percent = 0.0;
	if (percent_p0 > percent) percent = percent_p0;
	if (percent_p1 > percent) percent = percent_p1;
	if (percent_line > percent) percent = percent_line;
	return percent;
}

Real
Bone::influence_function(Real x)
{
	return sin(x*PI/2.0);
}


/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
