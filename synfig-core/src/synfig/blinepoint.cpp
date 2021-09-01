/* === S Y N F I G ========================================================= */
/*!	\file blinepoint.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "blinepoint.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
synfig::BLinePoint::reverse()
{
	if(split_tangent_both_)
	{
		std::swap(tangent_[0],tangent_[1]);
		tangent_[0]=-tangent_[0];
		tangent_[1]=-tangent_[1];
	}
	else
	if(merge_tangent_both_)
	{
		tangent_[0]=-tangent_[0];
		tangent_[1]=-tangent_[1];
	}
	else
	if(split_tangent_radius_)
	{
		Real mag0 = tangent_[0].mag(), mag1 = tangent_[1].mag();
		tangent_[0]=Vector(-mag1, mag0==0?Angle::rad(0):tangent_[0].angle());
		tangent_[1]=Vector(-mag0, mag1==0?Angle::rad(0):tangent_[1].angle());
		update_tangent2();
	}
	else
	{
		Real mag0 = tangent_[0].mag(), mag1 = tangent_[1].mag();
		Angle angle0 = mag0==0?Angle::rad(0):tangent_[0].angle();
		tangent_[0]=Vector(-mag0, mag1==0?Angle::rad(0):tangent_[1].angle());
		tangent_[1]=Vector(-mag1, angle0);
		update_tangent2();
	}
}

void
synfig::BLinePoint::update_tangent2()
{
	if(approximate_zero(tangent_[0].mag_squared()))
		tangent2_radius_split_=tangent_[1];
	else
		tangent2_radius_split_=Vector(tangent_[1].mag(), tangent_[0].angle());
	tangent2_angle_split_=Vector(tangent_[0].mag(), tangent_[1].angle());
	return;
}

void
synfig::BLinePoint::update_flags()
{
	split_tangent_both_= split_tangent_radius_ && split_tangent_angle_;
	merge_tangent_both_= !split_tangent_radius_ && !split_tangent_angle_;
}
