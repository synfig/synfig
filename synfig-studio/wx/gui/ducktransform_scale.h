/* === S Y N F I G ========================================================= */
/*!	\file ducktransform_scale.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
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
	Transform_Scale(const synfig::GUID& guid, const synfig::Vector& scale,const synfig::Vector& origin=synfig::Vector(0,0)):
		Transform(guid), scale(scale), origin(origin) { }
	synfig::Vector perform(const synfig::Vector& x)const { return synfig::Vector((x[0]-origin[0])*scale[0]+origin[0],(x[1]-origin[1])*scale[1]+origin[1]); }
	synfig::Vector unperform(const synfig::Vector& x)const { return synfig::Vector((x[0]-origin[0])/scale[0]+origin[0],(x[1]-origin[1])/scale[1]+origin[1]); }

	synfig::String get_string()const
	{
		return "duck scale";
	}
};

};

/* === E N D =============================================================== */

#endif
