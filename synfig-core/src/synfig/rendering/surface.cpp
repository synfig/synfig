/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surface.cpp
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

#include "surface.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
rendering::Surface::assign(int width, int height)
{
	if (width <= 0 || height <= 0) { width = 0; height = 0; }
	assign_size_vfunc(width, height);
	changed();
}

void
rendering::Surface::assign(const Handle &surface)
{
	if (!surface || surface->empty())
	{
		if (empty()) return;
		assign_size_vfunc(0, 0);
	}
	else
	{
		assign_surface_vfunc(surface);
	}
	changed();
}

bool
rendering::Surface::empty() const
{
	int width = 0, height = 0;
	get_size_vfunc(width, height);
	return width <= 0 || height <= 0;
}

int
rendering::Surface::get_width() const
{
	int width = 0, height = 0;
	get_size_vfunc(width, height);
	return width <= 0 || height <= 0 ? 0 : width;
}

int
rendering::Surface::get_height() const
{
	int width = 0, height = 0;
	get_size_vfunc(width, height);
	return width <= 0 || height <= 0 ? 0 : height;
}

void
rendering::Surface::get_pixels(Color *buffer) const
	{ get_pixels_vfunc(buffer); }

/* === E N T R Y P O I N T ================================================= */
