/* === S Y N F I G ========================================================= */
/*!	\file rect.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "rect.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Rect
Rect::full_plane()
{
	const value_type infinity(HUGE_VAL);
	return Rect(-infinity, -infinity, infinity, infinity);
}

Rect
Rect::horizontal_strip(const value_type &y1, const value_type &y2)
{
	const value_type infinity(HUGE_VAL);
	return Rect(-infinity, y1, infinity, y2);
}

Rect
Rect::vertical_strip(const value_type &x1, const value_type &x2)
{
	const value_type infinity(HUGE_VAL);
	return Rect(x1, -infinity, x2, infinity);
}
