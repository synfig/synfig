/* === S Y N F I G ========================================================= */
/*!	\file widthpoint.h
**	\brief Template Header for the implementation of a Width Point
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Carlos López
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

#ifndef __SYNFIG_WIDTHPOINT_H
#define __SYNFIG_WIDTHPOINT_H

/* === H E A D E R S ======================================================= */

#include "uniqueid.h"
#include "real.h"
#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class WidthPoint : public UniqueID
{
private:
	Real position_;
	Real width_;
	int side_type_[2]; // Before [0] and After[1] side types
	bool dash_; // if true, widthpoint is used for dashed outlines
	Real lower_bound_; // The lower boundary of the withpoint position
	Real upper_bound_; // The upper boundary of the withpoint position

public:

	enum SideType
	{
		TYPE_INTERPOLATE     =0,
		TYPE_ROUNDED         =1,
		TYPE_SQUARED         =2,
		TYPE_PEAK            =3,
		TYPE_FLAT            =4,
		TYPE_INNER_ROUNDED   =5,
		TYPE_INNER_PEAK      =6,
	};

	WidthPoint();
	WidthPoint(Real position, Real width, int sidebefore=TYPE_INTERPOLATE,
		int sideafter=TYPE_INTERPOLATE, bool dash_=false);

	const Real& get_position()const;
	void set_position(const Real& x);
	// gets the normalised position: converts it to be inside [0,1]
	Real get_norm_position(bool wplistloop)const;
	// gets the position inside the lower and upper boundaries
	Real get_bound_position(bool wplistloop)const;
	// changes the widthpoint's position to be inside [0,1)
	void normalize(bool loop);
	// reverse its position inside boundaries
	void reverse();

	const Real& get_width()const;
	void set_width(Real x);

	int get_side_type_before()const;
	void set_side_type_before(int sidebefore);
	int get_side_type_after()const;
	void set_side_type_after(int sideafter);
	int get_side_type(int i)const;
	bool get_dash()const;
	void set_dash(bool l);
	Real get_lower_bound()const;
	void set_lower_bound(Real lb);
	Real get_upper_bound()const;
	void set_upper_bound(Real ub);
	bool operator < (const WidthPoint& rhs) const;
	bool operator == (const WidthPoint& rhs) const;

}; // END of class WidthPoint

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
