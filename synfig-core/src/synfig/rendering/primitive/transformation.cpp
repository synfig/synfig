/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/transformation.cpp
**	\brief Transformation
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

#include "transformation.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Transformation::TransformedPoint
Transformation::transform_vfunc(const Point &x) const
{
	return TransformedPoint(x);
}

Transformation::TransformedPoint
Transformation::back_transform_vfunc(const Point &x) const
{
	return TransformedPoint(x);
}

Mesh::Handle
Transformation::build_mesh_vfunc(const Rect &target_rect, const Vector &precision) const
{
	typedef std::pair<int, Mesh::Vertex> GridPoint;

	const Vector grid_p0 = target_rect.get_min();
	const Vector grid_p1 = target_rect.get_max();
	const Vector grid_size = grid_p1 - grid_p0;
	const int grid_side_count_x = std::max(1, (int)round(grid_size[0]/precision[0])) + 1;
	const int grid_side_count_y = std::max(1, (int)round(grid_size[1]/precision[1])) + 1;

	const Vector grid_step(
		grid_size[0]/(Real)(grid_side_count_x - 1),
		grid_size[1]/(Real)(grid_side_count_y - 1) );
	//const Real grid_step_diagonal = grid_step.mag();

	// build grid
	int visible_vertex_count = 0;
	std::vector<GridPoint> grid;
	grid.reserve(grid_side_count_x * grid_side_count_y);
	for(int j = 0; j < grid_side_count_y; ++j)
		for(int i = 0; i < grid_side_count_x; ++i)
		{
			Vector p( grid_p0[0] + i*grid_step[0],
					  grid_p0[1] + j*grid_step[1] );
			TransformedPoint tp = back_transform_vfunc(p);
			if (tp.visible) {
				grid.push_back(GridPoint(visible_vertex_count, Mesh::Vertex(p, tp.p)));
				++visible_vertex_count;
			} else {
				grid.push_back(GridPoint(-1, Mesh::Vertex()));
			}
		}

	if (visible_vertex_count == 0)
		return Mesh::Handle();

	// copy vertices to mesh
	Mesh::Handle mesh(new Mesh());
	mesh->vertices.reserve(visible_vertex_count);
	for(std::vector<GridPoint>::const_iterator i = grid.begin(); i != grid.end(); ++i)
		if (i->first >= 0) mesh->vertices.push_back(i->second);

	// build triangles
	mesh->triangles.reserve(visible_vertex_count * 2);
	for(int j = 0; j < grid_side_count_y; ++j)
	{
		for(int i = 0; i < grid_side_count_x; ++i)
		{
			int tl = grid[(j-1)*grid_side_count_x + i-1].first;
			int tr = grid[(j-1)*grid_side_count_x + i  ].first;
			int bl = grid[    j*grid_side_count_x + i-1].first;
			int br = grid[    j*grid_side_count_x + i  ].first;
			int mode = (tl >= 0 ? 1 : 0)
					 | (tr >= 0 ? 2 : 0)
					 | (bl >= 0 ? 4 : 0)
					 | (br >= 0 ? 8 : 0);
			switch(mode)
			{
			case 1|2|4|8:
				mesh->triangles.push_back(Mesh::Triangle(tl, tr, bl));
				mesh->triangles.push_back(Mesh::Triangle(bl, tr, br));
				break;
			case 2|4|8:
				mesh->triangles.push_back(Mesh::Triangle(bl, tr, br));
				break;
			case 1|4|8:
				mesh->triangles.push_back(Mesh::Triangle(tl, br, bl));
				break;
			case 1|2|8:
				mesh->triangles.push_back(Mesh::Triangle(tl, tr, br));
				break;
			case 1|2|4:
				mesh->triangles.push_back(Mesh::Triangle(tl, tr, bl));
				break;
			default:
				break;
			}
		}
	}

	return mesh;
}

/*
Point
Transformation::get_derivation_vfunc(int level, const Point &x, Real epsilon) const
{
	if (level <= 0) return transform_vfunc(x).p;
	return Point(
		( get_derivation_vfunc(level-1, Point(x[0] + epsilon, x[1]), epsilon)[0]
		- get_derivation_vfunc(level-1, Point(x[0] - epsilon, x[1]), epsilon)[0] ) / (2.0*epsilon),
		( get_derivation_vfunc(level-1, Point(x[0], x[1] + epsilon), epsilon)[1]
		- get_derivation_vfunc(level-1, Point(x[0], x[1] - epsilon), epsilon)[1] ) / (2.0*epsilon) );
}
*/

Transformation::TransformedPoint
Transformation::transform(const Point &x) const
	{ return transform_vfunc(x); }

Transformation::TransformedPoint
Transformation::back_transform(const Point &x) const
	{ return back_transform_vfunc(x); }


Mesh::Handle
Transformation::build_mesh(const Rect &target_rect, const Vector &precision) const
{
	const Real epsilon = 1e-6;

	if ( target_rect.get_min().is_nan_or_inf()
	  || target_rect.get_max().is_nan_or_inf()
	  || fabs(target_rect.maxx - target_rect.minx) < epsilon
	  || fabs(target_rect.maxy - target_rect.miny) < epsilon )
		return Mesh::Handle();

	Rect valid_target_rect(target_rect);
	if (valid_target_rect.maxx < valid_target_rect.minx)
		std::swap(valid_target_rect.maxx, valid_target_rect.minx);
	if (valid_target_rect.maxy < valid_target_rect.miny)
		std::swap(valid_target_rect.maxy, valid_target_rect.miny);

	Vector valid_precision(fabs(precision[0]), fabs(precision[1]));
	if (isnan(valid_precision[0]) || isinf(valid_precision[0]))
		valid_precision[0] = valid_target_rect.maxx - valid_target_rect.minx;
	if (isnan(valid_precision[1]) || isinf(valid_precision[1]))
		valid_precision[1] = valid_target_rect.maxx - valid_target_rect.minx;
	if (valid_precision[0] < epsilon)
		valid_precision[0] = epsilon;
	if (valid_precision[1] < epsilon)
		valid_precision[1] = epsilon;

	return build_mesh_vfunc(valid_target_rect, valid_precision);
}

Mesh::Handle
Transformation::build_mesh(const Point &target_rect_lt, const Point &target_rect_rb, const Vector &precision) const
{
	return build_mesh(Rect(target_rect_lt, target_rect_rb), precision);
}


/* === E N T R Y P O I N T ================================================= */
