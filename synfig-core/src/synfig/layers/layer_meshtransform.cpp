/* === S Y N F I G ========================================================= */
/*!	\file layer_meshtransform.cpp
**	\brief Implementation of the "MeshTransform" layer
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

#include <cmath>
#include <climits>

#include <algorithm>

#include "layer_meshtransform.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/transform.h>

#include <synfig/rendering/software/task/taskmeshsw.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === C L A S S E S ======================================================= */

class synfig::Mesh_Trans : public Transform
{
	etl::handle<const Layer_MeshTransform> layer;
public:
	Mesh_Trans(const Layer_MeshTransform* x):Transform(x->get_guid()),layer(x) { }

	synfig::Vector perform(const synfig::Vector& x)const
	{
		Vector v(INFINITY, INFINITY);
		layer->mesh.transform_coord_texture_to_world(x, v);
		return v;
	}

	synfig::Vector unperform(const synfig::Vector& x)const
	{
		Vector v(INFINITY, INFINITY);
		layer->mesh.transform_coord_world_to_texture(x, v);
		return v;
	}

	synfig::String get_string()const
	{
		return "mesh";
	}
};

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

Layer_MeshTransform::Layer_MeshTransform():
	max_texture_size(4096),
	max_texture_scale(INFINITY),
	world_bounds(Rect::zero()),
	texture_bounds(Rect::zero())
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_MeshTransform::~Layer_MeshTransform()
{
}

void
Layer_MeshTransform::update_mesh_and_mask()
{
	// TODO: check mask to calculate bounds

	texture_scale_dependency_from_x = Vector::zero();
	texture_scale_dependency_from_y = Vector::zero();

	if (mesh.vertices.empty())
	{
		world_bounds = Rect::zero();
		texture_bounds = Rect::zero();
	}
	else
	{
		Mesh::VertexList::const_iterator i = mesh.vertices.begin();
		world_bounds.set_point(i->position);
		texture_bounds.set_point(i->tex_coords);
		for(++i; i != mesh.vertices.end(); ++i)
		{
			world_bounds.expand(i->position);
			texture_bounds.expand(i->position);
		}

		const Real epsilon = 1e-10;
		for(Mesh::TriangleList::const_iterator i = mesh.triangles.begin(); i != mesh.triangles.end(); ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				const Mesh::Vertex &v0 = mesh.vertices[i->vertices[j]];
				const Mesh::Vertex &v1 = mesh.vertices[i->vertices[(j+1)%3]];
				Vector wd( fabs(v1.position[0] - v0.position[0]),
						   fabs(v1.position[1] - v0.position[1]) );
				Vector td( fabs(v1.tex_coords[0] - v0.tex_coords[0]),
						   fabs(v1.tex_coords[1] - v0.tex_coords[1]) );
				if (td[0] > epsilon)
				{
					Vector dep(wd[0]/td[0], wd[1]/td[0]);
					if (dep[0] > texture_scale_dependency_from_x[0])
						texture_scale_dependency_from_x[0] = dep[0];
					if (dep[1] > texture_scale_dependency_from_y[0])
						texture_scale_dependency_from_y[0] = dep[1];
				}
				if (td[1] > epsilon)
				{
					Vector dep(wd[0]/td[1], wd[1]/td[1]);
					if (dep[0] > texture_scale_dependency_from_x[1])
						texture_scale_dependency_from_x[1] = dep[0];
					if (dep[1] > texture_scale_dependency_from_y[1])
						texture_scale_dependency_from_y[1] = dep[1];
				}
			}
		}

		if (max_texture_scale > 0.0)
		{
			if (texture_scale_dependency_from_x[0] > max_texture_scale)
				texture_scale_dependency_from_x[0] = max_texture_scale;
			if (texture_scale_dependency_from_x[1] > max_texture_scale)
				texture_scale_dependency_from_x[1] = max_texture_scale;
			if (texture_scale_dependency_from_y[0] > max_texture_scale)
				texture_scale_dependency_from_y[0] = max_texture_scale;
			if (texture_scale_dependency_from_y[1] > max_texture_scale)
				texture_scale_dependency_from_y[1] = max_texture_scale;
		}
	}
}

Layer::Handle
Layer_MeshTransform::hit_check(synfig::Context context, const synfig::Point &point)const
{
	// TODO: check mask
	Vector v;
	return mesh.transform_coord_world_to_texture(point, v)
		 ? context.hit_check(v)
		 : Layer::Handle();
}

