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

#include "layer_meshtransform.h"

#include <synfig/general.h>
#include <synfig/localization.h>

#include <synfig/context.h>
#include <synfig/transform.h>

#include <synfig/rendering/common/task/taskmesh.h>
#include <synfig/rendering/common/task/taskblend.h>
#include <synfig/rendering/common/task/taskcontour.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === C L A S S E S ======================================================= */

namespace {
	class MeshTransform : public Transform
	{
	private:
		rendering::Mesh::Handle mesh;
	public:
		MeshTransform(const GUID& guid, const rendering::Mesh::Handle &mesh):
			Transform(guid), mesh(mesh) { }

		synfig::Vector perform(const synfig::Vector& x)const
		{
			Vector v(INFINITY, INFINITY);
			mesh->transform_coord_texture_to_world(x, v);
			return v;
		}

		synfig::Vector unperform(const synfig::Vector& x)const
		{
			Vector v(INFINITY, INFINITY);
			mesh->transform_coord_world_to_texture(x, v);
			return v;
		}

		synfig::String get_string()const { return "mesh"; }
	};
}

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

Layer_MeshTransform::Layer_MeshTransform(Real amount, Color::BlendMethod blend_method):
	Layer_CompositeFork(amount, blend_method),
	mesh(new rendering::Mesh())
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_MeshTransform::~Layer_MeshTransform()
	{ }

Layer::Handle
Layer_MeshTransform::hit_check(synfig::Context context, const synfig::Point &point)const
{
	Vector v;
	Layer::Handle layer;
	if ( get_amount() > 0.1
	  && mesh->transform_coord_world_to_texture(point, v)
	  && mask->is_inside(v)
	  && (layer = context.hit_check(v)) )
		return layer;
	return context.hit_check(point);
}

Color
Layer_MeshTransform::get_color(Context context, const Point &pos)const
{
	Vector v;
	if ( get_amount() > 0.1
	  && mesh->transform_coord_world_to_texture(pos, v)
	  && mask->is_inside(v) )
		return Color::blend(context.get_color(v), context.get_color(pos), get_amount(), get_blend_method());
	return context.get_color(pos);
}

Rect
Layer_MeshTransform::get_bounding_rect()const
	{ return mesh->get_target_rectangle(); }

etl::handle<synfig::Transform>
Layer_MeshTransform::get_transform()const
	{ return new MeshTransform(get_guid(), mesh); }

rendering::Task::Handle
Layer_MeshTransform::build_composite_fork_task_vfunc(ContextParams /* context_params */, rendering::Task::Handle sub_task)const
{
	if (!sub_task) return rendering::Task::Handle();
	
	rendering::TaskContour::Handle task_contour(new rendering::TaskContour());
	task_contour->contour = new rendering::Contour();
	task_contour->contour->assign(*mask);
	task_contour->contour->color = Color(1, 1, 1, 1);
	task_contour->contour->invert = !mask->invert;

	rendering::TaskBlend::Handle task_blend(new rendering::TaskBlend());
	task_blend->blend_method = Color::BLEND_ALPHA_OVER;
	task_blend->sub_task_a() = sub_task->clone_recursive();
	task_blend->sub_task_b() = task_contour;

	rendering::TaskMesh::Handle task_mesh(new rendering::TaskMesh());
	task_mesh->mesh = new rendering::Mesh();
	task_mesh->mesh->assign(*mesh);
	task_mesh->sub_task() = task_blend;
	return task_mesh;
}
