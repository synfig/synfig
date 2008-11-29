/* === S Y N F I G ========================================================= */
/*!	\file bone.h
**	\brief Bone Header
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

#ifndef __SYNFIG_BONE_H
#define __SYNFIG_BONE_H

/* === H E A D E R S ======================================================= */
#include "matrix.h"
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig {

class Bone: public UniqueID
{
private:
	//!This is the current origin of the bone relative to parent
	Point origin_;
	//!This is the origin of the bone at the setup time
	Point origin0_;
	//!This is the current angle if the bone relative to parent.
	Angle angle_;
	//!This is the angle of the bone at the setup time
	Angle angle0_;
	//!This is the current scale of the bone.
	Real scale_;
//	Real scale0_; // Scale0 is always = 1.0
	//!This is the length at setup time
	Real length;
	//!A pointer to the parent bone.
	Bone *parent_;
public:
	//!Default constructor
	Bone();
	//!Constructor by origin and tip
	Bone(const Point &origin, const Point &tip);
	//!Construtor by origin, legth and parent (default no parent)
	Bone(const Point origin, const Angle angle, const Real length, const Bone* p=0);
	//!Wrappers for origin_ & origin0_
	const Point& get_origin() {return origin_;}
	void set_origin(const Point &x) {origin_=x;}
	const Point& get_origin0() {return origin0_;}
	void set_origin0(const Point &x) {origin0_=x;}
	//!Wrappers for angle_ & angle0_
	const Angle& get_angle() {return angle_;}
	void set_origin(const Angle &x) {angle_=x;}
	const Angle& get_angle0() {return angle0_;}
	void set_angle0(const Angle &x) {angle0_=x;}
	//!Wrapper for scale
	const Real& get_scale() {return scale_;}
	void set_scale(const Real &x) {scale_=x;}
	//!Wrapper for lenght. Notice that a length of 0 is not allowed.
	const Real& get_lenght() {return length_;}
	void set_length(const Real &x) {x<0.00001:length_=0.00001:length_=x;}
	//!This gets the calculated tip of the bone based on
	//!tip=origin+[length,0]*Rotate(alpha)*Scalex(scale)
	const Point & get_tip();
	//!Wrapper for parent bone
	Bone *get_parent() {return parent_;}
	void set_parent(Bone *p) {parent_=p;}

}; // END of class Bone

} // END of namespace synfig
/* === E N D =============================================================== */

#endif
