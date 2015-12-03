/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskmeshsw.cpp
**	\brief TaskMeshSW
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

#include "taskmeshsw.h"

#include "../surfacesw.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class TaskMeshSW::Internal
{
public:
	struct IntVector
	{
		union
		{
			struct { int x, y; };
			int coords[2];
		};

		inline IntVector(): x(0), y(0) { }
		inline IntVector(int x, int y): x(x), y(y) { }
		inline IntVector(const Vector &v): x((int)roundf(v[0])), y((int)roundf(v[1])) { }
		inline int& operator[] (int index) { return coords[index]; }
		inline const int& operator[] (int index) const { return coords[index]; }
		inline bool operator == (const IntVector &other) const { return x == other.x && y == other.y; }
		inline bool operator != (const IntVector &other) const { return !(*this == other); }
		inline IntVector operator+ (const IntVector &other) const { return IntVector(x+other.x, y+other.y); }
		inline IntVector operator- (const IntVector &other) const { return IntVector(x-other.x, y-other.y); }

		inline Vector to_real() const { return Vector(Real(x), Real(y)); }
		inline long long get_fixed_x_div_y() { return y == 0 ? 0 : int_to_fixed(x)/y; }
	};

	enum { FIXED_SHIFT = sizeof(int)*8 };

	inline static long long int_to_fixed(int i)
		{ return (long long)i << FIXED_SHIFT; }
	inline static int fixed_to_int(long long f)
		{ return (int)(f >> FIXED_SHIFT); }

	inline static void norm_tex_coords(Vector &coords, const Vector &size)
	{
		if (coords[0] < 0.0 || coords[0] > size[0])
			coords[0] -= floor(coords[0]/size[0])*size[0];
		if (coords[1] < 0.0 || coords[1] > size[1])
			coords[1] -= floor(coords[1]/size[1])*size[1];
	}
};

void
TaskMeshSW::render_triangle(
	synfig::Surface &target_surface,
	const Vector &p0,
	const Vector &p1,
	const Vector &p2,
	const Color &color,
	Color::value_type opacity,
	Color::BlendMethod blend_method )
{
	// convert points to int
	Internal::IntVector ip0(p0), ip1(p1), ip2(p2);
	if (ip0 == ip1 || ip0 == ip2 || ip1 == ip2) return;

	if (ip0.x < 0 && ip1.x < 0 && ip2.x < 0) return;
	if (ip0.y < 0 && ip1.y < 0 && ip2.y < 0) return;

	int width = target_surface.get_w();
	int height = target_surface.get_h();
	if (width == 0 || height == 0) return;

	if (ip0.x >= width && ip1.x >= width && ip2.x >= width) return;
	if (ip0.y >= height && ip1.y >= height && ip2.y >= height) return;

	synfig::Surface::alpha_pen apen(target_surface.get_pen(0, 0));
	apen.set_alpha(opacity);
	apen.set_blend_method(blend_method);

    // sort points
    if (ip0.y > ip1.y) std::swap(ip0, ip1);
    if (ip0.y > ip2.y) std::swap(ip0, ip2);
    if (ip1.y > ip2.y) std::swap(ip1, ip2);

    // increments
    long long dx02 = (ip2-ip0).get_fixed_x_div_y();
    long long dx01 = (ip1-ip0).get_fixed_x_div_y();
    long long dx12 = (ip2-ip1).get_fixed_x_div_y();

    // work points
    // initially at top point (p0)
    long long wx0 = Internal::int_to_fixed(ip0.x);
    long long wx1 = wx0;

    // process top part of triangle

    // make copy of dx02
    long long dx02_copy = dx02;
    // sort increments
    if (dx01 < dx02) std::swap(dx02, dx01);
    // rasterize
    for (int y = ip0.y; y < ip1.y; ++y)
    {
		// draw horizontal line (this code has a copy below)
    	if (y >= 0 && y < height)
    	{
			int x0 = Internal::fixed_to_int(wx0);
			int x1 = Internal::fixed_to_int(wx1);
			if (x0 < 0) x0 = 0;
			if (x1 >= width) x1 = width-1;
			if (x1 >= x0)
			{
				apen.move_to(x0, y);
				for(int x = x0; x <= x1; ++x)
				{
					apen.put_value(color);
					apen.inc_x();
				}
			}
    	}

		wx0 += dx02;
		wx1 += dx01;
    }

    if (ip0.y == ip1.y) {
		wx0 = Internal::int_to_fixed(ip0.x);
		wx1 = Internal::int_to_fixed(ip1.x);
		if (wx0 > wx1) std::swap(wx0, wx1);
    }

    // process bottom part of triangle

    // sort increments
    if (dx02_copy < dx12) std::swap(dx02_copy, dx12);

    // rasterize
    for (int y = ip1.y; y <= ip2.y; ++y){
		// draw horizontal line (this code has a copy above)
    	if (y >= 0 && y < height)
    	{
			int x0 = Internal::fixed_to_int(wx0);
			int x1 = Internal::fixed_to_int(wx1);
			if (x0 < 0) x0 = 0;
			if (x1 >= width) x1 = width-1;
			if (x1 >= x0)
			{
				apen.move_to(x0, y);
				for(int x = x0; x <= x1; ++x)
				{
					apen.put_value(color);
					apen.inc_x();
				}
			}
    	}

		wx0 += dx02_copy;
		wx1 += dx12;
    }
}

