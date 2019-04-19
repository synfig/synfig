/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/mesh.cpp
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

#include "mesh.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Mesh::Mesh():
	resolution_transfrom_calculated(false) { }

void
Mesh::assign(const Mesh &other) {
	vertices = other.vertices;
	triangles = other.triangles;
	
	// other mesh is constant, so we need to lock mutexes for read the relolution data
	// see comment for calculate_resolution_transfrom() declaration
	{
		Mutex::Lock lock(other.resolution_transfrom_read_mutex);
		resolution_transfrom_calculated = other.resolution_transfrom_calculated;
		target_rectangle = other.target_rectangle;
		source_rectangle = other.source_rectangle;
		resolution_transfrom = other.resolution_transfrom;
	}
}

void
Mesh::clear()
{
	vertices.clear();
	triangles.clear();
	reset_resolution_transfrom();
}

void
Mesh::reset_resolution_transfrom()
	{ resolution_transfrom_calculated = false; }

Rect
Mesh::calc_target_rectangle() const
{
	if (vertices.empty()) return Rect::zero();
	Rect target_rectangle = Rect(vertices[0].position);
	for(std::vector<Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		target_rectangle.expand(i->position);
	return target_rectangle;
}

Rect
Mesh::calc_target_rectangle(const Matrix &transform_matrix) const
{
	if (vertices.empty()) return Rect::zero();
	Rect target_rectangle = Rect(transform_matrix.get_transformed(vertices[0].position));
	for(std::vector<Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		target_rectangle.expand( transform_matrix.get_transformed(i->position) );
	return target_rectangle;
}


Rect
Mesh::calc_source_rectangle() const
{
	if (vertices.empty()) return Rect::zero();
	Rect source_rectangle = Rect(vertices[0].position);
	for(std::vector<Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		source_rectangle.expand(i->position);
	return source_rectangle;
}
Rect
Mesh::calc_source_rectangle(const Matrix &transform_matrix) const
{
	if (vertices.empty()) return Rect::zero();
	Rect source_rectangle = Rect(transform_matrix.get_transformed(vertices[0].tex_coords));
	for(std::vector<Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
		source_rectangle.expand( transform_matrix.get_transformed(i->tex_coords) );
	return source_rectangle;
}

void
Mesh::calculate_resolution_transfrom_no_lock(bool force) const
{
	if (resolution_transfrom_calculated && !force)
		return;
	// TODO:
	resolution_transfrom.set_identity();
	target_rectangle = calc_target_rectangle();
	source_rectangle = calc_source_rectangle();
	resolution_transfrom_calculated = true;
}

void
Mesh::calculate_resolution_transfrom(bool force) const
{
	Mutex::Lock lock(resolution_transfrom_read_mutex);
	calculate_resolution_transfrom_no_lock(force);
}

Matrix2
Mesh::get_resolution_transfrom() const
{
	Mutex::Lock lock(resolution_transfrom_read_mutex);
	calculate_resolution_transfrom_no_lock();
	return resolution_transfrom;
}

Rect
Mesh::get_target_rectangle() const
{
	Mutex::Lock lock(resolution_transfrom_read_mutex);
	calculate_resolution_transfrom_no_lock();
	return target_rectangle;
}

Rect
Mesh::get_source_rectangle() const
{
	Mutex::Lock lock(resolution_transfrom_read_mutex);
	calculate_resolution_transfrom_no_lock();
	return source_rectangle;
}

bool
Mesh::transform_coord_world_to_texture(
	const Vector &src,
	Vector &dest,
	const Vector &p0,
	const Vector &t0,
	const Vector &p1,
	const Vector &t1,
	const Vector &p2,
	const Vector &t2 )
{
	// is point inside triangle?
	Matrix matrix_of_base_triangle(
		p1[0]-p0[0], p1[1]-p0[1], 0.0,
		p2[0]-p0[0], p2[1]-p0[1], 0.0,
		p0[0], p0[1], 1.0 );
	if (!matrix_of_base_triangle.is_invertible()) return false;

	matrix_of_base_triangle.invert();
	Vector v = matrix_of_base_triangle.get_transformed(src);
	if (v[0] < 0.0 || v[0] > 1.0
	 || v[1] < 0.0 || v[1] > 1.0
	 || v[0] + v[1] > 1.0) return false;

	// get coords at texture
	Matrix matrix_of_texture_triangle(
		t1[0]-t0[0], t1[1]-t0[1], 0.0,
		t2[0]-t0[0], t2[1]-t0[1], 0.0,
		t0[0], t0[1], 1.0 );
	dest = matrix_of_texture_triangle.get_transformed(v);
	return true;
}

bool
Mesh::transform_coord_world_to_texture(const Vector &src, Vector &dest) const
{
	// process triangles backward
	for(TriangleList::const_reverse_iterator ri = triangles.rbegin(); ri != triangles.rend(); ++ri)
		if (transform_coord_world_to_texture(
			src,
			dest,
			vertices[ri->vertices[0]].position,
			vertices[ri->vertices[0]].tex_coords,
			vertices[ri->vertices[1]].position,
			vertices[ri->vertices[1]].tex_coords,
			vertices[ri->vertices[2]].position,
			vertices[ri->vertices[2]].tex_coords
		))
			return true;
	return false;
}

bool
Mesh::transform_coord_texture_to_world(const Vector &src, Vector &dest) const
{
	// process triangles backward
	for(TriangleList::const_reverse_iterator ri = triangles.rbegin(); ri != triangles.rend(); ++ri)
		if (transform_coord_texture_to_world(
			src,
			dest,
			vertices[ri->vertices[0]].position,
			vertices[ri->vertices[0]].tex_coords,
			vertices[ri->vertices[1]].position,
			vertices[ri->vertices[1]].tex_coords,
			vertices[ri->vertices[2]].position,
			vertices[ri->vertices[2]].tex_coords
		))
			return true;
	return false;
}

/* === E N T R Y P O I N T ================================================= */
