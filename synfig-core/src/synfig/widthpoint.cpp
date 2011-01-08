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
	cup_type_[0] = cup_type_[1] = CUPTYPE_INTERPOLATE;
}

WidthPoint::WidthPoint(Real position, Real width, int cupbefore, int cupafter):
	position_(position),
	width_(width)
{
	cup_type_[0]=cupbefore;
	cup_type_[1]=cupafter;
}

const Real&
WidthPoint::get_position()const
{
	return position_;
}

const Real&
WidthPoint::get_norm_position()const
{
	Real ret_pos(fabs(fmod(position_, 1.0f));
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
WidthPoint::get_cup_type_before()
{
	return cup_type_[0];
}

void
WidthPoint::set_cup_type_before(int cupbefore)
{
	cup_type_[0]=cupbefore;
}

int
WidthPoint::get_cup_type_after()
{
	return cup_type_[1];
}

void
WidthPoint::set_cup_type_after(int cupafter)
{
	cup_type_[1]=cupafter;
}

int
WidthPoint::get_cup_type(int i)
{
	return i>0? cup_type_[1]: cup_type_[0];
}
