/* === S Y N F I G ========================================================= */
/*!	\file halftone.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_HALFTONE_H
#define __SYNFIG_HALFTONE_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/angle.h>
#include <synfig/value.h>

/* === M A C R O S ========================================================= */

#define TYPE_SYMMETRIC		0
#define TYPE_DARKONLIGHT	1
#define TYPE_LIGHTONDARK	2
#define TYPE_DIAMOND		3
#define TYPE_STRIPE			4

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */
using namespace synfig;

class Halftone
{
public:
	//! Parameter: (int)
	ValueBase param_type;
	//! Parameter: (synfig::Point)
	ValueBase param_origin;
	//! Parameter: (synfig::Vector)
	ValueBase param_size;
	//! Parameter: (synfig::Angle)
	ValueBase param_angle;

	float mask(synfig::Point point)const;

	float operator()(const synfig::Point &point, const float& intensity, float supersample=0)const;
};

/* === E N D =============================================================== */

#endif
