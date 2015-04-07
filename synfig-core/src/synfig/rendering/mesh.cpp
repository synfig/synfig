/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/mesh.cpp
**	\brief Mesh
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

#include "mesh.h"

#endif

using namespace std;
using namespace synfig;
using namespace etl;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// MeshBase

void rendering::MeshBase::set_color(const Color &x)
	{ if (get_color() != x) { color = x; changed_common_data(); } }

void rendering::MeshBase::set_surface(const Surface::Handle &x)
	{ if (get_surface() != x) { surface = x; changed_common_data(); } }

void rendering::MeshBase::set_task(const RenderingTask::Handle &x)
	{ if (get_task() != x) { task = x; changed_common_data(); } }

void rendering::MeshBase::set_resolution_transfrom(const Matrix &x)
	{ if (get_resolution_transfrom() != x) { resolution_transfrom = x; changed_common_data(); } }

void rendering::MeshBase::apply_common_data(const MeshBase &data)
{
	set_color(data.get_color());
	set_surface(data.get_surface());
	set_task(data.get_task());
	set_resolution_transfrom(data.get_resolution_transfrom());
}

void rendering::MeshBase::changed_common_data()
{
	Handle alternative_mesh = Handle::cast_dynamic(get_alternative());
	if (alternative_mesh) alternative_mesh->apply_common_data(*this); else changed();
}

// Mesh

void rendering::Mesh::set_vertex_fields(int x)
{
	if (get_vertex_fields() != x)
	{
		int count = get_vertices_count();
		vertex_fields = x;
		vertices.resize(count * get_vertex_size(), 0);
		changed();
	}
}

void rendering::Mesh::set_vertices_count(int x)
{
	if (x < 0) x = 0;
	if (get_vertices_count() != x)
	{
		vertices.resize(x * get_vertex_size(), 0);
		changed();
	}
}

void* rendering::Mesh::get_editable_vertex_pointer()
{
	changed();
	return vertices.empty() ? NULL : &vertices.front();
}

void rendering::Mesh::set_triangles_count(int x)
{
	if (x < 0) x = 0;
	if (get_triangles_count() != x)
	{
		triangles.resize(x);
		changed();
	}
}

void* rendering::Mesh::get_editable_triangles_pointer()
{
	changed();
	return triangles.empty() ? NULL : &triangles.front();
}

/* === E N T R Y P O I N T ================================================= */
