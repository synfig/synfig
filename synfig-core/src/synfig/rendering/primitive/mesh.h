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

	#pragma pack(push, 1)
	struct Vertex
	{
		Vector position;
		Vector tex_coords;
		Vertex() { }
		Vertex(const Vector &position, const Vector &tex_coords):
			position(position), tex_coords(tex_coords) { }
	};
	#pragma pack(pop)

	struct Triangle
	{
		int vertices[3];
		Triangle()
			{ memset(vertices, 0, sizeof(vertices)); }
		Triangle(int v0, int v1, int v2)
			{ vertices[0] = v0; vertices[1] = v1; vertices[2] = v2; }
	};

	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;

private:
	mutable Rect source_rectangle;
	mutable Matrix2 resolution_transfrom;
	mutable bool resolution_transfrom_calculated;

public:
	Mesh(): resolution_transfrom_calculated(false) { }

	void reset_resolution_transfrom()
		{ resolution_transfrom_calculated = false; }

	void calculate_resolution_transfrom() const;

	void recalculate_resolution_transfrom()
	{
		reset_resolution_transfrom();
		calculate_resolution_transfrom();
	}

	const Matrix2& get_resolution_transfrom() const
	{
		if (!resolution_transfrom_calculated)
			calculate_resolution_transfrom();
		return resolution_transfrom;
	}

	const Rect& get_source_rectangle() const
	{
		if (!resolution_transfrom_calculated)
			calculate_resolution_transfrom();
		return source_rectangle;
	}
};

} /* end namespace rendering */
} /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
