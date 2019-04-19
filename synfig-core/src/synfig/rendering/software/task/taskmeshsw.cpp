/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/task/taskmeshsw.cpp
**	\brief TaskMeshSW
**
**	$Id$
**
**	\legal
**	......... ... 2015-2019 Ivan Mahonin
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

#include "../../common/task/taskmesh.h"
#include "tasksw.h"
#include "../function/mesh.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace {

class TaskMeshSW: public TaskMesh, public TaskSW
{
	typedef etl::handle<TaskMeshSW> Handle;
	static Token token;
	virtual Token::Handle get_token() const { return token.handle(); }

	virtual bool run(RunParams&) const {
		if (!is_valid() || !sub_task() || !sub_task()->is_valid())
			return true;
		if (!mesh)
			return false;

		Vector ppu = get_pixels_per_unit();
		Matrix transfromation_matrix;
		transfromation_matrix.m00 = ppu[0];
		transfromation_matrix.m11 = ppu[1];
		transfromation_matrix.m20 = target_rect.minx - source_rect.minx*ppu[0];
		transfromation_matrix.m21 = target_rect.miny - source_rect.miny*ppu[1];
		transfromation_matrix *=  transformation->matrix;

		Vector sub_ppu = sub_task()->get_pixels_per_unit();
		Matrix texture_transfromation_matrix;
		texture_transfromation_matrix.m00 = sub_ppu[0];
		texture_transfromation_matrix.m11 = sub_ppu[1];
		texture_transfromation_matrix.m20 = sub_task()->target_rect.minx - sub_task()->source_rect.minx*sub_ppu[0];
		texture_transfromation_matrix.m21 = sub_task()->target_rect.miny - sub_task()->source_rect.miny*sub_ppu[1];

		if (target_surface == sub_task()->target_surface)
			return false;

		LockWrite la(this);
		if (!la) return false;
		LockRead lb(sub_task());
		if (!lb) return false;

		RectInt sub_target_rect_int = sub_task()->target_rect;
		Rect sub_target_rect;
		sub_target_rect.minx = sub_target_rect_int.minx;
		sub_target_rect.miny = sub_target_rect_int.miny;
		sub_target_rect.maxx = sub_target_rect_int.maxx;
		sub_target_rect.maxy = sub_target_rect_int.maxy;
		
		software::Mesh::render_mesh(
			la->get_surface(),
			target_rect,
			*mesh,
			lb->get_surface(),
			sub_target_rect,
			transfromation_matrix,
			texture_transfromation_matrix,
			1.0,
			Color::BLEND_COMPOSITE );

		return true;
	}
};

Task::Token TaskMeshSW::token(
	DescReal<TaskMeshSW, TaskMesh>("MeshSW") );

} // end of anonimous namespace

/* === E N T R Y P O I N T ================================================= */
