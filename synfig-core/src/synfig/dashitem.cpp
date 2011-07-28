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

using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

DashItem::DashItem()
{
	set_offset(0.1);
	set_length(0.1);
	set_side_type_before(TYPE_FLAT);
	set_side_type_after(TYPE_FLAT);
}

DashItem::DashItem(const DashItem &ref) :
WidthPoint::WidthPoint(ref.get_offset(), ref.get_length(), ref.get_side_type_before(), ref.get_side_type_after())
{
}

DashItem::DashItem(Real offset, Real length, int sidebefore, int sideafter)
{
	set_offset(offset);
	set_width(length);
	set_side_type_before(sidebefore);
	set_side_type_after(sideafter);
}


void
DashItem::set_length(Real x)
{
	set_width(x);
}

const Real&
DashItem::get_length()const
{
	return get_width();
}

void
DashItem::set_offset(Real x)
{
	set_position(x);
}

const Real&
DashItem::get_offset()const
{
	return get_position();
}
