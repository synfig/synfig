/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/renderersoftware.h
**	\brief RendererSoftware Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_RENDERERSOFTWARE_H
#define __SYNFIG_RENDERING_RENDERERSOFTWARE_H

/* === H E A D E R S ======================================================= */

#include "../vector.h"
#include "../matrix.h"
#include "../surface.h"

#include "renderer.h"
#include "surface.h"
#include "transformation.h"
#include "blending.h"
#include "primitive.h"
#include "polyspan.h"
#include "contour.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class RendererSoftware: public Renderer
{
public:
	typedef etl::handle<RendererSoftware> Handle;

private:
	class Internal;

public:
	static void render_triangle(
		synfig::Surface &target_surface,
		const Vector &p0,
		const Vector &p1,
		const Vector &p2,
		const Color &color,
		Color::value_type opacity,
		Color::BlendMethod blend_method );

	static void render_triangle(
		synfig::Surface &target_surface,
		const Vector &p0,
		const Vector &t0,
		const Vector &p1,
		const Vector &t1,
		const Vector &p2,
		const Vector &t2,
		const synfig::Surface &texture,
		Color::value_type opacity,
		Color::BlendMethod blend_method );

	static void render_polygon(
		synfig::Surface &target_surface,
		const Vector *vertices,
		int vertices_strip,
		const int *triangles,
		int triangles_strip,
		int triangles_count,
		const Matrix &transform_matrix,
		const Color &color,
		Color::value_type opacity,
		Color::BlendMethod blend_method );

	static void render_mesh(
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
		Color::BlendMethod blend_method );

	static void render_polyspan(
		synfig::Surface &target_surface,
		const Polyspan &polyspan,
		bool invert,
		bool antialias,
		Polyspan::WindingStyle winding_style,
		const Color &color,
		Color::value_type opacity,
		Color::BlendMethod blend_method );

	static void render_contour(
		synfig::Surface &target_surface,
		const Contour::ChunkList &chunks,
		bool invert,
		bool antialias,
		Polyspan::WindingStyle winding_style,
		const Matrix &transform_matrix,
		const Color &color,
		Color::value_type opacity,
		Color::BlendMethod blend_method );

protected:
	virtual bool is_supported_vfunc(const DependentObject::Handle &obj) const;
	virtual DependentObject::Handle convert_vfunc(const DependentObject::Handle &obj);
	virtual bool draw_vfunc(
		const Params &params,
		const Surface::Handle &target_surface,
		const Transformation::Handle &transformation,
		const Blending::Handle &blending,
		const Primitive::Handle &primitive );
};

}; /* end namespace rendering */
}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
