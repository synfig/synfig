/* === S Y N F I G ========================================================= */
/*!	\file blinepoint.cpp
**	\brief Template File for a Width Point implementation
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <math.h>
#include "widthpoint.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

WidthPoint::WidthPoint():
	position_(0.0),
	width_(0.01)
{
	side_type_[0] = side_type_[1] = TYPE_INTERPOLATE;
}

WidthPoint::WidthPoint(Real position, Real width, int sidebefore, int sideafter):
	position_(position),
	width_(width)
{
	side_type_[0]=sidebefore;
	side_type_[1]=sideafter;
}

const Real&
WidthPoint::get_position()const
{
	return position_;
}

Real
WidthPoint::get_norm_position()const
{
	Real ret_pos(fabs(fmod(position_, Real(1.0f))));
	if(fabs(position_) >= 1.0 && ret_pos==0.0)
		return 1.0;
	return ret_pos;
}

void
WidthPoint::set_position(const Real& x)
{

	position_=x;
}

const Real&
WidthPoint::get_width()const
{
	return width_;
}

void
WidthPoint::set_width(Real x)
{
	width_=x;
}

int
WidthPoint::get_side_type_before()const
{
	return side_type_[0];
}

void
WidthPoint::set_side_type_before(int sidebefore)
{
	side_type_[0]=sidebefore;
}

int
WidthPoint::get_side_type_after()const
{
	return side_type_[1];
}

void
WidthPoint::set_side_type_after(int sideafter)
{
	side_type_[1]=sideafter;
}

int
WidthPoint::get_side_type(int i)const
{
	return i>0? side_type_[1]: side_type_[0];
}