Color
Layer_MeshTransform::get_color(Context context, const Point &pos)const
{
	// TODO: check mask
	Vector v;
	return mesh.transform_coord_world_to_texture(pos, v)
		 ? context.get_color(v)
		 : Color();
}

Rect
Layer_MeshTransform::get_full_bounding_rect(Context /* context */)const
{
	return world_bounds;
}

etl::handle<synfig::Transform>
Layer_MeshTransform::get_transform()const
{
	return new Mesh_Trans(this);
}

bool
Layer_MeshTransform::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	const Real epsilon = 1e-10;

	// initialize surface
	surface->set_wh(renddesc.get_w(),renddesc.get_h());
	surface->clear();

	// calculate texture size
	RendDesc texture_renddesc(renddesc);
	texture_renddesc.set_transformation_matrix(Matrix());
	texture_renddesc.set_tl(texture_bounds.get_min());
	texture_renddesc.set_br(texture_bounds.get_max());
	{
		int texture_width, texture_height;
		Real pw = fabs(renddesc.get_pw());
		Real ph = fabs(renddesc.get_ph());
		if (pw < epsilon || pw < epsilon) return true;
		pw = 1.0/pw;
		ph = 1.0/ph;
		Vector texture_size = texture_bounds.get_max() - texture_bounds.get_min();
		Real texture_pw = std::max(
			texture_scale_dependency_from_x[0]*pw,
			texture_scale_dependency_from_y[0]*ph );
		Real texture_ph = std::max(
			texture_scale_dependency_from_x[1]*pw,
			texture_scale_dependency_from_y[1]*ph );
		texture_width = std::max(1, (int)roundf(texture_pw*texture_size[0]));
		texture_height = std::max(1, (int)roundf(texture_ph*texture_size[1]));
		if (max_texture_size > 0)
		{
			if (texture_width > max_texture_size) texture_width = max_texture_size;
			if (texture_height > max_texture_size) texture_height = max_texture_size;
		}
		texture_renddesc.set_w(texture_width);
		texture_renddesc.set_h(texture_height);
	}

	// render texture
	Surface texture;
	if(!context.accelerated_render(&texture,quality,texture_renddesc,cb))
	{
		if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	{ // render mask
		Surface maskSurface;
		maskSurface.set_wh(texture.get_w(), texture.get_h());
		maskSurface.fill(Color::alpha());
		rendering::TaskMeshSW::render_polygon(
			maskSurface,
			&*mask.vertices.begin(),
			sizeof(*mask.vertices.begin()),
			mask.triangles.begin()->vertices,
			sizeof(*mask.triangles.begin()),
			mask.triangles.size(),
			texture_renddesc.get_transformation_matrix()
		  * texture_renddesc.get_world_to_pixels_matrix(),
			Color::white(),
			1.0,
			Color::BLEND_COMPOSITE );

		// apply mask
		Surface::pen a(texture.get_pen(0, 0));
		Surface::pen b(maskSurface.get_pen(0, 0));
		for(int i = 0; i < texture.get_h(); ++i)
		{
			for(int j = 0; j < texture.get_w(); ++j)
			{
				a.put_value(a.get_value()*b.get_value().get_a());
				a.inc_x();
				b.inc_x();
			}
			a.dec_x(texture.get_w());
			b.dec_x(texture.get_w());
			a.inc_y();
			b.inc_y();
		}
	}

	// prepare transformation matrices
	Matrix world_to_pixels_matrix =
		renddesc.get_transformation_matrix()
	  * renddesc.get_world_to_pixels_matrix();
	Matrix texture_to_texels_matrix =
		texture_renddesc.get_world_to_pixels_matrix();

	// render mesh
	rendering::TaskMeshSW::render_mesh(
		*surface,
		&mesh.vertices.begin()->position,
		sizeof(*mesh.vertices.begin()),
		&mesh.vertices.begin()->tex_coords,
		sizeof(*mesh.vertices.begin()),
		mesh.triangles.begin()->vertices,
		sizeof(*mesh.triangles.begin()),
		mesh.triangles.size(),
		texture,
		world_to_pixels_matrix,
		texture_to_texels_matrix,
		1.0,
		Color::BLEND_COMPOSITE
	);

	return true;
}
