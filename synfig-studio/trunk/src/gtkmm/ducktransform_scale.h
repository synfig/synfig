/* === S Y N F I G ========================================================= */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: ducktransform_scale.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_DUCK_TRANSFORM_SCALE_H
#define __SYNFIG_STUDIO_DUCK_TRANSFORM_SCALE_H

/* === H E A D E R S ======================================================= */

#include "duckmatic.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Transform_Scale : public synfig::Transform
{
private:
	synfig::Vector scale;
	synfig::Vector origin;
public:
	Transform_Scale(const synfig::Vector& scale,const synfig::Vector& origin=synfig::Vector(0,0)):scale(scale),origin(origin) { }
	synfig::Vector perform(const synfig::Vector& x)const { return synfig::Vector((x[0]-origin[0])*scale[0]+origin[0],(x[1]-origin[1])*scale[1]+origin[1]); }
	synfig::Vector unperform(const synfig::Vector& x)const { return synfig::Vector((x[0]-origin[0])/scale[0]+origin[0],(x[1]-origin[1])/scale[1]+origin[1]); }
};

};

/* === E N D =============================================================== */

#endif
