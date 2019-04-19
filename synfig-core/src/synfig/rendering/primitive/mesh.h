/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/primitive/mesh.h
**	\brief Mesh Header
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

#ifndef __SYNFIG_RENDERING_MESH_H
#define __SYNFIG_RENDERING_MESH_H

/* === H E A D E R S ======================================================= */

#include <cstring>

#include <vector>

#include <ETL/handle>

#include <synfig/rect.h>
#include <synfig/matrix.h>
#include <synfig/mutex.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{
namespace rendering
{

class Mesh: public etl::shared_object
{
public:
	typedef etl::handle<Mesh> Handle;

	struct Vertex
	{
		Vector position;
		Vector tex_coords;
		Vertex() { }
		Vertex(const Vector &position, const Vector &tex_coords):
			position(position), tex_coords(tex_coords) { }
	};

	struct Triangle
	{
		int vertices[3];
		Triangle()
			{ vertices[0] = vertices[1] = vertices[2] = 0; }
		Triangle(int v0, int v1, int v2)
			{ vertices[0] = v0; vertices[1] = v1; vertices[2] = v2; }
	};

	typedef std::vector<Vertex> VertexList;
	typedef std::vector<Triangle> TriangleList;

	VertexList vertices;
	TriangleList triangles;

private:
	mutable Mutex resolution_transfrom_read_mutex;
	mutable bool resolution_transfrom_calculated;
	mutable Rect target_rectangle;
	mutable Rect source_rectangle;
	mutable Matrix2 resolution_transfrom;
	
	void calculate_resolution_transfrom_no_lock(bool force = false) const;

public:
	Mesh();
	void assign(const Mesh &other);
	void clear();
	void reset_resolution_transfrom();

	Rect calc_target_rectangle() const;
	Rect calc_target_rectangle(const Matrix &transform_matrix) const;

	Rect calc_source_rectangle() const;
	Rect calc_source_rectangle(const Matrix &transform_matrix) const;

	// actualize internal resolution data (if need) and return it
	// method is thread-safe for constant meshes - you must not modify a mesh while call these methods
	void calculate_resolution_transfrom(bool force = false) const;
	Matrix2 get_resolution_transfrom() const;
	Rect get_target_rectangle() const;
	Rect get_source_rectangle() const;

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

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
