/* === S Y N F I G ========================================================= */
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

#ifndef __SYNFIG_STUDIO_DUCK_TRANSFORM_TRANSLATE_H
#define __SYNFIG_STUDIO_DUCK_TRANSFORM_TRANSLATE_H

/* === H E A D E R S ======================================================= */

#include "duckmatic.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {
	
class Transform_Translate : public synfig::Transform
{
private:
	synfig::Vector origin;
	std::vector<synfig::Vector> positions;

public:
	Transform_Translate(const synfig::Vector& origin): origin(origin) { }
	synfig::Vector perform(const synfig::Vector& x)const { return x+origin; }
	synfig::Vector unperform(const synfig::Vector& x)const { return x-origin; }
}; // END of class synfig::Transform_Translate

}; // END of namespace studio
/* === E N D =============================================================== */

#endif
