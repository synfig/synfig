/* === S Y N F I G ========================================================= */
/*!	\file ducktransform_rotate.h
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

#ifndef __SYNFIG_STUDIO_DUCK_TRANSFORM_ROTATE_H
#define __SYNFIG_STUDIO_DUCK_TRANSFORM_ROTATE_H

/* === H E A D E R S ======================================================= */

#include <gui/duckmatic.h>
#include <synfig/angle.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class Transform_Rotate : public synfig::Transform
{
private:
	//synfig::Angle angle;
	synfig::Vector origin;
	synfig::Real sin_val;
	synfig::Real cos_val;

public:
	Transform_Rotate(const synfig::GUID& guid, const synfig::Angle& angle,const synfig::Vector& origin=synfig::Vector(0,0)):
		Transform(guid),
		//angle(angle),
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

	synfig::String get_string()const
	{
		return "duck rotate";
	}
};

};

/* === E N D =============================================================== */

#endif