void
TaskMeshSW::render_triangle(
	synfig::Surface &target_surface,
	const Vector &p0,
	const Vector &t0,
	const Vector &p1,
	const Vector &t1,
	const Vector &p2,
	const Vector &t2,
	const synfig::Surface &texture,
	Color::value_type opacity,
	Color::BlendMethod blend_method )
{
	if (t0[0] < 0.0 && t1[0] < 0.0 && t2[0] < 0.0) return;
	if (t0[1] < 0.0 && t1[1] < 0.0 && t2[1] < 0.0) return;

	// convert points to int
	Internal::IntVector ip0(p0), ip1(p1), ip2(p2);
	if (ip0 == ip1 || ip0 == ip2 || ip1 == ip2) return;

	if (ip0.x < 0 && ip1.x < 0 && ip2.x < 0) return;
	if (ip0.y < 0 && ip1.y < 0 && ip2.y < 0) return;

	int width = target_surface.get_w();
	int height = target_surface.get_h();
	if (width == 0 || height == 0) return;

	if (ip0.x >= width && ip1.x >= width && ip2.x >= width) return;
	if (ip0.y >= height && ip1.y >= height && ip2.y >= height) return;

	int tex_width = texture.get_w();
	int tex_height = texture.get_h();
	if (tex_width == 0 || tex_height == 0) return;
	Vector tex_size = Vector(Real(tex_width), Real(tex_height));

	if (t0[0] > tex_size[0] && t1[0] > tex_size[0] && t2[0] > tex_size[0]) return;
	if (t0[1] > tex_size[1] && t1[1] > tex_size[1] && t2[1] > tex_size[1]) return;

	// prepare texture matrix
	Matrix matrix_of_texture_triangle(
		t1[0]-t0[0], t1[1]-t0[1], 0.0,
		t2[0]-t0[0], t2[1]-t0[1], 0.0,
		t0[0], t0[1], 1.0 );
	Matrix matrix_of_target_triangle(
		p1[0]-p0[0], p1[1]-p0[1], 0.0,
		p2[0]-p0[0], p2[1]-p0[1], 0.0,
		p0[0], p0[1], 1.0 );
	matrix_of_target_triangle.invert();

	Matrix matrix = matrix_of_target_triangle * matrix_of_texture_triangle;
	Vector tdx = matrix.get_transformed(Vector(1.0, 0.0), false);
	//Vector tdy = matrix.get_transformed(Vector(0.0, 1.0), false);

	synfig::Surface::alpha_pen apen(target_surface.get_pen(0, 0));
	apen.set_alpha(opacity);
	apen.set_blend_method(blend_method);

    // sort points
    if (ip0.y > ip1.y) std::swap(ip0, ip1);
    if (ip0.y > ip2.y) std::swap(ip0, ip2);
    if (ip1.y > ip2.y) std::swap(ip1, ip2);

    // increments
    long long dx02 = (ip2-ip0).get_fixed_x_div_y();
    long long dx01 = (ip1-ip0).get_fixed_x_div_y();
    long long dx12 = (ip2-ip1).get_fixed_x_div_y();

    // work points
    // initially at top point (p0)
    long long wx0 = Internal::int_to_fixed(ip0.x);
    long long wx1 = wx0;

    // process top part of triangle

    // make copy of dx02
    long long dx02_copy = dx02;
    // sort increments
    if (dx01 < dx02) std::swap(dx02, dx01);
    // rasterize
    for (int y = ip0.y; y < ip1.y; ++y)
    {
		// draw horizontal line (this code has a copy below)
    	if (y >= 0 && y < height)
    	{
			int x0 = Internal::fixed_to_int(wx0);
			int x1 = Internal::fixed_to_int(wx1);
			if (x0 < 0) x0 = 0;
			if (x1 >= width) x1 = width-1;
			if (x1 >= x0)
			{
				apen.move_to(x0, y);
				Vector tex_point = matrix.get_transformed(Vector(Real(x0), Real(y)));
				for(int x = x0; x <= x1; ++x)
				{
					if (tex_point[0] < 0.0 || tex_point[0] > tex_size[0]
					 || tex_point[1] < 0.0 || tex_point[1] > tex_size[1])
					{
						apen.set_alpha(0.0);
						apen.put_value(Color());
					}
					else
					{
						apen.set_alpha(opacity);
						apen.put_value(texture.cubic_sample(tex_point[0], tex_point[1]));
					}
					apen.inc_x();
					tex_point += tdx;
				}
			}
    	}

		wx0 += dx02;
		wx1 += dx01;
    }

    if (ip0.y == ip1.y) {
		wx0 = Internal::int_to_fixed(ip0.x);
		wx1 = Internal::int_to_fixed(ip1.x);
		if (wx0 > wx1) std::swap(wx0, wx1);
    }

    // process bottom part of triangle

    // sort increments
    if (dx02_copy < dx12) std::swap(dx02_copy, dx12);

    // rasterize
    for (int y = ip1.y; y <= ip2.y; ++y){
		// draw horizontal line (this code has a copy above)
    	if (y >= 0 && y < height)
    	{
			int x0 = Internal::fixed_to_int(wx0);
			int x1 = Internal::fixed_to_int(wx1);
			if (x0 < 0) x0 = 0;
			if (x1 >= width) x1 = width-1;
			if (x1 >= x0)
			{
				apen.move_to(x0, y);
				Vector tex_point = matrix.get_transformed(Vector(Real(x0), Real(y)));
				for(int x = x0; x <= x1; ++x)
				{
					if (tex_point[0] < 0.0 || tex_point[0] > tex_size[0]
					 || tex_point[1] < 0.0 || tex_point[1] > tex_size[1])
					{
						apen.set_alpha(0.0);
						apen.put_value(Color());
					}
					else
					{
						apen.set_alpha(opacity);
						apen.put_value(texture.cubic_sample(tex_point[0], tex_point[1]));
					}
					apen.inc_x();
					tex_point += tdx;
				}
			}
    	}

		wx0 += dx02_copy;
		wx1 += dx12;
    }
}

