/* === S Y N F I G ========================================================= */
/*!	\file mesh.h
**	\brief Mesh
**
**	$Id$
**
**	\legal
**	Copyright (c) 2008 Carlos López & Chirs Moore
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

#include "angle.h"
#include "real.h"
#include "vector.h"
#include "string.h"
#include <cassert>
#include <math.h>
#include <iostream>
#include <ETL/stringf>

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
	};

	class Triangle {
	public:
		int vertices[3];
		inline Triangle() { vertices[0] = vertices[1] = vertices[2] = 0; }
	};

	typedef std::vector<Vertex> VertexList;
	typedef std::vector<Triangle> TriangleList;

	VertexList vertices;
	TriangleList triangles;
};

}; // END of namespace synfig

#endif
