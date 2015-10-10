/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/surfacememoryreadwrapper.cpp
**	\brief SurfaceMemoryReadWrapper
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

#include <cstring>

#include "surfacememoryreadwrapper.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
SurfaceMemoryReadWrapper::create_vfunc()
	{ return false; }

bool
SurfaceMemoryReadWrapper::assign_vfunc(const Surface & /* surface */)
	{ return false; }

void
SurfaceMemoryReadWrapper::destroy_vfunc()
	{ buffer = NULL; }

bool
SurfaceMemoryReadWrapper::get_pixels_vfunc(Color *buffer) const
	{ memcpy(buffer, this->buffer, get_buffer_size()); return true; }

void
SurfaceMemoryReadWrapper::set_buffer(const Color *buffer)
{
	if (!empty() && this->buffer != buffer)
	{
		unset_alternative();
		this->buffer = buffer;
		mark_as_created(this->buffer);
	}
}

/* === E N T R Y P O I N T ================================================= */