void
TaskMeshSW::render_polygon(
	synfig::Surface &target_surface,
	const Vector *vertices,
	int vertices_strip,
	const int *triangles,
	int triangles_strip,
	int triangles_count,
	const Matrix &transform_matrix,
	const Color &color,
	Color::value_type opacity,
	Color::BlendMethod blend_method )
{
	if (!target_surface.is_valid()) return;

	if (vertices_strip <= 0) vertices_strip = sizeof(Vector);
	if (triangles_strip <= 0) triangles_strip = sizeof(int[3]);

	for(int i = 0; i < triangles_count; ++i)
	{
		int *triangle = (int*)((char*)triangles + i*triangles_strip);
		render_triangle(
			target_surface,
			transform_matrix.get_transformed(*(Vector*)((char*)vertices + triangle[0]*vertices_strip)),
			transform_matrix.get_transformed(*(Vector*)((char*)vertices + triangle[1]*vertices_strip)),
			transform_matrix.get_transformed(*(Vector*)((char*)vertices + triangle[2]*vertices_strip)),
			color,
			opacity,
			blend_method );
	}
}

void
TaskMeshSW::render_mesh(
	synfig::Surface &target_surface,
	const Vector *vertices,
	int vertices_strip,
	const Vector *tex_coords,
	int tex_coords_strip,
	const int *triangles,
	int triangles_strip,
	int triangles_count,
	const synfig::Surface &texture,
	const Matrix &transform_matrix,
	const Matrix &texture_matrix,
	Color::value_type opacity,
	Color::BlendMethod blend_method )
{
	if (!target_surface.is_valid()) return;
	if (!texture.is_valid()) return;

	if (vertices_strip <= 0) vertices_strip = sizeof(Vector);
	if (tex_coords_strip <= 0) tex_coords_strip = sizeof(Vector);
	if (triangles_strip <= 0) triangles_strip = sizeof(int[3]);

	for(int i = 0; i < triangles_count; ++i)
	{
		int *triangle = (int*)((char*)triangles + i*triangles_strip);
		render_triangle(
			target_surface,
			transform_matrix.get_transformed(*(Vector*)((char*)vertices + triangle[0]*vertices_strip)),
			texture_matrix.get_transformed(*(Vector*)((char*)tex_coords + triangle[0]*tex_coords_strip)),
			transform_matrix.get_transformed(*(Vector*)((char*)vertices + triangle[1]*vertices_strip)),
			texture_matrix.get_transformed(*(Vector*)((char*)tex_coords + triangle[1]*tex_coords_strip)),
			transform_matrix.get_transformed(*(Vector*)((char*)vertices + triangle[2]*vertices_strip)),
			texture_matrix.get_transformed(*(Vector*)((char*)tex_coords + triangle[2]*tex_coords_strip)),
			texture,
			opacity,
			blend_method );
	}
}


