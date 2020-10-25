/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/surfacesw.cpp
**	\brief SurfaceSW
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include "surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


rendering::Surface::Token SurfaceSW::token(
	Desc<SurfaceSW>("SurfaceSW") );


SurfaceSW::SurfaceSW():
	own_surface(true),
	surface(new synfig::Surface())
{ }

SurfaceSW::SurfaceSW(synfig::Surface &surface, bool own_surface):
	own_surface(own_surface),
	surface(&surface)
{
	assert(this->surface);
	set_desc(this->surface->get_w(), this->surface->get_h(), false);
	assert((int)this->surface->get_pitch() == (int)sizeof(Color)*get_width());
}

SurfaceSW::~SurfaceSW()
{
	if (own_surface)
		{ assert(surface); delete surface; }
	surface = NULL;
	set_desc(0, 0, true);
}

bool
SurfaceSW::create_vfunc(int width, int height)
{
	assert(surface);
	surface->set_wh(width, height);
	surface->clear();
	return true;
}

bool
SurfaceSW::assign_vfunc(const rendering::Surface &surface)
{
	assert(this->surface);
	this->surface->set_wh(surface.get_width(), surface.get_height());
	if (surface.get_pixels(&(*this->surface)[0][0]))
		return true;
	this->surface->set_wh(0, 0);
	set_desc(0, 0, true);
	return false;
}

bool
SurfaceSW::clear_vfunc()
{
	assert(surface);
	surface->clear();
	return true;
}

bool
SurfaceSW::reset_vfunc()
{
	assert(surface);
	surface->set_wh(0, 0);
	return true;
}

const Color*
SurfaceSW::get_pixels_pointer_vfunc() const
{
	assert(surface);
	assert((int)surface->get_pitch() == (int)sizeof(Color)*get_width());
	return &(*this->surface)[0][0];
}

void
SurfaceSW::set_surface(synfig::Surface &surface, bool own_surface)
{
	if (&surface == this->surface) {
		this->own_surface = own_surface;
		return;
	}

	if (this->own_surface) {
		assert(this->surface);
		delete(this->surface);
	}

	this->surface = &surface;
	assert(this->surface);
	set_desc(surface.get_w(), surface.get_h(), false);
	assert((int)this->surface->get_pitch() == (int)sizeof(Color)*get_width());
}

void
SurfaceSW::reset_surface()
{
	if (own_surface) {
		assert(surface);
		delete(surface);
	}
	own_surface = true;
	surface = new synfig::Surface();
	set_desc(0, 0, true);
}

/* === E N T R Y P O I N T ================================================= */
