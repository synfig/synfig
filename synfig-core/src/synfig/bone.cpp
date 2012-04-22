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
#include "valuenode_bone.h"
#include <ETL/stringf>
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
	origin0_(Point(0,0)),
	angle_(Angle::deg(0.0)),
	angle0_(Angle::deg(0.0)),
	scalelx_(1.0), scalely_(1.0),
	scalex_(1.0),  scaley_(1.0),
	length_(1.0),
	strength_(1.0),
	setup_(false),
	parent_(0)
{
	if (getenv("SYNFIG_DEBUG_NEW_BONES"))
		printf("%s:%d new bone\n", __FILE__, __LINE__);
}

//!Constructor by origin and tip
Bone::Bone(const Point &o, const Point &t):
	origin_(o),
	origin0_(o),
	angle_((t-o).angle()),
	angle0_((t-o).angle()),
	scalelx_(1.0), scalely_(1.0),
	scalex_(1.0),  scaley_(1.0),
	length_(1.0),
	strength_(1.0),
	setup_(false),
	parent_(0)
{
	if (getenv("SYNFIG_DEBUG_NEW_BONES"))
		printf("%s:%d new bone\n", __FILE__, __LINE__);
}

//!Constructor by origin, angle, length, strength, parent bone (default = no parent)
Bone::Bone(const String &n, const Point &o, const Angle &a, const Real &l, const Real &s, ValueNode_Bone* p):
	name_(n),
	origin_(o),
	origin0_(o),
	angle_(a),
	angle0_(a),
	scalelx_(1.0), scalely_(1.0),
	scalex_(1.0),  scaley_(1.0),
	length_(l),
	strength_(s),
	setup_(false),
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
	return strprintf("N=%s O=(%.4f %.4f) O0=(%.4f %.4f) a=%.4f a0=%.4f slx=%.4f sly=%.4f sx=%.4f sy=%.4f l=%.4f St=%.4f Se=%d P=%lx",
					 name_.c_str(),
					 origin_[0], origin_[1],
					 origin0_[0], origin0_[1],
					 Angle::deg(angle_).get(),
					 Angle::deg(angle0_).get(),
					 scalelx_, scalely_, scalex_, scaley_, length_, strength_, setup_, uintptr_t(parent_));
}

bool
Bone::is_root()
{
	return get_parent()->is_root();
}

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */
