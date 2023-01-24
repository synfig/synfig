/* === S Y N F I G ========================================================= */
/*!	\file segment.h
**	\brief Template Header
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_SEGMENT_H
#define __SYNFIG_SEGMENT_H

/* === H E A D E R S ======================================================= */

#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\struct Segment
**	\todo writeme
*/
struct Segment
{
	Point	p1,p2;
	Vector	t1,t2;

	Segment() { }
	Segment(Point p1,Vector t1,Point p2, Vector t2):
		p1(p1),
		p2(p2),
		t1(t1),
		t2(t2)
	{ }
	Segment(Point p1,Point p2):
		p1(p1),
		p2(p2),
		t1(p2-p1),
		t2(p2-p1)
	{ }
}; // END of struct Segment

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
