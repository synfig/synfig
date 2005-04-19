/* === S Y N F I G ========================================================= */
/*!	\file halftone.h
**	\brief Template Header
**
**	$Id: halftone.h,v 1.1.1.1 2005/01/04 01:23:10 darco Exp $
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

#ifndef __SYNFIG_HALFTONE_H
#define __SYNFIG_HALFTONE_H

/* === H E A D E R S ======================================================= */

#include <synfig/vector.h>
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

#define TYPE_SYMMETRIC		0
#define TYPE_DARKONLIGHT	1
#define TYPE_LIGHTONDARK	2
#define TYPE_DIAMOND		3
#define TYPE_STRIPE			4

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

class Halftone
{
public:

	int type;
	synfig::Point offset;
	synfig::Vector size;
	synfig::Angle angle;

	float mask(synfig::Point point)const;
	
	float operator()(const synfig::Point &point, const float& intensity, float supersample=0)const;
};

/* === E N D =============================================================== */

#endif
