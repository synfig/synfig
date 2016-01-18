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

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#include "mesh.h"

#endif

using namespace synfig;
using namespace rendering;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
Mesh::calculate_resolution_transfrom() const
{
	if (resolution_transfrom_calculated) return;
	resolution_transfrom_calculated = true;

	// TODO:
	resolution_transfrom.set_identity();

	if (vertices.empty())
	{
		source_rectangle = Rect::zero();
	}
	else
	{
		source_rectangle = Rect(vertices[0].tex_coords);
		for(std::vector<Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
			source_rectangle.expand(i->tex_coords);
	}
}

/* === E N T R Y P O I N T ================================================= */
