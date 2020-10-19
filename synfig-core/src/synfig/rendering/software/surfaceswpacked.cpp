/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/surfaceswpacked.cpp
**	\brief SurfaceSWPacked
**
**	$Id$
**
**	\legal
**	......... ... 2016-2018 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "surfaceswpacked.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


rendering::Surface::Token SurfaceSWPacked::token(
	Desc<SurfaceSWPacked>("SurfaceSWPacked") );


bool
SurfaceSWPacked::assign_vfunc(const rendering::Surface &surface)
{
	std::vector<Color> data;
	const Color *pixels = surface.get_pixels_pointer();
	if (!pixels) {
		data.resize(get_pixels_count());
		if (!surface.get_pixels(&data.front()))
			return false;
		pixels = &data.front();
	}
	this->surface.set_pixels(pixels, surface.get_width(), surface.get_height());
	return true;
}

bool
SurfaceSWPacked::reset_vfunc()
{
	surface.clear();
	return true;
}

bool
SurfaceSWPacked::get_pixels_vfunc(Color *buffer) const
{
	surface.get_pixels(buffer);
	return true;
}

/* === E N T R Y P O I N T ================================================= */
