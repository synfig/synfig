/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskmeshsw.h
**	\brief TaskMeshSW Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_TASKMESHSW_H
#define __SYNFIG_RENDERING_TASKMESHSW_H

/* === H E A D E R S ======================================================= */

#include <synfig/surface.h>

#include "tasksw.h"
#include "../../primitive/mesh.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class TaskMeshSW: public Task, public TaskSW
{
private:
	class Internal;

public:
	typedef etl::handle<TaskMeshSW> Handle;

	Mesh::Handle mesh;

	Task::Handle clone() const { return clone_pointer(this); }

	const Task::Handle& sub_task() const { return Task::sub_task(0); }
	Task::Handle& sub_task() { return Task::sub_task(0); }

	virtual bool run(RunParams &params) const;

	// static

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
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
