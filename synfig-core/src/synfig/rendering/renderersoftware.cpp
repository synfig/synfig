/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderersoftware.cpp
**	\brief RendererSoftware
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
#include "surfacesoftware.h"
#include "transformationaffine.h"
#include "blendingsimple.h"
#include "mesh.h"
#include "blur.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

class rendering::RendererSoftware::Internal
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
rendering::RendererSoftware::render_triangle(
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
    if (ip0.y > ip1.y) swap(ip0, ip1);
    if (ip0.y > ip2.y) swap(ip0, ip2);
    if (ip1.y > ip2.y) swap(ip1, ip2);

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
    if (dx01 < dx02) swap(dx02, dx01);
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
rendering::RendererSoftware::render_triangle(
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
    if (ip0.y > ip1.y) swap(ip0, ip1);
    if (ip0.y > ip2.y) swap(ip0, ip2);
    if (ip1.y > ip2.y) swap(ip1, ip2);

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
    if (dx01 < dx02) swap(dx02, dx01);
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
rendering::RendererSoftware::render_polygon(
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
rendering::RendererSoftware::render_mesh(
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

void
rendering::RendererSoftware::render_polyspan(
	synfig::Surface &target_surface,
	const Polyspan &polyspan,
	bool invert,
	bool antialias,
	Polyspan::WindingStyle winding_style,
	const Color &color,
	Color::value_type opacity,
	Color::BlendMethod blend_method )
{
	synfig::Surface::alpha_pen p(target_surface.begin(), opacity, blend_method);
	const Polyspan::ContextRect &window = polyspan.get_window();
	const Polyspan::cover_array &covers = polyspan.get_covers();

	Polyspan::cover_array::iterator cur_mark = covers.begin();
	Polyspan::cover_array::iterator end_mark = covers.end();

	Real cover = 0, area = 0, alpha = 0;
	int	y = 0, x = 0;

	p.set_value(color);
	cover = 0;

	if (cur_mark == end_mark)
	{
		// no marks at all
		if (invert)
		{
			p.move_to(window.minx, window.miny);
			p.put_block(window.maxy - window.miny, window.maxx - window.minx);
		}
		return;
	}

	// fill initial rect / line
	if (invert)
	{
		// fill all the area above the first vertex
		p.move_to(window.minx, window.miny);
		y = window.miny;
		int l = window.maxx - window.minx;

		p.put_block(cur_mark->y - window.miny, l);

		// fill the area to the left of the first vertex on that line
		l = cur_mark->x - window.minx;
		p.move_to(window.minx, cur_mark->y);
		if (l) p.put_hline(l);
	}

	while(true)
	{
		y = cur_mark->y;
		x = cur_mark->x;

		p.move_to(x,y);

		area = cur_mark->area;
		cover += cur_mark->cover;

		// accumulate for the current pixel
		while(++cur_mark != covers.end())
		{
			if (y != cur_mark->y || x != cur_mark->x)
				break;

			area += cur_mark->area;
			cover += cur_mark->cover;
		}

		// draw pixel - based on covered area
		if (area) // if we're ok, draw the current pixel
		{
			alpha = polyspan.extract_alpha(cover - area, winding_style);
			if (invert) alpha = 1 - alpha;

			if (antialias)
			{
				if (alpha) p.put_value_alpha(alpha);
			}
			else
			{
				if (alpha >= .5) p.put_value();
			}

			p.inc_x();
			x++;
		}

		// if we're done, don't use iterator and exit
		if (cur_mark == end_mark) break;

		// if there is no more live pixels on this line, goto next
		if (y != cur_mark->y)
		{
			if (invert)
			{
				// fill the area at the end of the line
				p.put_hline(window.maxx - x);

				// fill area at the beginning of the next line
				p.move_to(window.minx, cur_mark->y);
				p.put_hline(cur_mark->x - window.minx);
			}

			cover = 0;
			continue;
		}

		// draw span to next pixel - based on total amount of pixel cover
		if (x < cur_mark->x)
		{
			alpha = polyspan.extract_alpha(cover, winding_style);
			if (invert) alpha = 1 - alpha;

			if (antialias)
			{
				if (alpha) p.put_hline(cur_mark->x - x, alpha);
			}
			else
			{
				if (alpha >= .5) p.put_hline(cur_mark->x - x);
			}
		}
	}

	// fill the after stuff
	if (invert)
	{
		//fill the area at the end of the line
		p.put_hline(window.maxx - x);

		//fill area at the beginning of the next line
		p.move_to(window.minx, y+1);
		p.put_block(window.maxy - y - 1, window.maxx - window.minx);
	}
}

void
rendering::RendererSoftware::render_contour(
	synfig::Surface &target_surface,
	const Contour::ChunkList &chunks,
	bool invert,
	bool antialias,
	Polyspan::WindingStyle winding_style,
	const Matrix &transform_matrix,
	const Color &color,
	Color::value_type opacity,
	Color::BlendMethod blend_method )
{
	Polyspan span;
	span.init(0, 0, target_surface.get_w(), target_surface.get_h());
	Vector p1, t0, t1;
	p1 = transform_matrix.get_transformed(Vector::zero());
	span.move_to(p1[0], p1[1]);
	for(Contour::ChunkList::const_iterator i = chunks.begin(); i != chunks.end(); ++i)
	{
		switch(i->type)
		{
			case Contour::ChunkType::CLOSE:
				span.close();
				break;
			case Contour::ChunkType::MOVE:
				p1 = transform_matrix.get_transformed(i->p1);
				span.move_to(p1[0], p1[0]);
				break;
			case Contour::ChunkType::LINE:
				p1 = transform_matrix.get_transformed(i->p1);
				span.line_to(p1[0], p1[1]);
				break;
			case Contour::ChunkType::CONIC:
				p1 = transform_matrix.get_transformed(i->p1);
				t0 = transform_matrix.get_transformed(i->t0, false);
				span.conic_to(t0[0], t0[1], p1[0], p1[1]);
				break;
			case Contour::ChunkType::CUBIC:
				p1 = transform_matrix.get_transformed(i->p1);
				t0 = transform_matrix.get_transformed(i->t0, false);
				t1 = transform_matrix.get_transformed(i->t1, false);
				span.cubic_to(t0[0], t0[1], t1[0], t1[1], p1[0], p1[1]);
				break;
			default:
				break;
		}
	}
	span.sort_marks();

	return render_polyspan(
		target_surface,
		span,
		invert,
		antialias,
		winding_style,
		color,
		opacity,
		blend_method );
}


bool
rendering::RendererSoftware::is_supported_vfunc(const DependentObject::Handle &obj) const
{
	if (TransformationAffine::Handle::cast_dynamic(obj))
	{
		return true;
	}
	else
	if (BlendingSimple::Handle simple = BlendingSimple::Handle::cast_dynamic(obj))
	{
		return !Color::is_straight(simple->get_method());
	}
	else
	if (Mesh::Handle mesh = Mesh::Handle::cast_dynamic(obj))
	{
		if (mesh->get_vertex_fields() == Mesh::MASK_POSITION)
			return true;
		if ( mesh->get_vertex_fields() == (Mesh::MASK_POSITION | Mesh::MASK_TEXCOORDS)
		  && (mesh->get_task() || mesh->get_surface()) )
			return true;
		return false;
	} else
	if (Contour::Handle::cast_dynamic(obj))
	{
		return true;
	} else
	if (Blur::Handle::cast_dynamic(obj))
	{
		return true;
	}
	return false;
}

rendering::Renderer::DependentObject::Handle
rendering::RendererSoftware::convert_vfunc(const DependentObject::Handle &obj)
{
	if (rendering::Surface::Handle surface = Surface::Handle::cast_dynamic(obj))
	{
		SurfaceSoftware::Handle surface_software(new SurfaceSoftware());
		surface_software->assign(surface);
		return surface_software;
	}
	return Renderer::convert_vfunc(obj);
}

bool
rendering::RendererSoftware::draw_vfunc(
	const Params &params,
	const Surface::Handle &target_surface,
	const Transformation::Handle &transformation,
	const Blending::Handle &blending,
	const Primitive::Handle &primitive )
{
	SurfaceSoftware::Handle surface_software = SurfaceSoftware::Handle::cast_dynamic(target_surface);
	if (!surface_software) return false;

	TransformationAffine::Handle transformation_affine = TransformationAffine::Handle::cast_dynamic(transformation);
	if (!transformation_affine) return false;

	BlendingSimple::Handle blending_simple = BlendingSimple::Handle::cast_dynamic(blending);
	if (!blending_simple) return false;

	if (Mesh::Handle mesh = Mesh::Handle::cast_dynamic(primitive))
	{
		if (mesh->check_vertex_field(Mesh::FIELD_TEXCOORDS))
		{
			Surface::Handle texture;
			if (mesh->get_task())
			{
				Vector size((Real)(surface_software->get_width()), (Real)(surface_software->get_height()));
				size = mesh->get_resolution_transfrom().get_transformed(size);
				int w = (int)round(size[0]);
				int h = (int)round(size[0]);
				if (w <= 0) w = 1;
				if (h <= 0) h = 1;
				if (w > params.max_surface_width) w = params.max_surface_width;
				if (h > params.max_surface_height) h = params.max_surface_height;

				texture = new SurfaceSoftware();
				texture->assign(w, h);
				mesh->get_task()->draw(params.root_renderer, params, texture);
			}

			if (!texture)
				texture = mesh->get_surface();

			SurfaceSoftware::Handle texture_software = convert(texture);
			if (!texture_software)
				return false;

			render_mesh(
				*surface_software,
				(const Vector*)mesh->get_vertex_pointer(Mesh::FIELD_POSITION),
				mesh->get_vertex_size(),
				(const Vector*)mesh->get_vertex_pointer(Mesh::FIELD_TEXCOORDS),
				mesh->get_vertex_size(),
				((const Mesh::Triangle*)mesh->get_triangles_pointer())->vertices,
				sizeof(Mesh::Triangle),
				mesh->get_triangles_count(),
				*texture_software,
				transformation_affine->get_matrix(),
				Matrix(),
				mesh->get_color().get_a(),
				blending_simple->get_method() );

			return true;
		}
		else
		{
			render_polygon(
				*surface_software,
				(const Vector*)mesh->get_vertex_pointer(Mesh::FIELD_POSITION),
				mesh->get_vertex_size(),
				((const Mesh::Triangle*)mesh->get_triangles_pointer())->vertices,
				sizeof(Mesh::Triangle),
				mesh->get_triangles_count(),
				transformation_affine->get_matrix(),
				mesh->get_color(),
				mesh->get_color().get_a(),
				blending_simple->get_method() );

			return true;
		}
	} else
	if (Contour::Handle contour = Contour::Handle::cast_dynamic(primitive))
	{
-		render_contour(
-			*surface_software,
			contour->get_chunks(),
			contour->get_invert(),
			contour->get_antialias(),
			contour->get_winding_style(),
			transformation_affine->get_matrix(),
			contour->get_color(),
			contour->get_color().get_a(),
			blending_simple->get_method() );

		return true;
	} else
	if (Blur::Handle blur = Blur::Handle::cast_dynamic(primitive))
	{
		assert(transformation_affine->get_matrix().is_identity());
		::Blur b(blur->get_size(), blur->get_type());
		int ex = 0, ey = 0;
		b.get_surface_extra_size(0.5*surface_software->get_w(), 0.5*surface_software->get_h(), ex, ey);

		Surface::Handle texture;
		if (blur->get_task())
		{
			int w = surface_software->get_w() + ex;
			int h = surface_software->get_h() + ey;
			if (w > params.max_surface_width) w = params.max_surface_width;
			if (h > params.max_surface_height) h = params.max_surface_height;

			texture = new SurfaceSoftware();
			texture->assign(w, h);
			blur->get_task()->draw(params.root_renderer, params, texture);
		}

		if (!texture)
			texture = blur->get_surface();

		SurfaceSoftware::Handle texture_software = convert(texture);
		if (!texture_software)
			return false;

			render_mesh(
				*surface_software,
				(const Vector*)mesh->get_vertex_pointer(Mesh::FIELD_POSITION),
				mesh->get_vertex_size(),
				(const Vector*)mesh->get_vertex_pointer(Mesh::FIELD_TEXCOORDS),
				mesh->get_vertex_size(),
				((const Mesh::Triangle*)mesh->get_triangles_pointer())->vertices,
				sizeof(Mesh::Triangle),
				mesh->get_triangles_count(),
				*texture_software,
				transformation_affine->get_matrix(),
				Matrix(),
				mesh->get_color().get_a(),
				blending_simple->get_method() );

			return true;
		}
	}

	return false;
}



/* === E N T R Y P O I N T ================================================= */
