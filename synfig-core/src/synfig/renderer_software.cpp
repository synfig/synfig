/* === S Y N F I G ========================================================= */
/*!	\file synfig/renderer_software.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 IvanMahonin
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

#include "renderer_software.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer::RendererId RendererSoftware::id = 0;

Renderer::RendererId RendererSoftware::get_id() { return id; }

void RendererSoftware::initialize()
{
	if (id != 0) return;
	register_renderer(id);
	// TODO:
}

void RendererSoftware::deinitialize()
{
	unregister_renderer(id);
}

RendererSoftware::RendererSoftware()
{
	// TODO:
}

Renderer::Result RendererSoftware::render_surface(const Params &params, const Primitive<PrimitiveTypeSurface> &primitive)
	{ return ResultNotSupported; }
Renderer::Result RendererSoftware::render_polygon(const Params &params, const Primitive<PrimitiveTypePolygon> &primitive)
	{ return ResultNotSupported; }
Renderer::Result RendererSoftware::render_colored_polygon(const Params &params, const Primitive<PrimitiveTypeColoredPolygon> &primitive)
	{ return ResultNotSupported; }
Renderer::Result RendererSoftware::render_mesh(const Params &params, const Primitive<PrimitiveTypeMesh> &primitive)
	{ return ResultNotSupported; }

/* === E N T R Y P O I N T ================================================= */
