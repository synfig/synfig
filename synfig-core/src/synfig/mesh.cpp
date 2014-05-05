/* === S Y N F I G ========================================================= */
/*!	\file mesh.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**  ......... ... 2014 Ivan Mahonin
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
#include "matrix.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

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
