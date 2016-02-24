/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/surfacesw.cpp
**	\brief SurfaceSW
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include <synfig/rendering/software/surfacesw.h>

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

SurfaceSW::SurfaceSW():
	own_surface(true), surface(new synfig::Surface())
{ }

SurfaceSW::SurfaceSW(const Surface &other):
	own_surface(true), surface(new synfig::Surface())
{
	assign(other);
}

SurfaceSW::~SurfaceSW()
{
	if (!own_surface) reset_surface();
	destroy();
}

bool
SurfaceSW::create_vfunc()
{
	surface->set_wh(get_width(), get_height());
	surface->clear();
	return true;
}

bool
SurfaceSW::assign_vfunc(const rendering::Surface &surface)
{
	this->surface->set_wh(get_width(), get_height());
	if (surface.get_pixels(&(*this->surface)[0][0]))
		return true;
	this->surface->set_wh(0, 0);
	return false;
}

void
SurfaceSW::destroy_vfunc()
{
	assert(surface);
	surface->set_wh(0, 0);
}

bool
SurfaceSW::get_pixels_vfunc(Color *buffer) const
{
	assert(surface);
	assert((int)surface->get_pitch() == (int)sizeof(Color)*get_width());
	memcpy(buffer, &(*this->surface)[0][0], get_buffer_size());
	return true;
}

void
SurfaceSW::set_surface(synfig::Surface &surface, bool own_surface)
{
	this->own_surface = own_surface;

	if (&surface == this->surface)
		return;

	unset_alternative();

	this->surface = &surface;
	assert(this->surface);
	mark_as_created(false);
	set_size(this->surface->get_w(), this->surface->get_h());
	mark_as_created(this->surface->get_w() > 0 && this->surface->get_h() > 0);
}

void
SurfaceSW::reset_surface()
{
	unset_alternative();
	if (!own_surface)
	{
		own_surface = true;
		surface = new synfig::Surface();
	}
	surface->set_wh(0, 0);
	mark_as_created(false);
}

/* === E N T R Y P O I N T ================================================= */
