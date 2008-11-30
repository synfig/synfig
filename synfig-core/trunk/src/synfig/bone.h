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
#include "uniqueid.h"
/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
namespace synfig {

class Bone: public UniqueID
{
	/*
 --	** -- T Y P E S -----------------------------------------------------------
	*/

public:
	// typedef etl::handle<Bone> Handle;
	// typedef etl::loose_handle<Bone> LooseHandle;

	/*
 --	** -- D A T A -------------------------------------------------------------
	*/

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
	Real length_;
	//!This is the strength at setup time
	Real strength_;
	//!The parent bone.
	const Bone *parent_;
public:
	//!Default constructor
	Bone();
	//!Constructor by origin and tip
	Bone(const Point &origin, const Point &tip);
	//!Construtor by origin, legth and parent (default no parent)
	Bone(const Point &origin, const Angle &angle, const Real &length, const Real &strength, Bone* p=0);

	//!Wrappers for origin_ & origin0_
	const Point& get_origin() {return origin_;}
	void set_origin(const Point &x) {origin_=x;}
	const Point& get_origin0() {return origin0_;}
	void set_origin0(const Point &x) {origin0_=x;}

	//!Wrappers for angle_ & angle0_
	const Angle& get_angle() {return angle_;}
	void set_angle(const Angle &x) {angle_=x;}
	const Angle& get_angle0() {return angle0_;}
	void set_angle0(const Angle &x) {angle0_=x;}

	//!Wrapper for scale
	const Real& get_scale() {return scale_;}
	void set_scale(const Real &x) {scale_=x;}

	//!Wrapper for length. Notice that a length of 0 is not allowed.
	const Real& get_length() {return length_;}
	void set_length(const Real &x) {length_=x<0.00001?0.00001:x;}

	//!Wrapper for strength
	const Real& get_strength() {return strength_;}
	void set_strength(const Real &x) {strength_=x;}

	//!This gets the calculated tip of the bone based on
	//!tip=origin+[length,0]*Rotate(alpha)*Scalex(scale)
	Point get_tip();

	//!Wrapper for parent bone
	const Bone &get_parent() {return *parent_;}
	void set_parent(const Bone &p) {parent_=&p;}

	//!Setup Transfomration matrix.
	//!This matrix applied to a setup point in global
	//!coordinates calculates the local coordinates of
	//!the point relative to the current bone.
	Matrix get_setup_matrix();
}; // END of class Bone

}; // END of namespace synfig
/* === E N D =============================================================== */

#endif
