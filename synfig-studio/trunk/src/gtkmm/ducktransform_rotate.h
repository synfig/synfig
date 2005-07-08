/* === S Y N F I G ========================================================= */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: ducktransform_rotate.h,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#ifndef __SYNFIG_STUDIO_DUCK_TRANSFORM_ROTATE_H
#define __SYNFIG_STUDIO_DUCK_TRANSFORM_ROTATE_H

/* === H E A D E R S ======================================================= */

#include "duckmatic.h"
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Transform_Rotate : public synfig::Transform
{
private:
	synfig::Angle angle;
	synfig::Vector origin;
	synfig::Real sin_val;
	synfig::Real cos_val;

public:
	Transform_Rotate(const synfig::Angle& angle,const synfig::Vector& origin=synfig::Vector(0,0)):
		angle(angle),
		origin(origin),
		sin_val(synfig::Angle::sin(angle).get()),
		cos_val(synfig::Angle::cos(angle).get())
	{
	}
	
	synfig::Vector perform(const synfig::Vector& x)const
	{
		synfig::Point pos(x-origin);
		return synfig::Point(cos_val*pos[0]-sin_val*pos[1],sin_val*pos[0]+cos_val*pos[1])+origin;
	}
	synfig::Vector unperform(const synfig::Vector& x)const
	{
		synfig::Point pos(x-origin);
		return synfig::Point(cos_val*pos[0]+sin_val*pos[1],-sin_val*pos[0]+cos_val*pos[1])+origin;
	}
};

};

/* === E N D =============================================================== */

#endif
