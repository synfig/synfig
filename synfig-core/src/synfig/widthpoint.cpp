/* === S Y N F I G ========================================================= */
/*!	\file widthpoint.cpp
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

#include <cmath>
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
	width_(0.01),
	dash_(false),
	lower_bound_(0.0),
	upper_bound_(1.0)
{
	side_type_[0] = side_type_[1] = TYPE_INTERPOLATE;
}

WidthPoint::WidthPoint(Real position, Real width, int sidebefore, int sideafter, bool dash):
	position_(position),
	width_(width)
{
	side_type_[0]=sidebefore;
	side_type_[1]=sideafter;
	dash_=dash;
	lower_bound_=0.0;
	upper_bound_=1.0;
}

const Real&
WidthPoint::get_position()const
{
	return position_;
}

Real
WidthPoint::get_norm_position(bool wplistloop)const
{
	return (get_bound_position(wplistloop)-lower_bound_)/(upper_bound_-lower_bound_);
}

Real
WidthPoint::get_bound_position(bool wplistloop)const
{
	Real ret(position_-lower_bound_);
	Real range(upper_bound_-lower_bound_);
	if(!wplistloop)
	{
		if (ret < 0) ret = 0;
		if (ret > range) ret = range;
	}
	else
	{
		ret=fmod(ret, range);
		ret=ret>=0.0?ret:(range+ret);
	}
	return ret+lower_bound_;
}


void
WidthPoint::normalize(bool loop)
{
	set_position(get_norm_position(loop));
}

void
WidthPoint::reverse()
{
	position_=upper_bound_-(position_-lower_bound_);
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

void
WidthPoint::set_dash(bool l)
{
	dash_=l;
}

bool
WidthPoint::get_dash()const
{
	return dash_;
}

void
WidthPoint::set_lower_bound(Real lb)
{
	lower_bound_=lb;
}

Real
WidthPoint::get_lower_bound()const
{
	return lower_bound_;
}

void
WidthPoint::set_upper_bound(Real ub)
{
	upper_bound_=ub;
}

Real
WidthPoint::get_upper_bound()const
{
	return upper_bound_;
}

bool
WidthPoint::operator<(const WidthPoint& rhs) const
{
	return get_position() < rhs.get_position();
}

bool
WidthPoint::operator == (const WidthPoint& rhs) const
{
	return 	side_type_[0] == rhs.get_side_type(0) &&
			side_type_[1] == rhs.get_side_type(1) &&
			position_ == rhs.get_position() &&
			width_ == rhs.get_width() &&
			lower_bound_==rhs.get_lower_bound() &&
			upper_bound_==rhs.get_upper_bound()
			;
}
