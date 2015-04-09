/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderer.cpp
**	\brief Renderer
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

#include "renderer.h"
#include "surface.h"
#include "transformation.h"
#include "blending.h"
#include "primitive.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

rendering::Renderer::DependentObject::Handle
rendering::Renderer::convert_obj(const DependentObject::Handle &obj)
{
	if (!obj) return NULL;

	// find converted
	if (obj->check_renderer_ptr(this)) return obj;
	for(DependentObject::AlternativeList::const_iterator i = obj->get_alternatives().begin(); i != obj->get_alternatives().end(); ++i)
		if ((*i)->check_renderer_ptr(this))
			return *i;

	// find supported
	for(DependentObject::AlternativeList::const_iterator i = obj->get_alternatives().begin(); i != obj->get_alternatives().end(); ++i)
		if (is_supported_vfunc(*i))
			{ (*i)->add_renderer(this); return *i; }

	// try to convert
	DependentObject::Handle converted = convert_vfunc(obj);
	if (converted)
		{ converted->add_renderer(this); return converted; }

	// TODO: warning cannot convert
	return NULL;
}

bool
rendering::Renderer::is_supported_vfunc(const DependentObject::Handle&) const
	{ return false; }

rendering::Renderer::DependentObject::Handle
rendering::Renderer::convert_vfunc(const DependentObject::Handle&)
	{ return NULL; }

bool
rendering::Renderer::draw_vfunc(
	const Params &,
	const etl::handle<Surface>&,
	const etl::handle<Transformation>&,
	const etl::handle<Blending>&,
	const etl::handle<Primitive>& )
{
	return false;
}

etl::handle<rendering::Surface>
rendering::Renderer::convert_surface(const etl::handle<Surface> &surface)
	{ return etl::handle<Surface>::cast_dynamic(convert_obj(surface)); }

etl::handle<rendering::Transformation>
rendering::Renderer::convert_transformation(const etl::handle<Transformation> &transformation)
	{ return etl::handle<Transformation>::cast_dynamic(convert_obj(transformation)); }

etl::handle<rendering::Blending>
rendering::Renderer::convert_blending(const etl::handle<Blending> &blending)
	{ return etl::handle<Blending>::cast_dynamic(convert_obj(blending)); }

etl::handle<rendering::Primitive>
rendering::Renderer::convert_primtive(const etl::handle<Primitive> &primitive)
	{ return etl::handle<Primitive>::cast_dynamic(convert_obj(primitive)); }

etl::handle<rendering::Surface>
rendering::Renderer::convert(const etl::handle<rendering::Surface> &surface)
	{ return convert_surface(surface); }

etl::handle<rendering::Transformation>
rendering::Renderer::convert(const etl::handle<rendering::Transformation> &transformation)
	{ return convert_transformation(transformation); }

etl::handle<rendering::Blending>
rendering::Renderer::convert(const etl::handle<rendering::Blending> &blending)
	{ return convert_blending(blending); }

etl::handle<rendering::Primitive>
rendering::Renderer::convert(const etl::handle<rendering::Primitive> &primitive)
	{ return convert_primtive(primitive); }


bool
rendering::Renderer::draw(
		const Params &params,
		const etl::handle<Surface> &target_surface,
		const etl::handle<Transformation> &transformation,
		const etl::handle<Blending> &blending,
		const etl::handle<Primitive> &primitive )
{
	if (!target_surface) return false; // TODO: warning
	if (target_surface->empty()) return false; // TODO: warning
	if (!blending) return false; // TODO: warning
	if (!primitive) return false; // TODO: warning

	Params sub_params = params;
	if (!sub_params.root_renderer)
		sub_params.root_renderer = this;
	if (sub_params.max_surface_width <= 0)
		sub_params.max_surface_width = INT_MAX;
	if (sub_params.max_surface_height <= 0)
		sub_params.max_surface_height = INT_MAX;

	int sw = sub_params.max_surface_width > target_surface->get_width()
		   ? sub_params.max_surface_width
		   : target_surface->get_width();
	int sh = sub_params.max_surface_height > target_surface->get_height()
		   ? sub_params.max_surface_height
		   : target_surface->get_height();

	if ( target_surface->get_width() != sw
	  || target_surface->get_height() != sh )
	{
		// TODO: warning surface too large, removed
		target_surface->assign(sw, sh);
	}

	etl::handle<Surface> converted_surface = convert(target_surface);
	etl::handle<Transformation> converted_transformation = convert(transformation);
	etl::handle<Blending> converted_blending = convert(blending);
	etl::handle<Primitive> converted_primitive = convert(primitive);

	if ( (bool)converted_surface        == (bool)target_surface
	  && (bool)converted_transformation == (bool)transformation
	  && (bool)converted_blending       == (bool)blending
	  && (bool)converted_primitive      == (bool)primitive
	  && draw_vfunc(sub_params, converted_surface, converted_transformation, converted_blending, converted_primitive) )
	{
		// TODO: warning surface conversion
		if (target_surface != converted_surface)
			target_surface->assign(converted_surface);
		return true;
	}

	if (get_alternative())
	{
		// TODO: warning call alternative
		return get_alternative()->draw(sub_params, target_surface, transformation, blending, primitive);
	}

	// TODO: warning no more alternative rendering to call
	return false;
}

/* === E N T R Y P O I N T ================================================= */
