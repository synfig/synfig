/* === S Y N F I G ========================================================= */
/*!	\file mesh.h
**	\brief Mesh
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

#ifndef __SYNFIG_MESH_H
#define __SYNFIG_MESH_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Mesh
{
public:
	class Vertex {
	public:
		Vector position;
		Vector tex_coords;
		inline Vertex() { }
		inline Vertex(const Vector &position, const Vector &tex_coords):
			position(position), tex_coords(tex_coords) { }
	};

	class Triangle {
	public:
		int vertices[3];
		inline Triangle() { vertices[0] = vertices[1] = vertices[2] = 0; }
		inline Triangle(int v0, int v1, int v2) { vertices[0] = v0; vertices[1] = v1; vertices[2] = v2; }
	};

	typedef std::vector<Vertex> VertexList;
	typedef std::vector<Triangle> TriangleList;

	VertexList vertices;
	TriangleList triangles;

	void clear() { vertices.clear(); triangles.clear(); }
	bool transform_coord_world_to_texture(const Vector &src, Vector &dest) const;
	bool transform_coord_texture_to_world(const Vector &src, Vector &dest) const;

	static bool transform_coord_world_to_texture(
		const Vector &src,
		Vector &dest,
		const Vector &p0,
		const Vector &t0,
		const Vector &p1,
		const Vector &t1,
		const Vector &p2,
		const Vector &t2 );

	inline static bool transform_coord_texture_to_world(
		const Vector &src,
		Vector &dest,
		const Vector &p0,
		const Vector &t0,
		const Vector &p1,
		const Vector &t1,
		const Vector &p2,
		const Vector &t2 )
	{
		return transform_coord_world_to_texture(src, dest, t0, p0, t1, p1, t2, p2);
	}
};

}; // END of namespace synfig

#endif