bool
TaskMeshSW::run(RunParams & /* params */) const
{
	synfig::Surface &a =
		SurfaceSW::Handle::cast_dynamic( target_surface )->get_surface();
	const synfig::Surface &b =
		SurfaceSW::Handle::cast_dynamic( sub_task()->target_surface )->get_surface();

	// TODO: target_rect

	Matrix3 transfromation_matrix;
	transfromation_matrix.m00 = get_units_per_pixel()[0];
	transfromation_matrix.m11 = get_units_per_pixel()[1];
	transfromation_matrix.m20 = get_source_rect_lt()[0];
	transfromation_matrix.m21 = get_source_rect_lt()[1];

	Matrix3 texture_transfromation_matrix;
	texture_transfromation_matrix.m00 = sub_task()->get_units_per_pixel()[0];
	texture_transfromation_matrix.m11 = sub_task()->get_units_per_pixel()[1];
	texture_transfromation_matrix.m20 = sub_task()->get_source_rect_lt()[0];
	texture_transfromation_matrix.m21 = sub_task()->get_source_rect_lt()[1];

	render_mesh(
		a,
		&mesh->vertices.front().position,
		sizeof(mesh->vertices.front()),
		&mesh->vertices.front().tex_coords,
		sizeof(mesh->vertices.front()),
		mesh->triangles.front().vertices,
		sizeof(mesh->triangles.front()),
		mesh->triangles.size(),
		b,
		transfromation_matrix,
		texture_transfromation_matrix,
		1.0,
		Color::BLEND_COMPOSITE );

	return true;
}

/* === E N T R Y P O I N T ================================================= */
