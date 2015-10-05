/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/task/tasksurfaceresamplesw.cpp
**	\brief TaskSurfaceResampleSW
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

#include <synfig/general.h>

#include "tasksurfaceresamplesw.h"

#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

bool
TaskSurfaceResampleSW::run(RunParams & /* params */) const
{
	const synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic(sub_task()->target_surface)->get_surface();
	synfig::Surface &target =
		SurfaceSW::Handle::cast_dynamic(target_surface)->get_surface();

	Matrix bounds_transfromation;
	bounds_transfromation.m00 = get_pixels_per_unit()[0];
	bounds_transfromation.m11 = get_pixels_per_unit()[1];
	bounds_transfromation.m20 = -rect_lt[0] * bounds_transfromation.m00;
	bounds_transfromation.m21 = -rect_lt[1] * bounds_transfromation.m11;

	Matrix matrix = transformation * bounds_transfromation;

	Rect boundsf(   matrix.get_transformed(Vector(0.0, 0.0)) );
	boundsf.expand( matrix.get_transformed(Vector(1.0, 0.0)) );
	boundsf.expand( matrix.get_transformed(Vector(0.0, 1.0)) );
	boundsf.expand( matrix.get_transformed(Vector(1.0, 1.0)) );

	RectInt bounds( (int)floor(boundsf.minx) - 1,
			        (int)floor(boundsf.miny) - 1,
					(int)floor(boundsf.maxx) + 2,
					(int)floor(boundsf.maxy) + 2 );
	etl::set_intersect(bounds, bounds, RectInt(0, 0, target.get_w(), target.get_h()));

	Matrix a_matrix;
	a_matrix.m00 = (crop_rb[0] - crop_lt[0])*a.get_w();
	a_matrix.m11 = (crop_rb[1] - crop_lt[1])*a.get_h();
	a_matrix.m20 = -crop_lt[0] * a_matrix.m00;
	a_matrix.m21 = -crop_lt[1] * a_matrix.m11;

	Matrix inv_matrix = matrix;
	inv_matrix.invert();

	Matrix sub_matrix = inv_matrix * a_matrix;

	// TODO: gamma, interpolation

	Vector aa_offset( 0.5*a_matrix.m00, 0.5*a_matrix.m11 );
	Vector aa_scale( 2.0/(a_matrix.m00*inv_matrix.get_axis_x().mag()),
					  2.0/(a_matrix.m11*inv_matrix.get_axis_y().mag()) );
	synfig::Surface::alpha_pen p(target.get_pen(0, 0));
	p.set_blend_method(blend ? blend_method : Color::BLEND_COMPOSITE);
	for(int y = bounds.miny; y < bounds.maxy; ++y)
	{
		for(int x = bounds.minx; x < bounds.maxx; ++x)
		{
			Vector pos = sub_matrix.get_transformed(Vector((Real)x, (Real)y));
			Real aa = std::max(std::min( (aa_offset[0] - fabs(aa_offset[0] - pos[0]))*aa_scale[0] + 0.5, 1.0), 0.0)
					* std::max(std::min( (aa_offset[1] - fabs(aa_offset[1] - pos[1]))*aa_scale[1] + 0.5, 1.0), 0.0);
			p.move_to(x, y);
			p.put_value(a.linear_sample(pos[0] - 0.5, pos[1] - 0.5), aa);
		}
	}

	return true;
}

/* === E N T R Y P O I N T ================================================= */
