/* === S Y N F I G ========================================================= */
/*!	\file dashitem.cpp
**	\brief Template File for a Dash Item implementation
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include "dashitem.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DashItem::DashItem():
offset_(0.1),
length_(0.1)
{
	side_type_[0] = side_type_[1] = TYPE_FLAT;
}


DashItem::DashItem(Real offset, Real length, int sidebefore, int sideafter):
	offset_(offset),
	length_(length)
{
	side_type_[0]=sidebefore;
	side_type_[1]=sideafter;
}

void
DashItem::set_length(Real x)
{
	length_=x;
}

const Real&
DashItem::get_length()const
{
	return length_;
}

void
DashItem::set_offset(Real x)
{
	offset_=x;
}

const Real&
DashItem::get_offset()const
{
	return offset_;
}

int
DashItem::get_side_type_before()const
{
	return side_type_[0];
}

void
DashItem::set_side_type_before(int sidebefore)
{
	side_type_[0]=sidebefore;
}

int
DashItem::get_side_type_after()const
{
	return side_type_[1];
}

void
DashItem::set_side_type_after(int sideafter)
{
	side_type_[1]=sideafter;
}

int
DashItem::get_side_type(int i)const
{
	return i>0? side_type_[1]: side_type_[0];
}

bool
DashItem::operator == (const DashItem& rhs) const
{
	return 	side_type_[0] == rhs.get_side_type(0) &&
			side_type_[1] == rhs.get_side_type(1) &&
			length_ == rhs.get_length() &&
			offset_ == rhs.get_offset();
}
