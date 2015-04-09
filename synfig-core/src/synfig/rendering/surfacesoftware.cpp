/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surfacesoftware.cpp
**	\brief Surface
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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "surfacesoftware.h"
#include "renderer.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
rendering::SurfaceSoftware::assign_size_vfunc(int width, int height)
{
	set_wh(width, height);
}

void
rendering::SurfaceSoftware::assign_surface_vfunc(const rendering::Surface::Handle &surface)
{
	assert(get_pitch() == (int)(sizeof(Color)*get_w()));
	set_wh(surface->get_width(), surface->get_height(), 0);
	surface->get_pixels(&(*this)[0][0]);
}

void
rendering::SurfaceSoftware::get_size_vfunc(int &out_width, int &out_height) const
	{ out_width = get_w(); out_height = get_h(); }

void
rendering::SurfaceSoftware::get_pixels_vfunc(Color *buffer) const
{
	assert(get_pitch() == sizeof(Color)*get_width());
	memcpy(buffer, &(*this)[0][0], sizeof(Color)*get_w()*get_h());
}

/* === E N T R Y P O I N T ================================================= */
