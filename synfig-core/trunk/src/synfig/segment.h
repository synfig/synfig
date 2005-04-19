/* === S I N F G =========================================================== */
/*!	\file segment.h
**	\brief Template Header
**
**	$Id: segment.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_SEGMENT_H
#define __SINFG_SEGMENT_H

/* === H E A D E R S ======================================================= */

#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
	
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

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
