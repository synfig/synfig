/* === S I N F G =========================================================== */
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

#ifndef __SINFG_STUDIO_DUCK_TRANSFORM_ROTATE_H
#define __SINFG_STUDIO_DUCK_TRANSFORM_ROTATE_H

/* === H E A D E R S ======================================================= */

#include "duckmatic.h"
#include <sinfg/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Transform_Rotate : public sinfg::Transform
{
private:
	sinfg::Angle angle;
	sinfg::Vector origin;
	sinfg::Real sin_val;
	sinfg::Real cos_val;

public:
	Transform_Rotate(const sinfg::Angle& angle,const sinfg::Vector& origin=sinfg::Vector(0,0)):
		angle(angle),
		origin(origin),
		sin_val(sinfg::Angle::sin(angle).get()),
		cos_val(sinfg::Angle::cos(angle).get())
	{
	}
	
	sinfg::Vector perform(const sinfg::Vector& x)const
	{
		sinfg::Point pos(x-origin);
		return sinfg::Point(cos_val*pos[0]-sin_val*pos[1],sin_val*pos[0]+cos_val*pos[1])+origin;
	}
	sinfg::Vector unperform(const sinfg::Vector& x)const
	{
		sinfg::Point pos(x-origin);
		return sinfg::Point(cos_val*pos[0]+sin_val*pos[1],-sin_val*pos[0]+cos_val*pos[1])+origin;
	}
};

};

/* === E N D =============================================================== */

#endif
