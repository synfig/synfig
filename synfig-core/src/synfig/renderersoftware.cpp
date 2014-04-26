/* === S Y N F I G ========================================================= */
/*!	\file synfig/renderersoftware.cpp
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#include "renderersoftware.h"

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

Renderer::Result RendererSoftware::render_surface(const Params &/* params */, const Primitive<PrimitiveTypeSurface> &/* primitive */)
	{ return ResultNotSupported; }
Renderer::Result RendererSoftware::render_polygon(const Params &/* params */, const Primitive<PrimitiveTypePolygon> &/* primitive */)
	{ return ResultNotSupported; }
Renderer::Result RendererSoftware::render_colored_polygon(const Params &/* params */, const Primitive<PrimitiveTypeColoredPolygon> &/* primitive */)
	{ return ResultNotSupported; }
Renderer::Result RendererSoftware::render_mesh(const Params &/* params */, const Primitive<PrimitiveTypeMesh> &/* primitive */)
	{ return ResultNotSupported; }


struct RendererSoftware::Helper {
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

struct RendererSoftware::IntVector {
	union {
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
	inline long long get_fixed_x_div_y() { return y == 0 ? 0 : Helper::int_to_fixed(x)/y; }
};

void
RendererSoftware::render_triangle(
	synfig::Surface &target_surface,
	const Vector &p0,
	const Vector &t0,
	const Vector &p1,
	const Vector &t1,
	const Vector &p2,
	const Vector &t2,
	const synfig::Surface &texture,
	Real alpha,
	Color::BlendMethod blend_method )
{
	// convert points to int
	IntVector ip0(p0), ip1(p1), ip2(p2);
	if (ip0 == ip1 || ip0 == ip2 || ip1 == ip2) return;

	int width = target_surface.get_w();
	int height = target_surface.get_h();
	if (width == 0 || height == 0) return;

	int tex_width = texture.get_w();
	int tex_height = texture.get_h();
	if (tex_width == 0 || tex_height == 0) return;
	Vector tex_size = Vector(Real(tex_width), Real(tex_height));


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

	Surface::alpha_pen apen(target_surface.get_pen(0, 0));
	apen.set_alpha(alpha);
	apen.set_blend_method(blend_method);

    // sort points
    if (ip0.y > ip1.y) swap(ip0, ip1);
    if (ip0.y > ip2.y) swap(ip0, ip2);
    if (ip1.y > ip2.y) swap(ip1, ip2);

    // increments
    long long dx02 = (ip2-ip0).get_fixed_x_div_y();
    long long dx01 = (ip1-ip0).get_fixed_x_div_y();
    long long dx12 = (ip2-ip1).get_fixed_x_div_y();

    // work points
    // initially at top point (p0)
    long long wx0 = Helper::int_to_fixed(ip0.x);
    long long wx1 = wx0;

    // process top part of triangle

    // make copy of dx02
    long long dx02_copy = dx02;
    // sort increments
    if (dx01 < dx02) swap(dx02, dx01);
    // rasterize
    for (int y = ip0.y; y < ip1.y; ++y)
    {
		// draw horizontal line (this code has a copy below)
    	if (y >= 0 && y < height)
    	{
			int x0 = Helper::fixed_to_int(wx0);
			int x1 = Helper::fixed_to_int(wx1);
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
						apen.set_alpha(alpha);
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
		wx0 = Helper::int_to_fixed(ip0.x);
		wx1 = Helper::int_to_fixed(ip1.x);
		if (wx0 > wx1) swap(wx0, wx1);
    }

    // process bottom part of triangle

    // sort increments
    if (dx02_copy < dx12) swap(dx02_copy, dx12);

    // rasterize
    for (int y = ip1.y; y <= ip2.y; ++y){
		// draw horizontal line (this code has a copy above)
    	if (y >= 0 && y < height)
    	{
			int x0 = Helper::fixed_to_int(wx0);
			int x1 = Helper::fixed_to_int(wx1);
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
						apen.set_alpha(alpha);
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
RendererSoftware::render_mesh(
	synfig::Surface &target_surface,
	const synfig::Mesh &mesh,
	const synfig::Surface &texture,
	const Matrix &transform_matrix,
	const Matrix &texture_matrix,
	Real alpha,
	Color::BlendMethod blend_method )
{
	if (!target_surface.is_valid()) return;
	if (!texture.is_valid()) return;

	for(synfig::Mesh::TriangleList::const_iterator i = mesh.triangles.begin(); i != mesh.triangles.end(); ++i)
		render_triangle(
			target_surface,
			transform_matrix.get_transformed(mesh.vertices[i->vertices[0]].position),
			texture_matrix.get_transformed(mesh.vertices[i->vertices[0]].tex_coords),
			transform_matrix.get_transformed(mesh.vertices[i->vertices[1]].position),
			texture_matrix.get_transformed(mesh.vertices[i->vertices[1]].tex_coords),
			transform_matrix.get_transformed(mesh.vertices[i->vertices[2]].position),
			texture_matrix.get_transformed(mesh.vertices[i->vertices[2]].tex_coords),
			texture,
			alpha,
			blend_method );
}


/* === E N T R Y P O I N T ================================================= */
