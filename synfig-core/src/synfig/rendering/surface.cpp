/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/surface.cpp
**	\brief Surface
**
**	$Id$
**
**	\legal
**	......... ... 2015-2018 Ivan Mahonin
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

#include <cstring>

#include "surface.h"

#include "common/surfacememoryreadwrapper.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

synfig::Token Surface::token;
int SurfaceResource::last_id = 0;

Surface::Surface():
	blank(true),
	width(),
	height()
{ }

Surface::~Surface()
{ }

void
Surface::set_desc(int width, int height, bool blank)
{
	if (width > 0 && height > 0) {
		this->blank  = blank;
		this->width  = width;
		this->height = height;
	} else {
		this->blank  = true;
		this->width  = 0;
		this->height = 0;
	}
}

bool
Surface::get_pixels_vfunc(Color *dest) const
{
	const Color *src = get_pixels_pointer();
	if (!src) return false;
	memcpy(dest, src, get_buffer_size());
	return true;
}

bool
Surface::create(int width, int height)
{
	if (is_exists() && width == this->width && height == this->height && this->blank)
		return true;
	if (is_read_only() || width <= 0 || height <= 0)
		return false;
	if (is_exists())
		reset();
	if (!create_vfunc(width, height))
	{
		if (!reset()) assert(false);
		return false;
	}
	this->width = width;
	this->height = height;
	blank = true;
	return true;
}

bool
Surface::assign(const Surface &other)
{
	if (&other == this)
		return true;
	if (is_read_only())
		return false;
	if (!other.is_exists())
		return reset();
	if (other.is_blank())
		return create(other.get_width(), other.get_height());
	if (!assign_vfunc(other))
	{
		if (!reset()) assert(false);
		return false;
	}
	width = other.get_width();
	height = other.get_height();
	blank = false;
	return true;
}

bool
Surface::assign(const Color *pixels, int width, int height)
{
	if (pixels && pixels == get_pixels_pointer())
		return true;
	if (is_read_only() || !pixels)
		return false;
	return assign(SurfaceMemoryReadWrapper(pixels, width, height));
}

bool
Surface::clear()
{
	if (is_blank())
		return true;
	if (is_read_only())
		return false;
	if (!clear_vfunc())
	{
		int w = get_width();
		int h = get_height();
		reset();
		return create(w, h);
	}
	blank = true;
	return true;
}

bool
Surface::reset()
{
	if (!is_exists())
		return true;
	if (is_read_only())
		return false;
	if (!reset_vfunc())
		return false;
	width = 0;
	height = 0;
	blank = true;
	return true;
}

const Color*
Surface::get_pixels_pointer() const
{
	if (!is_exists())
		return NULL;
	return get_pixels_pointer_vfunc();
}

bool
Surface::get_pixels(Color *dest) const
{
	if (!is_exists() || !dest)
		return false;
	return get_pixels_vfunc(dest);
}

bool
Surface::touch()
{
	if (is_read_only() || !is_exists())
		return false;
	blank = false;
	return true;
}



SurfaceResource::SurfaceResource():
	id(++last_id),
	width(),
	height(),
	blank(true)
{ }

SurfaceResource::SurfaceResource(Surface::Handle surface):
	width(),
	height(),
	blank(true)
{ assign(surface); }

SurfaceResource::~SurfaceResource()
	{ reset(); }

Surface::Handle
SurfaceResource::get_surface(
	const Surface::Token::Handle &token,
	bool exclusive, // for write access
	bool full,
	const RectInt &rect,
	bool create,
	bool any )
{
	if (!token && !any)
		return Surface::Handle();
	if (!full && !rect.is_valid())
		return Surface::Handle();

	Glib::Threads::Mutex::Lock lock(mutex);

	if (width <= 0 || height <= 0)
		return Surface::Handle();
	if (!full && !etl::contains(RectInt(0, 0, width, height), rect))
		return Surface::Handle();

	Surface::Handle surface;

	Map::const_iterator i = surfaces.find(token);
	if (i != surfaces.end())
		surface = i->second;
	else
	if (any && !surfaces.empty())
		surface = surfaces.begin()->second;
	else
	if (!create)
		return Surface::Handle();

	if (!surface) {
		surface = token->fabric();
		if (!surface)
			return Surface::Handle();

		if (blank) {
			if (!surface->create(width, height))
				return Surface::Handle();
		} else {
			bool found = false;
			for(Map::const_iterator i = surfaces.begin(); i != surfaces.end() && !found; ++i)
				if (i->second->get_pixels_pointer() && surface->assign(*i->second))
					found = true;
			for(Map::const_iterator i = surfaces.begin(); i != surfaces.end() && !found; ++i)
				if (!i->second->get_pixels_pointer() && surface->assign(*i->second))
					found = true;
			if (!found)
				return Surface::Handle();
		}

		if (exclusive) surfaces.clear(); // all other surfaces invalidated
		surfaces[token] = surface;
	}

	if (exclusive) {
		if (surfaces.size() != 1) // keep only current surface in map
			{ surfaces.clear(); surfaces[token] = surface; }
		surface->touch();
		blank = false;
	}
	return surface;
}

void
SurfaceResource::create(int width, int height)
{
	Glib::Threads::RWLock::WriterLock lock(rwlock);
	Glib::Threads::Mutex::Lock short_lock(mutex);
	if (width > 0 && height > 0) {
		this->width  = width;
		this->height = height;
	} else {
		this->width  = 0;
		this->height = 0;
	}
	blank = true;
	surfaces.clear();
}

void
SurfaceResource::assign(Surface::Handle surface)
{
	Glib::Threads::RWLock::WriterLock lock(rwlock);
	Glib::Threads::Mutex::Lock short_lock(mutex);

	for(Map::const_iterator i = surfaces.begin(); i != surfaces.end(); ++i)
		if (i->second == surface)
			return;

	width = 0;
	height = 0;
	blank = true;
	surfaces.clear();
	if (!surface->is_exists())
		return;

	surfaces[surface->get_token()] = surface;
	width = surface->get_width();
	height = surface->get_height();
	blank = surface->is_blank();
}

void
SurfaceResource::clear()
{
	Glib::Threads::RWLock::WriterLock lock(rwlock);
	Glib::Threads::Mutex::Lock short_lock(mutex);
	blank = true;
	surfaces.clear();
}

void
SurfaceResource::reset()
{
	Glib::Threads::RWLock::WriterLock lock(rwlock);
	Glib::Threads::Mutex::Lock short_lock(mutex);
	width = 0;
	height = 0;
	blank = true;
	surfaces.clear();
}

/* === E N T R Y P O I N T ================================================= */
