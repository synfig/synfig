/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/common/task/taskmesh.cpp
**	\brief TaskMesh
**
**	\legal
**	......... ... 2018 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
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

#include "taskmesh.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


Task::Token TaskMesh::token(
	DescAbstract<TaskMesh>("Mesh") );


Rect
TaskMesh::calc_bounds() const
	{ return mesh && sub_task() ? mesh->calc_target_rectangle(transformation->matrix) : Rect(); }

void
TaskMesh::set_coords_sub_tasks()
{
	if (!mesh || !sub_task())
		{ trunc_to_zero(); return; }
	if (!is_valid_coords())
		{ sub_task()->set_coords_zero(); return; }
	// TODO: use mesh->get_resolution_transfrom()
	sub_task()->set_coords(mesh->get_source_rectangle(), target_rect.get_size()*3/2);
}

/* === E N T R Y P O I N T ================================================= */
