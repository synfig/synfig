/* === S I N F G =========================================================== */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: ducktransform_translate.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SINFG_STUDIO_DUCK_TRANSFORM_TRANSLATE_H
#define __SINFG_STUDIO_DUCK_TRANSFORM_TRANSLATE_H

/* === H E A D E R S ======================================================= */

#include "duckmatic.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class Transform_Translate : public sinfg::Transform
{
private:
	sinfg::Vector origin;
	std::vector<sinfg::Vector> positions;

public:
	Transform_Translate(const sinfg::Vector& origin): origin(origin) { }
	sinfg::Vector perform(const sinfg::Vector& x)const { return x+origin; }
	sinfg::Vector unperform(const sinfg::Vector& x)const { return x-origin; }
}; // END of class sinfg::Transform_Translate

}; // END of namespace studio
/* === E N D =============================================================== */

#endif
