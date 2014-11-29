/* === S Y N F I G ========================================================= */
/*!	\file polygon.h
**	\brief Polygon
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

#ifndef __SYNFIG_POLYGON_H
#define __SYNFIG_POLYGON_H

/* === H E A D E R S ======================================================= */

#include <vector>
#include "vector.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Polygon
{
public:
	typedef Vector Vertex;

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
};

}; // END of namespace synfig

#endif
