/* === S Y N F I G ========================================================= */
/*!	\file ducktransform_translate.h
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

#ifndef __SYNFIG_STUDIO_DUCK_TRANSFORM_TRANSLATE_H
#define __SYNFIG_STUDIO_DUCK_TRANSFORM_TRANSLATE_H

/* === H E A D E R S ======================================================= */

#include <gui/duckmatic.h>

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
	Transform_Translate(const synfig::GUID& guid, const synfig::Vector& origin):Transform(guid), origin(origin) { }
	synfig::Vector perform(const synfig::Vector& x)const { return x+origin; }
	synfig::Vector unperform(const synfig::Vector& x)const { return x-origin; }

	synfig::String get_string()const
	{
		return "duck translate";
	}
}; // END of class synfig::Transform_Translate

}; // END of namespace studio
/* === E N D =============================================================== */

#endif
